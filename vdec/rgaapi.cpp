#include "rgaapi.h"

int getMppFrame(T_RGA_INFO *ptRgaInfo, int w, int h)
{
    int i = 0;
    int iW = 0;
    int iH = 0;
    //w = MPP_ALIGN(w, 16);
    //h = MPP_ALIGN(h, 16);
    for (i = 0; i < MAX_MPP_FRAME_NUM; i++)
    {
        if(!ptRgaInfo->atMppBuffer[i] ||
                !ptRgaInfo->atMppFrame[i])
        {
            continue;
        }

        iW = mpp_frame_get_width(ptRgaInfo->atMppFrame[i]);
        iH = mpp_frame_get_height(ptRgaInfo->atMppFrame[i]);
        if(w == iW && h == iH)
        {
            return i;
        }
    }
    return -1;
}

int addMppFrame(T_RGA_INFO *ptRgaInfo, int w, int h)
{
    int i = 0;
    int iRet = 0;
    int idstSize = 0;
    //w = MPP_ALIGN(w, 16);
    //h = MPP_ALIGN(h, 16);
    idstSize = w * h * 3;
    for (i = 0; i < MAX_MPP_FRAME_NUM; i++)
    {
        if(ptRgaInfo->atMppBuffer[i] && ptRgaInfo->atMppFrame[i])
        {
            continue;
        }
        iRet = mpp_buffer_get(NULL, &ptRgaInfo->atMppBuffer[i], idstSize);
        if (iRet)
        {
            printf("failed to get dst buffer %d with size %d\n", iRet, idstSize);
            return -1;
        }


        iRet = mpp_frame_init(&ptRgaInfo->atMppFrame[i]);
        if (iRet) 
        {
            printf("failed to init src frame\n");
            return -1;
        }

        
        mpp_frame_set_buffer(ptRgaInfo->atMppFrame[i], ptRgaInfo->atMppBuffer[i]);
        mpp_frame_set_width(ptRgaInfo->atMppFrame[i],  w);
        mpp_frame_set_height(ptRgaInfo->atMppFrame[i], h);
        mpp_frame_set_hor_stride(ptRgaInfo->atMppFrame[i], w);
        mpp_frame_set_ver_stride(ptRgaInfo->atMppFrame[i], h);
        mpp_frame_set_fmt(ptRgaInfo->atMppFrame[i], MPP_FMT_YUV420SP);

        //mpp_frame_set_fmt(ptRgaInfo->atMppFrame[i], MPP_FMT_ARGB8888);
        //mpp_frame_set_fmt(ptRgaInfo->atMppFrame[i], MPP_FMT_ABGR8888);//no
        //mpp_frame_set_fmt(ptRgaInfo->atMppFrame[i], MPP_FMT_BGRA8888);//no
        //mpp_frame_set_fmt(ptRgaInfo->atMppFrame[i], MPP_FMT_RGBA8888);//no
        return i;
    }
    return -1;
}

int freeMppFrame(T_RGA_INFO *ptRgaInfo)
{
    int i = 0;    
    for (i = 0; i < MAX_MPP_FRAME_NUM; i++)
    {
        if (ptRgaInfo->atMppFrame[i])
        {
            mpp_frame_deinit(&ptRgaInfo->atMppFrame[i]);
            ptRgaInfo->atMppFrame[i] = NULL;
        }
        if (ptRgaInfo->atMppBuffer[i])
        {
            mpp_buffer_put(ptRgaInfo->atMppBuffer[i]);
            ptRgaInfo->atMppBuffer[i] = NULL;
        }
    }    
    return 0;
}

RGA_HANDLE rga_create(int iWidth, int iHeight)
{
    int iRet = 0;
    T_RGA_INFO *ptRgaInfo = NULL;
    ptRgaInfo = (T_RGA_INFO *)malloc(sizeof(T_RGA_INFO));
    if (NULL == ptRgaInfo)
    {
        return 0;	
    }

    memset((char *)ptRgaInfo, 0, sizeof(T_RGA_INFO));
    iRet = rga_init(&ptRgaInfo->ctx);
    if (iRet) 
    {
        printf("init rga context failed %d\n", iRet);
        free(ptRgaInfo);
        return 0;
    }

    ptRgaInfo->iFrameIndex = 0;

    iWidth = MPP_ALIGN(iWidth, 16);
    iHeight = MPP_ALIGN(iHeight, 16);

    iRet = rga_set_cur_frame(ptRgaInfo, iWidth, iHeight);

    if(iRet < 0)
    {
        printf("rga_set_cur_frame err %d\n", iRet);
        free(ptRgaInfo);
        return 0;
    }

    return (RGA_HANDLE)ptRgaInfo;
}

int rga_destroy(RGA_HANDLE hRga)
{
    int iRet = 0;
    T_RGA_INFO *ptRgaInfo = (T_RGA_INFO *)hRga;
    if (NULL == ptRgaInfo)
    {
        return -1;	
    }
    iRet = rga_deinit(ptRgaInfo->ctx);
    if (iRet) {
        printf("deinit rga context failed %d\n", iRet);
    }    
    freeMppFrame(ptRgaInfo);
    free(ptRgaInfo);
    return 0;
}

int rga_blit(RGA_HANDLE hRga, MppFrame tSrcMppFrame, MppFrame *tDstFrame)
{
    int iRet = 0;
    MppFrame tDstMppFrame = NULL;
    T_RGA_INFO *ptRgaInfo = (T_RGA_INFO *)hRga;
    if (NULL == ptRgaInfo)
    {
        return -1;	
    }

    tDstMppFrame = ptRgaInfo->atMppFrame[ptRgaInfo->iFrameIndex];
    if(tDstMppFrame == NULL)
    {
        printf("rga frame err \n");
        return -1;
    }

    // start copy process
    iRet = rga_control(ptRgaInfo->ctx, RGA_CMD_INIT, NULL);
    if (iRet) {
        printf("rga cmd init failed %d\n", iRet);
        goto END;
    }

    iRet = rga_control(ptRgaInfo->ctx, RGA_CMD_SET_SRC, tSrcMppFrame);
    if (iRet) {
        printf("rga cmd setup source failed %d\n", iRet);
        goto END;
    }

    iRet = rga_control(ptRgaInfo->ctx, RGA_CMD_SET_DST, tDstMppFrame);
    if (iRet) {
        printf("rga cmd setup destination failed %d\n", iRet);
        goto END;
    }

    iRet = rga_control(ptRgaInfo->ctx, RGA_CMD_RUN_SYNC, NULL);
    if (iRet) {
        printf("rga cmd process copy failed %d\n", iRet);
        goto END;
    }
    *tDstFrame = tDstMppFrame;
    return 0;

END:
    return -1;
}

int rga_set_cur_frame(RGA_HANDLE hRga, int iWidth, int iHeight)
{
    int iRet = 0;
    T_RGA_INFO *ptRgaInfo = (T_RGA_INFO *)hRga;
    if (NULL == ptRgaInfo)
    {
        return -1;
    }
    iRet = getMppFrame(ptRgaInfo, iWidth, iHeight);

    if(iRet < 0)
    {
        iRet = addMppFrame(ptRgaInfo, iWidth, iHeight);

    }
    if(iRet < 0)
    {
        return -1;
    }

    ptRgaInfo->iFrameIndex = iRet;

//    printf("ptRgaInfo->iFrameIndex :%d, %d,%d \n", iWidth, iHeight, ptRgaInfo->iFrameIndex);
    return 0;
}
