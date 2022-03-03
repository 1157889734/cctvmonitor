#ifndef _VDEC_H_
#define _VDEC_H_
#include <unistd.h>

#include "mutex.h"
#include "types.h"
#include "./debugout/debug.h"

#define H264_CODE 0
#define H265_CODE 1

#define DISPLAY_START    1
#define DISPLAY_STOP     0

#define STOP_STREAM_PLAY    0
#define START_STREAM_PLAY   1
#define PAUSE_STREAM_PLAY   2

typedef void* HWND;
typedef struct _T_WND_INFO
{
    HWND hWnd;
    void *pRenderHandle;
    int  nFlag; /* 0:unknow, 1:RGB_888 */
    int(*decode_func_callback)(unsigned char*data,int w,int h,int flag);
}T_WND_INFO;


typedef void* VDEC_HADNDLE;

/*************************************************
  函数功能:     VDEC_Init
  函数描述:     初使化视频解码
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
  备注：一个应用程序只需调用一次
*************************************************/
int VDEC_Init(void);

/*************************************************
  函数功能:     VDEC_Uninit
  函数描述:     反初使化视频解码
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2014-04-20
  修改:   
*************************************************/
int VDEC_Uninit();

/*************************************************
  函数功能:     VDEC_CreateVideoDecCh
  函数描述:     创建视频解码通道
  输入参数:     iDecType解码类型（硬解码、软解码...）CMP_VDEC_TYPE
				hWnd 窗口句柄
  输出参数:     无
  返回值:       返回视频解码句柄,失败的话为空
  作者:         丁金奇
  日期:         2015-08-31
  修改:   
*************************************************/
VDEC_HADNDLE VDEC_CreateVideoDecCh(T_WND_INFO *pWndInfo, int iWidth, int iHeight,int iDecType,int iCodecID);
/*************************************************
  函数功能:     VDEC_DestroyVideoDecCh
  函数描述:     销毁视频解码通道
  输入参数:     VHandle:视频解码通道句柄
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:         丁金奇
  日期:         2015-08-31
  修改:   
*************************************************/
int VDEC_DestroyVideoDecCh(VDEC_HADNDLE VHandle);

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
int VDEC_SendVideoStream(VDEC_HADNDLE VHandle, void *pData, int iLen, unsigned int iPts);


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
int VDEC_ChangeWindow(VDEC_HADNDLE VHandle, const T_WND_INFO *pWndInfo);

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
int VDEC_StartPlayStream(VDEC_HADNDLE VHandle);

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
int VDEC_StopPlayStream(VDEC_HADNDLE VHandle);

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
int VDEC_PausePlayStream(VDEC_HADNDLE VHandle);

/*************************************************
  函数功能:     VDEC_DisplayEnable
  函数描述:     暂停 Display
  输入参数:     无
  输出参数:     无
  返回值:       0:成功, 否则:失败
  作者:
  日期:
  修改:
*************************************************/
int VDEC_DisplayEnable(VDEC_HADNDLE VHandle, int displayFlag);

#endif
