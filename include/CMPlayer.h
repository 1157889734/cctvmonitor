#ifndef CMPLAYER_H
#define CMPLAYER_H


#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

typedef void* CMPHandle;

typedef enum _E_CMP_ERRCODE
{
    CMP_MEDIAHANDLE_NOTEXIST = -1,      //媒体句柄不存在
    CMP_VDECINIT_ERR = -2,   //视频解码初始化错误
    CMP_VDECCH_CREATE_ERR = -3, 		//视频解码通道创建失败
    CMP_WND_SETPOS_ERR = -4, 			//播放窗口位置设置失败
    CMP_VDECCH_DESTORY_ERR = -5,        //视频解码通道销毁失败
    CMP_VDECSTART_ERR = -6,				//启动视频解码失败
    CMP_MEDIAPLAY_ERR = -7,             //媒体播放失败
    CMP_VDECCH_ENABLE_ERR = -8, 		//解码通道使能失败
    CMP_STOPPLAY_ERR = -9,        		//停止播放失败
    CMP_DEVLOGIN_FAIL = -20,     	    //设备登录失败
    CMP_CREATEDECWIN_ERR = -21,    		    //解码通道窗口创建失败
    CMP_PLAY_ERR = -22,	  			    //播放失败
    CMP_DESTORYDECWIN_ERR = -23,   		    //解码通道窗口销毁失败
    CMP_MEM_ERR = -25,					//内存分配失败
    CMP_ERR_OTHER = -26   				    //其他错误
} _E_CMP_ERRCODE;

enum STREAM_TYPE
{
    CMP_TCP = 0,
    CMP_UDP
};

enum CMPPLAY_STATE
{
    CMP_STATE_IDLE = 0,        // 空闲
    CMP_STATE_PLAY,            // 播放
    CMP_STATE_FAST_FORWARD,    // 快进
    CMP_STATE_SLOW_FORWARD,    // 慢放
    CMP_STATE_PAUSE,           // 暂停
    CMP_STATE_STOP             // 停止状态
};

typedef enum _E_CMP_VO_TYPE    //视频输出类型
{
    CMP_VO_TYPE_HDMI,
    CMP_VO_TYPE_VGA,
    CMP_VO_TYPE_CVBS,
    CMP_VO_TYPE_MAX
} E_CMP_VO_TYPE;

typedef struct _T_VIDEO_INFO
{
    E_CMP_VO_TYPE eVoType;	//视频输出类型
    int iScreenWidth;	//屏宽
    int iScreenHeight;	//屏高
} T_VIDEO_INFO,*PT_VIDEO_INFO;


/*************************************************
  函数功能:     CMP_Init
  函数描述:     模块初使化
  输入参数:     无
  输出参数:     无
  返回值:       0-成功，否则失败
  作者：        dingjq
  日期:         2015-09-10
*************************************************/
int CMP_Init(PT_VIDEO_INFO ptVdecInfo);

/*************************************************
  函数功能:     CMP_UnInit
  函数描述:     模块反初使化
  输入参数:     无
  输出参数:     无
  返回值:       0：成功， <0:失败
  作者：        
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_UnInit(void);

/*************************************************
  函数功能:     CMP_SetBlackBackground
  
  函数描述: 将此块的背景黑化
  输入参数:     ix iy iWidth iHeight
  输出参数:     无
  返回值:       0：成功， <0:失败
  作者：        
  日期:         2018-11-21
  修改:
*************************************************/
int CMP_SetBlackBackground(int ix,int iy,int iWidth,int iHeight);


/*************************************************
  函数功能:     CMP_CreateMedia
  函数描述:     创建媒体句柄
  输入参数:     tWinInfo：媒体显示窗口位置信息
  返回值:       媒体句柄
  作者：      
  日期:         2016-11-01
*************************************************/
CMPHandle CMP_CreateMedia(void *pWinHandle);

/*************************************************
  函数功能:     CMP_DestroyMedia
  函数描述:     销毁媒体句柄
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       0：成功， <0:失败
  作者：     
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_DestroyMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_SetWndDisplayEnable
  函数描述:     设置播放窗口显示使能,调用后也关闭了解码
  输入参数:     hPlay：媒体句柄
                iEnable: 使能标志，1-使能，0-不使能
  输出参数:     无
  返回值:
  作者：       
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_SetWndDisplayEnable(CMPHandle hPlay, int iDispEnable,int iDecEnable );


/*************************************************
  函数功能:     CMP_SetWnd
  函数描述:     改变播放窗口
  输入参数:     hPlay：媒体句柄 tWnd 改变后的目标窗口
  输出参数:     无
  返回值:
  作者：    
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_ChangeWnd(CMPHandle hPlay, void *pWinHandle);


/*************************************************
  函数功能:     CMP_OpenMediaPreview
  函数描述:     打开预览媒体
  输入参数:     hPlay：媒体句柄， pcRtspUrl：rtsp地址， iTcpFlag：CMP_TCP or CMP_UDP
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：    
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_OpenMediaPreview(CMPHandle hPlay, const char *pcRtspUrl, int iTcpFlag);

/*************************************************
  函数功能:     CMP_OpenMediaFile
  函数描述:     打开点播媒体
  输入参数:     hPlay：媒体句柄， pcRtspFile:rtsp文件地址 ， iTcpFlag：CMP_TCP or CMP_UDP
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：    
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_OpenMediaFile(CMPHandle hPlay, const char *pcRtspFile, int iTcpFlag);

/*************************************************
  函数功能:     CMP_CloseMedia
  函数描述:     关闭媒体
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       0：成功， -1:未找到相应媒体句柄
  作者：  
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_CloseMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_GetPlayStatus
  函数描述:     获取播放状态
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       返回媒体播放状态
  作者：  
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_GetPlayStatus(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_PlayMedia
  函数描述:     开始播放媒体流
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：    
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_PlayMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_PauseMedia
  函数描述:     暂停播放媒体流
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:
  作者：   
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_PauseMedia(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_SetPosition
  函数描述:     设置播放位置（秒）
  输入参数:     hPlay：媒体句柄
  				nPosTime: 跳转时间(单位S)
  输出参数:     无
  返回值:
  作者：     
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_SetPosition(CMPHandle hPlay, int nPosTime);

/*************************************************
  函数功能:     CMP_SetPlaySpeed
  函数描述:     设置播放速度
  输入参数:     hPlay：媒体句柄
  				dSpeed: 速度值
  输出参数:     无
  返回值:
  作者：     
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_SetPlaySpeed(CMPHandle hPlay, double dSpeed);

/*************************************************
  函数功能:     CMP_GetPlayRange
  函数描述:     获取播放时长
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       文件播放总时长，以秒为单位
  作者：     
  日期:         2016-11-01
  修改:
*************************************************/
int CMP_GetPlayRange(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_GetCurrentPlayTime
  函数描述:     获取录像当前播放时间
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       录像当前播放时间，以秒为单位
  作者：     
  日期:         2017-5-12
  修改:
*************************************************/
int CMP_GetCurrentPlayTime(CMPHandle hPlay);

/*************************************************
  函数功能:     CMP_GetStreamState
  函数描述:     获取码流状态(是否有流)
  输入参数:     hPlay：媒体句柄
  输出参数:     无
  返回值:       是否有流的状态，1-有流，0-无流
  作者：     
  日期:         2017-5-12
  修改:
*************************************************/
int CMP_GetStreamState(CMPHandle hPlay);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif // CMPLAYER_H
