#ifndef CMPLAYER_H
#define CMPLAYER_H


#include <QHBoxLayout>
#include <QWidget>
#include "./state/state.h"
#include "./log/log.h"
//#include <sys/sysinfo.h>

#include "vdec.h"

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */
#define  CMPPlayer_API

typedef void* CMPHandle;

enum STREAM_TYPE
{
    CMP_TCP = 0,
    CMP_UDP
};

enum CMPPLAY_STATE
{
    CMP_STATE_IDLE = 0,         // 空闲
    CMP_STATE_PLAY,             // 播放
    CMP_STATE_FAST_FORWARD,     // 快进
    CMP_STATE_PAUSE,            // 暂停
    CMP_STATE_STOP,             // 停止状态
    CMP_STATE_ERROR,            // 错误
    CMP_STATE_SLOW_FORWARD      // 慢进
};

enum CMP_OPEN_MEDIA_STATE
{
    CMP_OPEN_MEDIA_UNKOWN = 0,
    CMP_OPEN_MEDIA_FAIL,
    CMP_OPEN_MEDIA_SUCC,
    CMP_OPEN_MEDIA_LOGIN_FAIL,
    CMP_OPEN_MEDIA_STREAM_FAIL,
    CMP_OPEN_MEDIA_PLAYCONTRO_FAIL,
};

#ifdef  __cplusplus
extern "C"
{
#endif


/*************************************************
  函数功能:     CMP_Init
  函数描述:     创建媒体句柄
  输入参数:     HWND hWnd：媒体显示窗口INFO
                iDecType：解码类型，enum CMP_VDEC_TYPE
  输出参数:     无
  返回值:       媒体句柄
  作者：        lxy
  日期:         2015-09-10
*************************************************/

CMPPlayer_API CMPHandle CMP_Init(T_WND_INFO *pWndInfo, CMP_VDEC_TYPE eDecType);

/*************************************************
  函数功能:     CMP_UnInit
  函数描述:     销毁媒体句柄
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_UnInit(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_OpenMediaPreview
  函数描述:     打开预览媒体
  输入参数:     hPlay：媒体句柄， pcRtspUrl：rtsp地址， iTcpFlag：CMP_TCP or CMP_UDP
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_OpenMediaPreview(CMPHandle hPlay, const char *pcRtspUrl, int iTcpFlag);

/*************************************************
  函数功能:     CMP_OpenMediaFile
  函数描述:     打开点播媒体
  输入参数:     hPlay：媒体句柄， pcRtspFile:rtsp文件地址 ， iTcpFlag：CMP_TCP or CMP_UDP
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_OpenMediaFile(CMPHandle hPlay, const char *pcRtspFile, int iTcpFlag);

/*************************************************
  函数功能:     CMP_CloseMedia
  函数描述:     关闭媒体
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_CloseMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_GetPlayStatus
  函数描述:     获取播放状态
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       返回媒体播放状态
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API CMPPLAY_STATE CMP_GetPlayStatus(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_PlayMedia
  函数描述:     开始播放媒体流
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_PlayMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_PauseMedia
  函数描述:     暂停播放媒体流
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_PauseMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_SetPosition
  函数描述:     设置播放位置（毫秒）
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_SetPosition(CMPHandle hPlay, int64_t nPosTime);

/*************************************************
  函数功能:     CMP_SetPlaySpeed
  函数描述:     设置播放速度
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_SetPlaySpeed(CMPHandle hPlay, double dSpeed);

/*************************************************
  函数功能:     CMP_SetVolume
  函数描述:     设置播放音量
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_SetVolume(CMPHandle hPlay, int nVolume);

/*************************************************
  函数功能:     CMP_ChangeWnd
  函数描述:     改变播放窗口 //只针对软解码
  输入参数:     hPlay：媒体句柄 hWnd 改变后的目标窗口
  输出参数:     无
  返回值:
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_ChangeWnd(CMPHandle hPlay,const T_WND_INFO *pWndInfo);

/*************************************************
  函数功能:     CMP_GetPlayRange
  函数描述:     获取播放时长
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       文件播放总时长，以秒为单位
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/
CMPPlayer_API int CMP_GetPlayRange(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_GetPlayTime
  函数描述:     获取当前播放时间
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       文件播放总时长，以秒为单位
  作者：        dingjq
  日期:         2015-09-10
  修改:
*************************************************/


CMPPlayer_API int CMP_SetPlayEnable(CMPHandle hPlay, int enable);


/*************************************************
  函数功能:     CMP_SetPlayState
  函数描述:     set dec start/stop
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：
  日期:         2021-11-24
  修改:
*************************************************/
CMPPlayer_API int CMP_SetPlayState(CMPHandle hPlay, int iState);

/*************************************************
  函数功能:     CMP_FillDisplayBk
  函数描述:     fill rgb
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：
  日期:         2021-11-24
  修改:
*************************************************/
CMPPlayer_API int CMP_FillDisplayBk(CMPHandle hPlay, uint32_t rgb);



CMPPlayer_API int CMP_GetOpenMediaState(CMPHandle hPlay);

CMPPlayer_API int CMP_GetStreamState(CMPHandle hPlay);

CMPPlayer_API int CMP_GetPlayTime(CMPHandle hPlay);

#ifdef  __cplusplus
}
#endif


#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif
