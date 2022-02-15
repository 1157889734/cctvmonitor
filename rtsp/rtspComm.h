#ifndef _RTSP_COMM_H_
#define _RTSP_COMM_H_

//#define WIN

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */


#ifdef WIN
#else
#define THREAD_HANDLE pthread_t
#endif

typedef enum _E_NET_TYPE
{
    TCP,
    UDP
} E_NET_TYPE;

typedef enum _E_STREAM_TYPE
{
    STREAM_TYPE_AUDIO,
    STREAM_TYPE_VIDEO
} E_STREAM_TYPE;

enum E_VIDEO_STREAM_TYPE
{
	E_STREAM_TYPE_H264 = 1,
       E_STREAM_TYPE_H265,
};

//typedef void* (*PF_THREAD_PROC_CALL_BACK)(void *)


void RTSP_Close(int iSocket);

//int Rtsp_ThreadCread(THREAD_HANDLE *pthreadId, char *pcAttibute, PF_THREAD_PROC_CALL_BACK pfThreadCb, void *pArg);

int RTSP_ThreadJoin(THREAD_HANDLE threadHandle);

void RTSP_MSleep(int iMsec);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */


#endif

