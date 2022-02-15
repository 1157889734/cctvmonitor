#ifndef _RTSP_API_H_
#define _RTSP_API_H_

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */


typedef  unsigned long RTSP_HANDLE ;

typedef enum _E_PLAY_STATE
{
    E_PLAY_STATE_PLAY,
    E_PLAY_STATE_PAUSE,
    E_PLAY_STATE_FAST_FORWARD,
    E_PLAY_STATE_FAST_REVERSE,
    E_PLAY_STATE_DRAG_POS,
	E_PLAY_STATE_STOP
} E_PLAY_STATE;

enum _E_PARAM_TYPE
{
	E_TYPE_PLAY_RANGE = 1,
};

RTSP_HANDLE RTSP_Login(const char *pUrl, char *pcUser, char *pcPasswd);
int RTSP_Logout(RTSP_HANDLE RHandle);
int RTSP_OpenStream(RTSP_HANDLE RHandle, int iCh, int iRtpTransportProtocol, void *pfRtpSetDataCB, void *pUserData);
int RTSP_CloseStream(RTSP_HANDLE RHandle, int iCh);
int RTSP_PlayControl(RTSP_HANDLE RHandle, int iCmd, double dValue);
int RTSP_SendHeart(RTSP_HANDLE RHandle);
int RTSP_GetRtspTimeout(RTSP_HANDLE RHandle);
int RTSP_GetParam(RTSP_HANDLE RHandle, int iType, void *pValue);
int RTSP_GetServIpAddr(RTSP_HANDLE RHandle, char *pcIpAddr, int iLen);
int RTSP_GetConnect(RTSP_HANDLE RHandle);
int RTSP_CBFunEnable(RTSP_HANDLE RHandle,int iEnable);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif
