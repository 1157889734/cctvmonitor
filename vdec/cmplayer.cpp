
#include <list>
#include "mutex.h"
#include "vdec.h"
#include "./debugout/debug.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rtsp/rtspApi.h"
#include "rtsp/rtspComm.h"
#include <stdarg.h>
#include "cmplayer.h"
#include <QDebug>
#include "shm.h"
typedef void* SHM_HANDLE;


struct SVideoCmdMessage{
    int  nType;//0 Play 1 Preview
    long lCmd;
    double dValue;
    SVideoCmdMessage()
    {
        nType = 1;
        lCmd = 0;
        dValue = 0;
    }
    SVideoCmdMessage(long lp1,double lp2,int np3){
        nType = np3;
        lCmd = lp1;
        dValue = lp2;
    }
    SVideoCmdMessage& operator=(SVideoCmdMessage& value)
    {
        nType = value.nType;
        lCmd = value.lCmd;
        dValue = value.dValue;
        return *this;
    }
};

#define PLAY_STREAM_TYPE_PREVIEW  0
#define PLAY_STREAM_TYPE_PLAYBACK 1

typedef struct _T_CMP_PLAYER_INFO
{
    char			acUrl[256];
    int 			iReconnectFlag;
    int				iThreadRunFlag;
    int             iThreadExitFlag;
    void 			*pUserInfo;
    VDEC_HADNDLE	VHandle;
    pthread_t			hPlayThread;
    CMPPLAY_STATE   ePlayState;  //播放状态
    int				iPlaySecs;           //播放了的时间（单位豪秒）
    int 			iAudioFlag;
    int				iPlayStreamType;     // 0: preview, 1: playback
    int				iTcpFlag;
    std::list<SVideoCmdMessage> lstMsg;	//
    CMutexLock		stcSecLock;
    int				iPlayRange;			 //视频时长（单位秒）
    HANDLE			hOpenMediaEvent;
    int				iOpenMediaState;
    int				iPlaySpeed;
    int             iGetFrameFlag;   //获取到帧数据的标识
    int             iStreamState; //是否有码流状态，0-无码流，1-有码流
    timeval	   	    tPrevFrameTs;
    int 			iIgnoreFrameNum;
    int				iRtspHeartCount;
    unsigned int	uiPrevFrameTs;
    unsigned int	uiPlayBaseTime;
    CMP_VDEC_TYPE   eDecType;   //软、硬解码或者鱼眼矫正
    T_WND_INFO      ptWndInfo;
    int             iDisplayFlag;
} T_CMP_PLAYER_INFO, *PT_CMP_PLAYER_INFO;

static int				 g_iCMPlayerInitFlag = 0;
static QWidget          *s_pFullScreenWidget = NULL;
static SHM_HANDLE        s_pFullScreenHandle = NULL;
static SHM_HANDLE        s_pBackuphandle = NULL;

static int WinSocketInit(void)
{
#ifdef _WIN32
	WSADATA	wsaData;
	int nRet = WSAStartup(0x202, &wsaData);
	if(nRet != 0)
	{
		WSACleanup();
		return -1;
	}	
#endif // _WIN32
	return 0;
}
static int WinSocketUninit(void)
{

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}


static void PushMessage(PT_CMP_PLAYER_INFO ptCmpPlayer, long lParm1,double dParm2,int nType = 0)
{
    if (NULL == ptCmpPlayer)
    {
        return ;
    }
    ptCmpPlayer->stcSecLock.Lock();
    SVideoCmdMessage sCmdmsg(lParm1,dParm2,nType);
    ptCmpPlayer->lstMsg.push_back(sCmdmsg);
    ptCmpPlayer->stcSecLock.Unlock();
}

static int GetMessage(PT_CMP_PLAYER_INFO ptCmpPlayer, long &lParm1,double &dParm2,int& nType)
{
    if (NULL == ptCmpPlayer)
    {
        return 0;
    }
    if(ptCmpPlayer->lstMsg.size()<1)
    {
        return 0;
    }
    ptCmpPlayer->stcSecLock.Lock();
    SVideoCmdMessage workmsg = *(ptCmpPlayer->lstMsg.begin());
    lParm1 = workmsg.lCmd;
    dParm2 = workmsg.dValue;
    nType = workmsg.nType;
    ptCmpPlayer->lstMsg.erase(ptCmpPlayer->lstMsg.begin());
    ptCmpPlayer->stcSecLock.Unlock();
    return 1;
}

static void InitMessgeList(PT_CMP_PLAYER_INFO ptCmpPlayer)
{
    if (NULL == ptCmpPlayer)
    {
        return ;
    }
    ptCmpPlayer->stcSecLock.Lock();
    ptCmpPlayer->lstMsg.clear();
    ptCmpPlayer->stcSecLock.Unlock();
}

void SetPlayState(CMPHandle hPlay, CMPPLAY_STATE ePlayState)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return ;
    }

    ptCmpPlayer->ePlayState = ePlayState;
}


int GetPlayState(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return -1;
    }

    return ptCmpPlayer->ePlayState;
}

#define TIME2MSEC(x) ((x.tv_sec) *1000 + (x.tv_usec) /1000)
static int RtpSetDataCallBack(int iFrameType, int iStreamType, char *pcFrame, int iFrameLen, unsigned int uiTs, void *pUserData)
{
    int iRet = 0;
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)pUserData;

    if(NULL ==  ptCmpPlayer )
    {
        return -1;
    }

    if((NULL == ptCmpPlayer->VHandle) && (STREAM_TYPE_VIDEO == iFrameType))
    {
        if(E_STREAM_TYPE_H265 == iStreamType)
        {
            ptCmpPlayer->VHandle = VDEC_CreateVideoDecCh(&ptCmpPlayer->ptWndInfo,1920, 1080,
                                                         ptCmpPlayer->eDecType, H265_CODE);
        }
        else if(E_STREAM_TYPE_H264 == iStreamType)
        {
            ptCmpPlayer->VHandle = VDEC_CreateVideoDecCh(&ptCmpPlayer->ptWndInfo,1920, 1080,
                                                         ptCmpPlayer->eDecType, H264_CODE);
        }

        if(ptCmpPlayer->ePlayState ==  CMP_STATE_PLAY && ptCmpPlayer->VHandle)
        {
            VDEC_DisplayEnable(ptCmpPlayer->VHandle, ptCmpPlayer->iDisplayFlag);
            VDEC_StartPlayStream(ptCmpPlayer->VHandle);
        }
        else
        {
            return -1;
        }
    }

    if (STREAM_TYPE_VIDEO == iFrameType)
    {
        PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)pUserData;

        ptCmpPlayer->iGetFrameFlag = 1;   //拿到rtp数据，将获取到帧数据的标识置1
        ptCmpPlayer->iRtspHeartCount = 0;   //拿到rtp数据，将计数清0，表示有流了重新进行计数监控
        ptCmpPlayer->iStreamState = 1;   //拿到rtp数据，则将码流状态置1，表示有流了

        if ((PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType) && (0 == ptCmpPlayer->iIgnoreFrameNum))
        {
            unsigned int uiNpt = 0;

            if ((ptCmpPlayer->uiPrevFrameTs != 0) && (uiTs >= ptCmpPlayer->uiPrevFrameTs))
            {
                uiNpt = (uiTs - ptCmpPlayer->uiPrevFrameTs) / 90;
                if (uiNpt > 5000) //(40 * 25 * 5)
                {
                    uiNpt = 40;
                }

            }
            else if (0 == ptCmpPlayer->uiPrevFrameTs)
            {
                uiNpt = 0;
            }
            else
            {
                uiNpt = 40;
            }

            if (CMP_STATE_FAST_FORWARD == ptCmpPlayer->ePlayState)
            {
                ptCmpPlayer->iPlaySecs += uiNpt;// * ptCmpPlayer->iPlaySpeed;
            }
            else if (CMP_STATE_SLOW_FORWARD == ptCmpPlayer->ePlayState)
            {
                ptCmpPlayer->iPlaySecs += uiNpt;// / ptCmpPlayer->iPlaySpeed;
            }
            else
            {
                ptCmpPlayer->iPlaySecs += uiNpt;
            }

            ptCmpPlayer->uiPrevFrameTs = uiTs;

            if (ptCmpPlayer->iPlaySecs / 1000 >= ptCmpPlayer->iPlayRange)
            {
                ptCmpPlayer->iPlaySecs = (ptCmpPlayer->iPlayRange - 1) * 1000;
            }
        }
        else
        {
            ptCmpPlayer->uiPrevFrameTs = uiTs;
        }

        if (ptCmpPlayer->iIgnoreFrameNum > 0)
        {
            ptCmpPlayer->iIgnoreFrameNum --;
        }
        VDEC_SendVideoStream(ptCmpPlayer->VHandle, (char *)pcFrame, iFrameLen, 0);
    }

    return iRet;
}
static int ParseRtspUrl(char *pcRawUrl, char *pcUrl, char *pcUser, char *pcPasswd)
{
    char acStr[256];
    char *pcTmp = NULL;
    char *pcNextTmp = NULL;
    char *pcPos = NULL;
    char *pcContent = NULL;
    char *pcIpaddr = NULL;


    if ((NULL == pcRawUrl) || (NULL == pcUrl) || (NULL == pcUser) || (NULL == pcUser))
    {
        return -1;
    }
    memset(acStr, 0, sizeof(acStr));
    strncpy(acStr, pcRawUrl, sizeof(acStr));

    /* rtsp:// */

    pcContent = acStr + 7;

    pcTmp = strtok_s(pcContent, "/",&pcNextTmp);

    if (pcTmp)
    {
        pcIpaddr = pcTmp;
        pcContent = pcNextTmp;
        if(strstr(pcIpaddr, "@"))
        {
            pcTmp = strtok_s(pcIpaddr, "@",&pcNextTmp);
            pcIpaddr = pcNextTmp;
        }
        else
        {
            pcTmp = NULL;
        }
    }

    if (pcTmp)
    {
        pcPos = pcTmp;
        pcTmp = strtok_s(pcPos, ":",&pcNextTmp);

        if (pcTmp)
        {
            strcpy(pcUser, pcTmp);
        }

        if (pcNextTmp)
        {
            strcpy(pcPasswd, pcNextTmp);
        }

    }

    if (NULL == pcIpaddr)
    {
        return -1;
    }

    if (pcContent)
    {
        sprintf(pcUrl, "rtsp://%s/%s", pcIpaddr, pcContent);
    }
    else
    {
        sprintf(pcUrl, "rtsp://%s", pcIpaddr);
    }


    return 0;
}

void* MonitorPlayThread(void *arg)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)arg;
    RTSP_HANDLE RHandle = 0;
    int iTimeout = 0;
    int iRet = 0;
    char acUrl[256] = {0}, acUserName[64] = {0}, acPassWd[64] = {0};
    int iRunCount = 0;
    int iReconnectFlag = 1;
    int iRtpProtocol = ptCmpPlayer->iTcpFlag;
    int iFreshCount = 0;
    int iWaitCount = 0;

    long lCmd = 0;
    double dValue = 0;
    int iType = 0;

    pthread_detach(pthread_self());

    if (NULL == arg)
    {
        return NULL;
    }

    memset(acUrl, 0, sizeof(acUrl));
    memset(acUserName, 0, 64);
    memset(acPassWd, 0, 64);
    ParseRtspUrl(ptCmpPlayer->acUrl, acUrl, acUserName, acPassWd);
    printf("acUserName:%s, acPassWd:%s \n", acUserName, acPassWd);

    while (ptCmpPlayer->iThreadRunFlag && iReconnectFlag)
    {
        if (RHandle)
        {
            if(ptCmpPlayer->iThreadRunFlag == 0)
            {
                break;
            }
            iRet = RTSP_CloseStream(RHandle, 2);
            RTSP_MSleep(50);
            iRet = RTSP_Logout(RHandle);
            RHandle = 0;
            iWaitCount = 0;
            while(ptCmpPlayer->iThreadRunFlag && (iWaitCount < 100))
            {
                iWaitCount++;
                RTSP_MSleep(50);
            }
        }

        RHandle = RTSP_Login(acUrl, acUserName, acPassWd);
        if (RHandle > 0)
        {
            //ptCmpPlayer->RHandle = RHandle;
            RTSP_GetParam(RHandle, E_TYPE_PLAY_RANGE, (void *)&ptCmpPlayer->iPlayRange);
            iRet =  RTSP_OpenStream(RHandle, 0, iRtpProtocol, (void *)RtpSetDataCallBack, (void *)ptCmpPlayer);

            if (iRet < 0)
            {
                if(ptCmpPlayer->iThreadRunFlag == 0)
                {
                    break;
                }
                DebugPrint(DEBUG_CMPLAYER_ERROR_PRINT, "RTSP_OpenStream error!iRet=%d\n",iRet);

                RTSP_Logout(RHandle);
                RHandle = 0;
                if(0 == iFreshCount)
                {
                    //pPlayWin=(Fl_Group *)ptCmpPlayer->pWinHandle;
                    //VDEC_Setfb1BKColor(ptCmpPlayer->VHandle,pPlayWin->x(),pPlayWin->y(),pPlayWin->w(),pPlayWin->h());
                }
                iFreshCount ++;
                iFreshCount = iFreshCount%20;
                iWaitCount = 0;
                while(ptCmpPlayer->iThreadRunFlag && (iWaitCount < 100))
                {
                    iWaitCount++;
                    RTSP_MSleep(50);
                }
                continue;
            }

            iRet =  RTSP_PlayControl(RHandle, E_PLAY_STATE_PLAY, 0);

            if (iRet < 0)
            {
                if(0 == iFreshCount)
                {
                    //pPlayWin=(Fl_Group *)ptCmpPlayer->pWinHandle;
                    //VDEC_Setfb1BKColor(ptCmpPlayer->VHandle,pPlayWin->x(),pPlayWin->y(),pPlayWin->w(),pPlayWin->h());
                }
                iFreshCount ++;
                iFreshCount = iFreshCount%20;
                continue;
            }
            //ptCmpPlayer->iIFrameFlag = 1;
            //SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);

            RTSP_MSleep(50);

            iTimeout = RTSP_GetRtspTimeout(RHandle);
            if (0 == iTimeout)
            {
                iTimeout = 60;
            }
            iFreshCount = 0;
        }
        else
        {
//            printf("--rtsp_login error--%s!\n", ptCmpPlayer->acUrl);
            DebugPrint(DEBUG_CMPLAYER_ERROR_PRINT, "rtsp_login %s error!\n", ptCmpPlayer->acUrl);
            if (PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType)
            {
                iReconnectFlag = 0;
            }
            if(0 == iFreshCount)
            {
                 //pPlayWin=(Fl_Group *)ptCmpPlayer->pWinHandle;
                 //VDEC_Setfb1BKColor(ptCmpPlayer->VHandle,pPlayWin->x(),pPlayWin->y(),pPlayWin->w(),pPlayWin->h());
            }
            iFreshCount ++;
            iFreshCount = iFreshCount%20;
            RTSP_MSleep(50);
            continue;
        }

        while (ptCmpPlayer->iThreadRunFlag)
        {
            iRet = GetMessage(ptCmpPlayer, lCmd, dValue, iType);
            if (iRet > 0)
            {
                iRet = RTSP_PlayControl(RHandle, lCmd, dValue);
                DebugPrint(DEBUG_CMPLAYER_NORMAL_PRINT, "MonitorPlayThread get playctrl cmd:%d, value=%lf\n", lCmd, dValue);
                if (iRet < 0)
                {
                    DebugPrint(DEBUG_CMPLAYER_ERROR_PRINT, "MonitorPlayThread  RTSP_PlayControl error! err=%d\n",iRet);
                }
                RTSP_MSleep(50);
            }

            ptCmpPlayer->iRtspHeartCount++;

            if (PLAY_STREAM_TYPE_PREVIEW == ptCmpPlayer->iPlayStreamType)
            {
                if (ptCmpPlayer->iRtspHeartCount >= 40)
                {
                    ptCmpPlayer->iStreamState = 0;
                    ptCmpPlayer->iRtspHeartCount = 0;
                    break;
                }
            }
            else
            {
                if (ptCmpPlayer->ePlayState != CMP_STATE_PAUSE)
                {
                    if ((ptCmpPlayer->iPlaySecs >= (ptCmpPlayer->iPlayRange - 30)*1000) && (ptCmpPlayer->iPlaySecs <= ptCmpPlayer->iPlayRange*1000))
                    {
                        if (ptCmpPlayer->iRtspHeartCount >= 20)
                        {
                            ptCmpPlayer->iStreamState = 0;
                            iReconnectFlag = 0;
                            break;
                        }
                    }
                    else
                    {
                        if (ptCmpPlayer->iRtspHeartCount >= 100)
                        {
                            ptCmpPlayer->iStreamState = 0;
                            iReconnectFlag = 0;
                            break;
                        }
                    }
                }
            }

            if (0 == iRunCount)
            {
                if(RHandle == NULL)
                {
                    break;
                }
                iRunCount = iTimeout * 2;
                iRet = RTSP_SendHeart(RHandle);
                if (iRet < 0)
                {
                    DebugPrint(DEBUG_CMPLAYER_ERROR_PRINT, "send heart failed!\n");
                    if (PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType)
                    {
                        iReconnectFlag = 0;
                    }
                    ptCmpPlayer->iStreamState = 0;
                    break;
                }
            }
            iRunCount--;

            RTSP_MSleep(50);
        }
    }

    if (PLAY_STREAM_TYPE_PLAYBACK == ptCmpPlayer->iPlayStreamType)
    {
        ptCmpPlayer->iPlaySecs = (ptCmpPlayer->iPlayRange) * 1000;
    }

    SetPlayState(ptCmpPlayer, CMP_STATE_STOP);

    RTSP_CBFunEnable(RHandle, 0);
    RTSP_MSleep(1);
    ptCmpPlayer->iThreadExitFlag = 1;
    if (RHandle)
    {
        iRet = RTSP_CloseStream(RHandle, 2);
        iRet = RTSP_Logout(RHandle);
        RHandle = 0;
    }


    printf("aMonitorPlayThread exit \n");

    return NULL;
}

CMPPlayer_API CMPHandle CMP_Init(T_WND_INFO *pWndInfo, CMP_VDEC_TYPE eDecType)
{
    if(pWndInfo == NULL)
    {
        return NULL;
    }
    PT_CMP_PLAYER_INFO ptCmpPlayer = NULL;
    ptCmpPlayer = new T_CMP_PLAYER_INFO;

    if (NULL == ptCmpPlayer)
    {
        return NULL;
    }

    ptCmpPlayer->iPlayRange = 0;
    if (0 == g_iCMPlayerInitFlag)
    {
        g_iCMPlayerInitFlag = 1;
        WinSocketInit();
        VDEC_Init();;
        SHM_Init();
    }

    memset(ptCmpPlayer->acUrl,0,sizeof(ptCmpPlayer->acUrl));
    ptCmpPlayer->ePlayState			= CMP_STATE_IDLE;
    ptCmpPlayer->iReconnectFlag		= 0;
    ptCmpPlayer->iPlaySecs = 0;
    ptCmpPlayer->iPlaySpeed = 1;
    ptCmpPlayer->iStreamState = 0;
    ptCmpPlayer->iGetFrameFlag = 0;

    ptCmpPlayer->iIgnoreFrameNum = 0;
    ptCmpPlayer->hOpenMediaEvent = NULL;
    ptCmpPlayer->iOpenMediaState  = CMP_OPEN_MEDIA_FAIL;
    ptCmpPlayer->hPlayThread = NULL;
    memset(&ptCmpPlayer->tPrevFrameTs,0,sizeof(timeval));
    ptCmpPlayer->iRtspHeartCount = 0;
    ptCmpPlayer->uiPrevFrameTs  = 0;
    ptCmpPlayer->uiPlayBaseTime  = 0;

    ptCmpPlayer->VHandle = NULL;
    ptCmpPlayer->eDecType = eDecType;
    ptCmpPlayer->iDisplayFlag = 0;

    ptCmpPlayer->ptWndInfo = *pWndInfo;
    ptCmpPlayer->ptWndInfo.pRenderHandle = SHM_AddRect((QWidget*)pWndInfo->hWnd);

    InitMessgeList(ptCmpPlayer);

    return (CMPHandle)ptCmpPlayer;
}

CMPPlayer_API int CMP_UnInit(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return -1;
    }

    SHM_FreeRect(ptCmpPlayer->ptWndInfo.pRenderHandle);
    ptCmpPlayer->ptWndInfo.pRenderHandle = NULL;

    InitMessgeList(ptCmpPlayer);
    delete ptCmpPlayer;
    ptCmpPlayer=NULL;
    return 0;
}

CMPPlayer_API int CMP_OpenMediaPreview(CMPHandle hPlay, const char *pcRtspUrl, int iTcpFlag)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        printf("ptCmpPlayer == NULL \n");
        return -1;
    }

    ptCmpPlayer->iPlayStreamType = PLAY_STREAM_TYPE_PREVIEW;
    ptCmpPlayer->iReconnectFlag = 1;
    sprintf(ptCmpPlayer->acUrl,  "%s", pcRtspUrl);
    ptCmpPlayer->iThreadRunFlag = 1;
    ptCmpPlayer->iThreadExitFlag = 0;
    ptCmpPlayer->iTcpFlag = iTcpFlag;
    ptCmpPlayer->iPlaySecs = 0;
    InitMessgeList(ptCmpPlayer);
    SetPlayState(ptCmpPlayer, CMP_STATE_IDLE);

    pthread_create(&ptCmpPlayer->hPlayThread, NULL, MonitorPlayThread, ptCmpPlayer);
    printf("CMP_OpenMediaPreview = %s",ptCmpPlayer->acUrl);

    return 0;
}


CMPPlayer_API int CMP_OpenMediaFile(CMPHandle hPlay, const char *pcRtspFile, int iTcpFlag)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return -1;
    }

    ptCmpPlayer->iPlayStreamType = PLAY_STREAM_TYPE_PLAYBACK;
    ptCmpPlayer->iReconnectFlag = 0;
    sprintf(ptCmpPlayer->acUrl,  "%s", pcRtspFile);
    ptCmpPlayer->iThreadRunFlag = 1;
    ptCmpPlayer->iThreadExitFlag = 0;
    ptCmpPlayer->iTcpFlag = iTcpFlag;
    ptCmpPlayer->iOpenMediaState = CMP_OPEN_MEDIA_UNKOWN;
    ptCmpPlayer->iPlaySecs = 0;
    InitMessgeList(ptCmpPlayer);
    SetPlayState(ptCmpPlayer, CMP_STATE_IDLE);

    pthread_create(&ptCmpPlayer->hPlayThread, NULL, MonitorPlayThread, ptCmpPlayer);

    printf("CMP_OpenMediaFile = %s",ptCmpPlayer->acUrl);

    return 0;
}

CMPPlayer_API int CMP_CloseMedia(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return -1;
    }
    VDEC_StopPlayStream(ptCmpPlayer->VHandle);
    ptCmpPlayer->iThreadRunFlag = 0;
//    RTSP_MSleep(1000); //

    if (ptCmpPlayer->hPlayThread)
    {
        while (0 == ptCmpPlayer->iThreadExitFlag)
        {
            RTSP_MSleep(15);
        }
        ptCmpPlayer->hPlayThread = NULL;
    }

    VDEC_DestroyVideoDecCh(ptCmpPlayer->VHandle);
    ptCmpPlayer->VHandle = NULL;
    SetPlayState(ptCmpPlayer, CMP_STATE_STOP);
    printf("CMP_CloseMedia\n");
    return 0;
}

CMPPLAY_STATE CMP_GetPlayStatus(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return CMP_STATE_ERROR;
    }

    return ptCmpPlayer->ePlayState;
}


CMPPlayer_API int CMP_GetPlayRange(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return -1;
    }
    return ptCmpPlayer->iPlayRange;
}

CMPPlayer_API int CMP_GetPlayTime(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return 0;
    }

    return ptCmpPlayer->iPlaySecs/1000;
}




CMPPlayer_API int CMP_PlayMedia(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
    if (NULL == ptCmpPlayer)
    {
        return -1;
    }

    ePlayState = CMP_GetPlayStatus(hPlay);
    if (ePlayState ==  CMP_STATE_ERROR)
    {
        return -1;
    }
    if (ePlayState == CMP_STATE_PLAY )
    {
        return 0;
    }

    PushMessage(ptCmpPlayer, E_PLAY_STATE_PLAY, 0);
    SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);
    VDEC_StartPlayStream(ptCmpPlayer->VHandle);
    ptCmpPlayer->iPlaySpeed = 1;
    return 0;
}

CMPPlayer_API int CMP_PauseMedia(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
    if (NULL == ptCmpPlayer)
    {
        return -1;
    }
    ePlayState = CMP_GetPlayStatus(hPlay);
    if (ePlayState ==  CMP_STATE_ERROR)
    {
        return -1;
    }
    if (ePlayState == CMP_STATE_PAUSE )
    {
        return 0;
    }
    SetPlayState(ptCmpPlayer, CMP_STATE_PAUSE);
    PushMessage(ptCmpPlayer, E_PLAY_STATE_PAUSE, 0);
    VDEC_PausePlayStream(ptCmpPlayer->VHandle);

    return 0;
}

CMPPlayer_API int CMP_SetPosition(CMPHandle hPlay, int64_t nPosTime)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
    double dValue = nPosTime;

    if (NULL == ptCmpPlayer)
    {
        return -1;
    }

    ePlayState = CMP_GetPlayStatus(hPlay);
    if (CMP_STATE_ERROR != ePlayState)
    {
        PushMessage(ptCmpPlayer, E_PLAY_STATE_DRAG_POS, dValue);
        SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);
        ptCmpPlayer->iPlaySpeed = 1;
        ptCmpPlayer->iIgnoreFrameNum = 10;
        ptCmpPlayer->iPlaySecs = nPosTime *1000;
    }

    return 0;
}

CMPPlayer_API int CMP_SetPlaySpeed(CMPHandle hPlay, double dSpeed)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    CMPPLAY_STATE ePlayState = CMP_STATE_IDLE;
    if (NULL == ptCmpPlayer)
    {
        qDebug()<<"***********NULL == ptCmpPlayer*****"<<__LINE__<<endl;
        return -1;
    }
    ePlayState = CMP_GetPlayStatus(hPlay);
    if (ePlayState ==  CMP_STATE_ERROR)
    {
        qDebug()<<"***********ePlayState ==  CMP_STATE_ERROR*****"<<__LINE__<<endl;
        return -1;
    }

    PushMessage(ptCmpPlayer, E_PLAY_STATE_FAST_FORWARD, dSpeed);


    DebugPrint(DEBUG_CMPLAYER_ERROR_PRINT,"[%s %d] CMP_SetPlaySpeed dSpeed = %.2f",__FUNCTION__, __LINE__, dSpeed);

    if (dSpeed > 0.11 && dSpeed < 0.13)
    {
        SetPlayState(ptCmpPlayer, CMP_STATE_SLOW_FORWARD);
        ptCmpPlayer->iPlaySpeed = 8;
    }
    else if (dSpeed > 0.24 && dSpeed < 0.26)
    {
        SetPlayState(ptCmpPlayer, CMP_STATE_SLOW_FORWARD);
        ptCmpPlayer->iPlaySpeed = 4;
    }
    else if (dSpeed > 0.49 && dSpeed < 0.51)
    {
        SetPlayState(ptCmpPlayer, CMP_STATE_SLOW_FORWARD);
        ptCmpPlayer->iPlaySpeed = 2;
    }
    else if (dSpeed > 1.9 && dSpeed < 2.1)
    {
        SetPlayState(ptCmpPlayer, CMP_STATE_FAST_FORWARD);
        ptCmpPlayer->iPlaySpeed = 2;
    }
    else if (dSpeed > 3.9 && dSpeed < 4.1)
    {
        SetPlayState(ptCmpPlayer, CMP_STATE_FAST_FORWARD);
        ptCmpPlayer->iPlaySpeed = 4;
    }
    else if(dSpeed < 1.1 && dSpeed > 0.9)
    {
        SetPlayState(ptCmpPlayer, CMP_STATE_PLAY);
        ptCmpPlayer->iPlaySpeed = 1;
    }

    return 0;
}

CMPPlayer_API int CMP_SetVolume(CMPHandle hPlay, int nVolume)
{
    return 0;
}

CMPPlayer_API int CMP_ChangeWnd(CMPHandle hPlay,const T_WND_INFO *pWndInfo)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    if (NULL == ptCmpPlayer)
    {
        return -1;
    }
    QWidget *parent = (QWidget*)pWndInfo->hWnd;
    if(parent == NULL)
    {
        ptCmpPlayer->ptWndInfo.pRenderHandle = s_pBackuphandle;
        SHM_DetchWnd(s_pFullScreenHandle);
        delete s_pFullScreenWidget;
        s_pFullScreenWidget = NULL;
        s_pBackuphandle = NULL;
    }
    else
    {
        if(s_pFullScreenWidget == NULL)
        {
            s_pFullScreenWidget = new QWidget(parent);
            s_pFullScreenWidget->setGeometry(0, 0, parent->width(), parent->height());
            s_pFullScreenHandle = SHM_AddRect(s_pFullScreenWidget);

        }
        s_pFullScreenWidget->show();
        SHM_AttchWnd(s_pFullScreenHandle);

        s_pBackuphandle = ptCmpPlayer->ptWndInfo.pRenderHandle;
        ptCmpPlayer->ptWndInfo.pRenderHandle = s_pFullScreenHandle;
    }


    return 1;
}

CMPPlayer_API int CMP_SetPlayEnable(CMPHandle hPlay, int enable)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    if (NULL == ptCmpPlayer)
    {
        return -1;
    }

    if(enable)
    {
        printf("*****************%s---%d\n",__FUNCTION__,__LINE__);
        SHM_AttchWnd(ptCmpPlayer->ptWndInfo.pRenderHandle);
        printf("*****************%s---%d\n",__FUNCTION__,__LINE__);
        if(ptCmpPlayer->VHandle)
        {
            printf("*****************%s---%d\n",__FUNCTION__,__LINE__);
            VDEC_DisplayEnable(ptCmpPlayer->VHandle, enable);
            printf("*****************%s---%d\n",__FUNCTION__,__LINE__);

        }
    }
    else
    {
        if(ptCmpPlayer->VHandle)
        {
            VDEC_DisplayEnable(ptCmpPlayer->VHandle, enable);
        }
        usleep(20000);
        SHM_DetchWnd(ptCmpPlayer->ptWndInfo.pRenderHandle);
    }
    ptCmpPlayer->iDisplayFlag = enable;
    return 1;
}


CMPPlayer_API int CMP_SetPlayState(CMPHandle hPlay, int iState)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    if(ptCmpPlayer == NULL)
    {
        return -1;
    }

    if(CMP_STATE_PLAY == iState)
    {
        VDEC_StartPlayStream(ptCmpPlayer->VHandle);
    }
    else
    {
        VDEC_PausePlayStream(ptCmpPlayer->VHandle);
    }

    return 0;
}

CMPPlayer_API int CMP_FillDisplayBk(CMPHandle hPlay, uint32_t rgb)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;
    if(ptCmpPlayer == NULL)
    {
        return -1;
    }
    SHM_FillRect(ptCmpPlayer->ptWndInfo.pRenderHandle, rgb);
}

CMPPlayer_API int CMP_GetOpenMediaState(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        return NULL;
    }
    return ptCmpPlayer->iOpenMediaState;
}

CMPPlayer_API int CMP_GetStreamState(CMPHandle hPlay)
{
    PT_CMP_PLAYER_INFO ptCmpPlayer = (PT_CMP_PLAYER_INFO)hPlay;

    if (NULL == ptCmpPlayer)
    {
        DebugPrint(DEBUG_CMPLAYER_ERROR_PRINT, "CMP_GetDecState error! media is not exist\n");
        return 0;
    }
    return ptCmpPlayer->iStreamState;
}
