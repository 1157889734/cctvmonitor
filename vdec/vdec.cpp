#include "vdec.h"
#include "shm.h"

#if 1
#define MAX_DEC_RES_W           1920
#define MAX_DEC_RES_H           1080
#define MAX_FILE_NAME_LENGTH    256
#define MPI_DEC_STREAM_SIZE     (SZ_4K)

typedef struct{
    unsigned int width;
    unsigned int height;
    unsigned int type;
}T_DEC_CMD;

typedef struct
{
    MppPacket 		packet;

    MppCtx          ctx;
    MppApi          *mpi;

    /* end of stream flag when set quit the loop */
    RK_U32          eos;

    /* input and output */
    MppBufferGroup  frm_grp;

}T_VDEC_INFO;

typedef struct _T_DATA_PACKET
{
    char *pcData;
    int iLen;
    unsigned int iPts;
}T_DATA_PACKET, *PT_DATA_PACKET;

typedef struct _T_DATA_PACKET_LIST
{
    T_DATA_PACKET tPkt;
    struct _T_DATA_PACKET_LIST *next;
}T_DATA_PACKET_LIST, *PT_DATA_PACKET_LIST;

typedef struct _T_PACKET_QUEUE {
    T_DATA_PACKET_LIST *first_pkt, *last_pkt;
    int nb_packets;
    CMutexLock *mutex;
}T_PACKET_QUEUE;

typedef struct _T_VIDEO_DEC_INFO
{
    T_PACKET_QUEUE tPacketQueue;
    pthread_t hVideoDecodecThread;
    CMutexLock cMute;
    volatile int iVideoExitFlag;
    volatile int iVideoExitFlagOver;
    volatile int iStartPlayFlag;
    volatile int iDisPlayFlag;
    int iDecType;  //CMP_VDEC_TYPE
    T_WND_INFO  *ptWndInfo;
    T_VDEC_INFO *ptDecInfo;
    //RGA_HANDLE   pRgaHandle;

}T_VIDEO_DEC_INFO, *PT_VIDEO_DEC_INFO;


int dec_simple(PT_VIDEO_DEC_INFO ptVideoInfo, T_DATA_PACKET *ptPkt)
{
    int rt = 0;
    T_VDEC_INFO *ptDecInfo = (T_VDEC_INFO *)ptVideoInfo->ptDecInfo;
    RK_U32 pkt_done = 0;
    RK_U32 pkt_eos  = 0;
    RK_U32 err_info = 0;
    MPP_RET ret = MPP_OK;
    MppCtx ctx  = ptDecInfo->ctx;
    MppApi *mpi = ptDecInfo->mpi;

    MppPacket packet = ptDecInfo->packet;
    MppFrame  frame  = NULL;

    mpp_packet_set_data(packet, ptPkt->pcData);
    if (ptPkt->iLen > mpp_packet_get_size(packet))
        mpp_packet_set_size(packet, ptPkt->iLen);
    mpp_packet_set_pos(packet, ptPkt->pcData);
    mpp_packet_set_length(packet, ptPkt->iLen);
    // setup eos flag
    if (pkt_eos)
        mpp_packet_set_eos(packet);

    do
    {
        RK_S32 times = 5;
        // send the packet first if packet is not done
        if (!pkt_done)
        {
            ret = mpi->decode_put_packet(ctx, packet);
            if (MPP_OK == ret)
            {
                pkt_done = 1;
            }
        }


        // then get all available frame and release
        do
        {
            RK_S32 get_frm = 0;
            RK_U32 frm_eos = 0;

        try_again:
            ret = mpi->decode_get_frame(ctx, &frame);
            if (MPP_ERR_TIMEOUT == ret)
            {
                if (times > 0) {
                    times--;
                    usleep(2000);
                    goto try_again;
                }
                printf("decode_get_frame failed too much time\n");
            }
            if (MPP_OK != ret)
            {
                printf("decode_get_frame failed ret %d\n", ret);
                break;
            }


            if (frame)
            {
                if (mpp_frame_get_info_change(frame))
                {
                    RK_U32 width = mpp_frame_get_width(frame);
                    RK_U32 height = mpp_frame_get_height(frame);
                    RK_U32 hor_stride = mpp_frame_get_hor_stride(frame);
                    RK_U32 ver_stride = mpp_frame_get_ver_stride(frame);
                    RK_U32 buf_size = mpp_frame_get_buf_size(frame);
                    RK_U32 ox = mpp_frame_get_offset_x(frame);
                    RK_U32 oy = mpp_frame_get_offset_y(frame);


//                    printf("decode_get_frame get info changed found\n");
//                    printf("decoder require buffer w:h [%d:%d] stride [%d:%d] buf_size %d,x:%d,y:%d \n",
//                              width, height, hor_stride, ver_stride, buf_size, ox, oy);

                    if (NULL == ptDecInfo->frm_grp) {
                        /* If buffer group is not set create one and limit it */
                        ret = mpp_buffer_group_get_internal(&ptDecInfo->frm_grp, MPP_BUFFER_TYPE_ION);
                        if (ret) {
                            printf("get mpp buffer group failed ret %d\n", ret);
                            break;
                        }

                        /* Set buffer to mpp decoder */
                        ret = mpi->control(ctx, MPP_DEC_SET_EXT_BUF_GROUP, ptDecInfo->frm_grp);
                        if (ret) {
                            printf("set buffer group failed ret %d\n", ret);
                            break;
                        }
                    } else {
                        /* If old buffer group exist clear it */
                        ret = mpp_buffer_group_clear(ptDecInfo->frm_grp);
                        if (ret) {
                            printf("clear buffer group failed ret %d\n", ret);
                            break;
                        }
                    }


                    /* Use limit config to limit buffer count to 24 with buf_size */
                    ret = mpp_buffer_group_limit_config(ptDecInfo->frm_grp, buf_size, 24);
                    if (ret) {
                        printf("limit buffer group failed ret %d\n", ret);
                        break;
                    }

                    /*
                     * All buffer group config done. Set info change ready to let
                     * decoder continue decoding
                     */
                    ret = mpi->control(ctx, MPP_DEC_SET_INFO_CHANGE_READY, NULL);
                    if (ret) {
                        printf("info change ready failed ret %d\n", ret);
                        break;
                    }
                }
                else
                {
                    err_info = mpp_frame_get_errinfo(frame) | mpp_frame_get_discard(frame);
                    if (!err_info)
                    {
                        //DRM_Display(frame);
                        if(ptVideoInfo->iDisPlayFlag == DISPLAY_START)
                        {
                            rt = SHM_Display(ptVideoInfo->ptWndInfo->pRenderHandle, frame);
                            if(rt < 0)
                            {
                                mpp_frame_deinit(&frame);
                                return rt;
                            }
                        }
                    }

                }
                frm_eos = mpp_frame_get_eos(frame);
                mpp_frame_deinit(&frame);
                frame = NULL;
                get_frm = 1;
            }

            // if last packet is send but last frame is not found continue
            if (pkt_eos && pkt_done && !frm_eos)
            {
                usleep(1000);
                continue;
            }

            if (frm_eos)
            {
                // mpp_log("found last frame\n");
                break;
            }

            if (get_frm)
                continue;
            break;
        } while (1);

        if (pkt_done)
            break;

        /*
         * why sleep here:
         * mpi->decode_put_packet will failed when packet in internal queue is
         * full,waiting the package is consumed .Usually hardware decode one
         * frame which resolution is 1080p needs 2 ms,so here we sleep 3ms
         * * is enough.
         */
        usleep(1000);
    } while (1);

    return rt;

}
int decode_open(T_VDEC_INFO* ptDecInfo, T_DEC_CMD* cmd)
{
    // base flow context
    MppCtx ctx          = NULL;
    MppApi *mpi         = NULL;

    // input / output
    MppPacket packet    = NULL;
    MppFrame  frame     = NULL;

    MpiCmd mpi_cmd      = MPP_CMD_BASE;
    MppParam param      = NULL;
    RK_U32 need_split   = 1;
    MppPollType timeout = MPP_POLL_NON_BLOCK;//cmd->timeout;

    // paramter for resource malloc
    RK_U32 width        = cmd->width;
    RK_U32 height       = cmd->height;
    MppCodingType type  = (MppCodingType)cmd->type;

    MppFrameFormat  format = MPP_FMT_YUV420P;


    int ret = 0;
    ret = mpp_packet_init(&packet, NULL, 0);
    if (ret) {
        printf("mpp_packet_init failed\n");
        goto MPP_TEST_OUT;
    }


    printf("mpi_dec_test decoder test start w %d h %d type %d\n", width, height, type);

    // decoder demo
    ret = mpp_create(&ctx, &mpi);

    if (MPP_OK != ret) {
        printf("mpp_create failed\n");
        goto MPP_TEST_OUT;
    }

    // NOTE: decoder split mode need to be set before init
    mpi_cmd = MPP_DEC_SET_PARSER_SPLIT_MODE;
    param = &need_split;
    ret = mpi->control(ctx, mpi_cmd, param);
    if (MPP_OK != ret) {
        printf("mpi->control failed\n");
        goto MPP_TEST_OUT;
    }

    // NOTE: timeout value please refer to MppPollType definition
    //  0   - non-block call (default)
    // -1   - block call
    // +val - timeout value in ms
    if (timeout) {
        param = &timeout;
        ret = mpi->control(ctx, MPP_SET_OUTPUT_TIMEOUT, param);
        if (MPP_OK != ret) {
            printf("Failed to set output timeout %d ret %d\n", timeout, ret);
            goto MPP_TEST_OUT;
        }
    }


    ret = mpp_init(ctx, MPP_CTX_DEC, type);
    if (MPP_OK != ret) {
        printf("mpp_init failed\n");
        goto MPP_TEST_OUT;
    }

    ptDecInfo->ctx            = ctx;
    ptDecInfo->mpi            = mpi;
    ptDecInfo->eos            = 0;
    ptDecInfo->packet         = packet;
    ptDecInfo->frm_grp      = NULL;

    ret = mpi->control(ctx, MPP_DEC_SET_OUTPUT_FORMAT, &format);

    return 0;

MPP_TEST_OUT:
    printf(" MPP_TEST_OUT    vv \n");
    if (packet) {
        mpp_packet_deinit(&packet);
        packet = NULL;
    }

    if (frame) {
        mpp_frame_deinit(&frame);
        frame = NULL;
    }

    if (ctx) {
        mpp_destroy(ctx);
        ctx = NULL;
    }

    if (ptDecInfo->frm_grp) {
        mpp_buffer_group_put(ptDecInfo->frm_grp);
        ptDecInfo->frm_grp = NULL;
    }

    return -1;
}

int decoder_close(T_VDEC_INFO* ptDecInfo)
{

    if (ptDecInfo->packet) {
        mpp_packet_deinit(&ptDecInfo->packet);
        ptDecInfo->packet = NULL;
    }

    if (ptDecInfo->ctx) {
        ptDecInfo->mpi->reset(ptDecInfo->ctx);
        mpp_destroy(ptDecInfo->ctx);
        ptDecInfo->ctx = NULL;
    }

    if (ptDecInfo->frm_grp) {
        mpp_buffer_group_put(ptDecInfo->frm_grp);
        ptDecInfo->frm_grp = NULL;
    }

    return 0;
}




static void packet_queue_init(T_PACKET_QUEUE *q)
{
    memset(q, 0, sizeof(T_PACKET_QUEUE));
    q->mutex = new CMutexLock();
}

static void packet_queue_flush(T_PACKET_QUEUE *q)
{
    PT_DATA_PACKET_LIST pkt, pkt1;
    if(NULL==q)
    {
        return;
    }
    q->mutex->Lock();
    for(pkt = q->first_pkt; pkt != NULL; pkt = pkt1) {
        pkt1 = pkt->next;
        if((NULL!=pkt) && (NULL!=pkt->tPkt.pcData))
        {
            free(pkt->tPkt.pcData);
            free(pkt);
        }
    }
    q->last_pkt = NULL;
    q->first_pkt = NULL;
    q->nb_packets = 0;
    q->mutex->Unlock();
}

static void packet_queue_uninit(T_PACKET_QUEUE *q)
{
    if(q)
    {
        packet_queue_flush(q);
        delete q->mutex;
        q->mutex = NULL;
        q = NULL;
    }
}

static int packet_queue_put(T_PACKET_QUEUE *q, PT_DATA_PACKET pkt)
{
    PT_DATA_PACKET_LIST pktl = NULL;

    if (NULL == pkt)
        return -1;
    if (NULL == q->mutex)
    {
        free(pkt);
        return -1;
    }

    pktl = (PT_DATA_PACKET_LIST)malloc(sizeof(T_DATA_PACKET_LIST));
    if (!pktl)
        return -1;
    pktl->tPkt = *pkt;
    pktl->next = NULL;

    while(1)
    {
        q->mutex->Lock();
        if (q->nb_packets > 100)
        {
            q->mutex->Unlock();
            Sleep(1);
        }
        else
        {
            q->mutex->Unlock();
            break;
        }
    }

    q->mutex->Lock();
    if (!q->last_pkt)

        q->first_pkt = pktl;
    else
        q->last_pkt->next = pktl;
    q->last_pkt = pktl;
    q->nb_packets++;

    q->mutex->Unlock();
    return 0;
}

static int packet_queue_get(T_PACKET_QUEUE *q, PT_DATA_PACKET pkt, int block)
{
    PT_DATA_PACKET_LIST pktl;
    int ret =0;
    if(NULL==q||NULL==q->mutex)
    {
        return ret;
    }
    q->mutex->Lock();
    for(;;)
    {
        pktl = q->first_pkt;
        if (pktl) {
            q->first_pkt = pktl->next;
            if (!q->first_pkt)
                q->last_pkt = NULL;
            q->nb_packets--;
            *pkt = pktl->tPkt;
            free(pktl);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            Sleep(1);
        }
    }
    q->mutex->Unlock();
    return ret;
}



/*************************************************
函数功能:     VDEC_Init
函数描述:     初使化视频解码
输入参数:     无
输出参数:     无
返回值:       0:成功, 否则:失败
作者:         丁金奇
日期:         2014-04-20
修改:
*************************************************/
int VDEC_Init(void)
{

    return 0;
}


int VDEC_Uninit()
{

    return 0;
}

/* 视频播放线程,流程是从视频队列取视频流,再进行H264解码,然后显示 */
void* DecodecVideoProc(void *argv)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)argv;
    T_DATA_PACKET tPkt;
    int iRet = 0;
    ptVideoInfo->iVideoExitFlagOver = 1;
    while(!ptVideoInfo->iVideoExitFlag)
    {
        tPkt.pcData = NULL;
        iRet = packet_queue_get(&ptVideoInfo->tPacketQueue, &tPkt, 0);
        if (0 == iRet )
        {
            usleep(10000);
            continue;
        }
        if(tPkt.pcData)
        {
            if(tPkt.iLen == 0)
            {
                free(tPkt.pcData);
                tPkt.pcData = NULL;
                continue;
            }
        }
        else
        {
            usleep(10000);
            continue;
        }
        if(ptVideoInfo->iStartPlayFlag != START_STREAM_PLAY)
        {
            free(tPkt.pcData);
            tPkt.pcData = NULL;  //
            continue;
        }
        iRet = dec_simple(ptVideoInfo, &tPkt);
        free(tPkt.pcData);
        tPkt.pcData = NULL;

    }

    ptVideoInfo->iVideoExitFlagOver = 0;

    return 0;
}

VDEC_HADNDLE VDEC_CreateVideoDecCh(T_WND_INFO *pWndInfo, int iWidth, int iHeight,int iDecType,int iCodecID)
{
    int iRet = 0;
    PT_VIDEO_DEC_INFO ptVideoInfo = NULL;
    ptVideoInfo = new T_VIDEO_DEC_INFO;
    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return NULL;
    }
    packet_queue_init(&ptVideoInfo->tPacketQueue);
    ptVideoInfo->hVideoDecodecThread = 0;
    ptVideoInfo->iVideoExitFlag = 0;
    ptVideoInfo->iVideoExitFlagOver = 0;

    ptVideoInfo->iDisPlayFlag = DISPLAY_STOP;
    ptVideoInfo->iStartPlayFlag = STOP_STREAM_PLAY;
    ptVideoInfo->iDecType = iDecType;
    ptVideoInfo->ptWndInfo = pWndInfo;

    T_DEC_CMD cmd;
    T_VDEC_INFO* ptDecInfo = NULL;
    cmd.width  = iWidth;
    cmd.height = iHeight;
    cmd.type   = H264_CODE == iCodecID ? MPP_VIDEO_CodingAVC : MPP_VIDEO_CodingHEVC;
    ptDecInfo = (T_VDEC_INFO*)malloc(sizeof(T_VDEC_INFO));
    if (NULL == ptDecInfo)
    {
        printf("Failed to allocate T_VPU_INFO\n");
        return 0;
    }
    memset(ptDecInfo,0,sizeof(T_VDEC_INFO));


    ptVideoInfo->ptDecInfo = NULL;
    iRet = decode_open(ptDecInfo, &cmd);
    if (iRet < 0)
    {
        free(ptDecInfo);
        ptDecInfo = NULL;
        printf("decode_open error \n");
    }
    ptVideoInfo->ptDecInfo = ptDecInfo;

    pthread_create(&ptVideoInfo->hVideoDecodecThread, NULL, DecodecVideoProc, ptVideoInfo);

    return (VDEC_HADNDLE)ptVideoInfo;

}


int VDEC_DestroyVideoDecCh(VDEC_HADNDLE VHandle)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return -1;
    }

    ptVideoInfo->iVideoExitFlag = 1;
    while(ptVideoInfo->iVideoExitFlagOver)
    {
        usleep(10000);
    }

    T_VDEC_INFO* ptDecInfo = (T_VDEC_INFO* )ptVideoInfo->ptDecInfo;
    if(ptDecInfo)
    {
        decoder_close(ptDecInfo);
        free(ptDecInfo);
        ptVideoInfo->ptDecInfo = NULL;
    }

    if (ptVideoInfo->hVideoDecodecThread)
    {
        pthread_join(ptVideoInfo->hVideoDecodecThread, NULL);
        ptVideoInfo->hVideoDecodecThread = 0;
    }

    packet_queue_flush(&ptVideoInfo->tPacketQueue);
    packet_queue_uninit(&ptVideoInfo->tPacketQueue);

    delete(ptVideoInfo);
    ptVideoInfo = NULL;

    return 0;
}

/*************************************************
函数功能:     VDEC_SendStream
函数描述:     发送解码流到视频解码模块
输入参数:     pData:视频流
iLen: 视频流长度
输出参数:     无
返回值:       0:成功, 否则:失败
作者:         丁金奇
日期:         2014-04-20
修改:
*************************************************/
int VDEC_SendVideoStream(VDEC_HADNDLE VHandle, void *pData, int iLen, unsigned int iPts)
{
    T_DATA_PACKET tPkt;
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

    if(iLen > 750000)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] iLen Biger 750000",__FUNCTION__, __LINE__);
    }

    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return -1;
    }

    if (NULL == pData || 0 == iLen)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] NULL == pData || 0 == iLen",__FUNCTION__, __LINE__);
        return -1;
    }

    if(ptVideoInfo->iStartPlayFlag != START_STREAM_PLAY)
    {
        return 0;
    }

    if(ptVideoInfo->tPacketQueue.mutex)
    {
        tPkt.pcData = (char *)malloc(iLen+32);
        tPkt.iLen = iLen;
        tPkt.iPts = iPts;
        memcpy(tPkt.pcData, pData, iLen);
        packet_queue_put(&ptVideoInfo->tPacketQueue, &tPkt);
    }
    return 0;
}


/*************************************************
函数功能:     VDEC_ChangeWindow
函数描述:     改变显示窗口
输入参数:     hWnd:新显示窗口
输出参数:     无
返回值:       0:成功, 否则:失败
作者:         丁金奇
日期:         2014-04-20
修改:
*************************************************/

int VDEC_ChangeWindow(VDEC_HADNDLE VHandle, const T_WND_INFO *pWndInfo)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return -1;
    }

    if(ptVideoInfo->ptWndInfo && ptVideoInfo->ptWndInfo->pRenderHandle)
    {

    }
    return 0;
}

/*************************************************
函数功能:     VDEC_StartPlayStream
函数描述:     开始播放解码流
输入参数:     无
输出参数:     无
返回值:       0:成功, 否则:失败
作者:         丁金奇
日期:         2014-04-30
修改:
*************************************************/
int VDEC_StartPlayStream(VDEC_HADNDLE VHandle)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return -1;
    }

    ptVideoInfo->iStartPlayFlag = START_STREAM_PLAY;

    return 0;
}

/*************************************************
函数功能:     VDEC_StopPlayStream
函数描述:     停止播放解码流
输入参数:     无
输出参数:     无
返回值:       0:成功, 否则:失败
作者:         丁金奇
日期:         2014-04-30
修改:
*************************************************/
int VDEC_StopPlayStream(VDEC_HADNDLE VHandle)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return -1;
    }
    ptVideoInfo->iStartPlayFlag = STOP_STREAM_PLAY;
    return 0;
}

/*************************************************
函数功能:     VDEC_PausePlayStream
函数描述:     暂停播放解码流
输入参数:     无
输出参数:     无
返回值:       0:成功, 否则:失败
作者:         丁金奇
日期:         2014-04-30
修改:
*************************************************/
int VDEC_PausePlayStream(VDEC_HADNDLE VHandle)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return -1;
    }
    ptVideoInfo->iStartPlayFlag = PAUSE_STREAM_PLAY;

    return 0;
}

int VDEC_DisplayEnable(VDEC_HADNDLE VHandle, int displayFlag)
{
    PT_VIDEO_DEC_INFO ptVideoInfo = (PT_VIDEO_DEC_INFO)VHandle;

    if (NULL == ptVideoInfo)
    {
        DebugPrint( DEBUG_VDEC_PRINT,"[%s %d] error",__FUNCTION__, __LINE__);
        return -1;
    }
    ptVideoInfo->iDisPlayFlag = displayFlag;
    return 0;
}

#endif
