#ifndef __RTSP_H_
#define __RTSP_H_

#include "./debugout/debug.h"
#include "mutex.h"
#include "types.h"
#include "rtspComm.h"
#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define RTSP_DEBUG DebugPrint



#define DEFAULT_RTSP_VERSION "RTSP/1.0"

/* Generic function return values                                             */
#define RET_OK               (0)
#define RET_ERROR            (-1)
#define RET_UNAUTHORIZATION  (-100)

#define AUTHORIZATE_TYPE_DIGEST        1   // ?a��a��??��
#define AUTHORIZATE_TYPE_BASIC         2   // ?����?��??��

#define MAX_HEADER_NUMBER 32
#define MAX_TCP_DATA_SIZE 1024
#define MAX_UDP_DATA_SIZE 409600

typedef enum _RTSPMsgMethod
{
    HTTP_METHOD_GET,                         /* Method - GET                  */
    HTTP_METHOD_POST,                        /* Method - POST                 */
    RTSP_METHOD_OPTIONS,                     /* Method - OPTIONS              */
    RTSP_METHOD_DESCRIBE,                    /* Method - DESCRIBE             */
    RTSP_METHOD_SETUP,                       /* Method - SETUP                */
    RTSP_METHOD_PLAY,                        /* Method - PLAY                 */
    RTSP_METHOD_TEARDOWN,                    /* Method - TEARDOWN             */
    RTSP_METHOD_PAUSE,                       /* Method - PAUSE                */
    RTSP_METHOD_GET_PARAMETER,               /* Method - GET_PARAMETER        */
    RTSP_METHOD_SET_PARAMETER,               /* Method - SET_PARAMETER        */
    RTSP_METHOD_ANNOUNCE,                    /* Method - ANNOUNCE             */    
    RTSP_METHOD_REDIRECT,                    /* Method - REDIRECT             */
    RTSP_METHOD_RECORD,                      /* Method - RECORD               */
    RTSP_METHOD_INVALID                      /* Method - INVALID              */

} E_RTSP_MSG_METHOD;

typedef enum _E_RTSP_STATE
{
	E_RTSP_STATE_OPTION,
	E_RTSP_STATE_DESCRIBE,
	E_RTSP_STATE_SETUP,
	E_RTSP_STATE_PLAY,
	E_RTSP_STATE_PAUSE,
	E_RTSP_STATE_TEARDOWN,
} E_RTSP_STATE;

typedef enum _E_TRACK_TYPE
{
	E_VIDEO_TRACK = 0,
	E_AUDIO_TRACK
} E_TRACK_TYPE;



typedef INT32 (*PF_RTP_SET_DATA_CALLBACK)(int iFrameType, int iStreamType, char *pcFrame, int iFrameLen, unsigned int uiTs, void *pUserData);

/* Structure for RTSP Headers                                                 */
typedef struct _T_RTSP_HEADER_STRUCT
{
    char                 *pHeaderName;         /* Name of RTSP Header          */
    char                 *pHeaderValue;        /* Value of RTSP Header         */

} T_RTSP_HEADER_STRUCT;

/* Structure for RTSP Response Message                                        */
typedef struct _T_RTSP_RESPONSE_MSG
{
    INT32                 ResStatusCode;                       /* Status code of response         */
    INT32                 ResSeqNumber;                        /* RTSP Sequence Number            */
    INT32                 ResHeadersCount;                     /* RTSP Response Header count      */
    char                  *pResReasonPhrase;                   /* Pointer to reason string        */
    char                  *pResRtspVersion;                     /* RTSP message version            */
	INT32                 iSdpLen;
	char                  *pSdpValue;
    T_RTSP_HEADER_STRUCT  ResMsgHeaders[MAX_HEADER_NUMBER];    /* RTSP Resp headers               */
    //T_RTSP_HEADER_STRUCT  ResMsgSdpHeaders[MAX_HEADER_NUMBER]; /* RTSP Resp SDP headers           */

} T_RTSP_RESPONSE_MSG, *PT_RTSP_RESPONSE_MSG;

typedef struct _T_SDP_INFO
{
    INT32                 ResSdpHeaderCount;                   /* RTSP Response SDP headers count */
	T_RTSP_HEADER_STRUCT  ResMsgSdpHeaders[MAX_HEADER_NUMBER]; /* RTSP Resp SDP headers           */
} T_SDP_INFO;

/* Structure for RTSP Request Message                                         */
typedef struct _T_RTSP_REQUEST_MSG
{
    INT32                 ReqSeqNumber;                     /* RTSP Sequence Number      */
    INT32                 ReqHeadersCount;                  /* RTSP Request Header count */
    char                  *pReqMsgURI;                      /* RTSP mesaage URI          */
    E_RTSP_MSG_METHOD     ReqMethod;                    /* RTSP message method type  */
    T_RTSP_HEADER_STRUCT  ReqMsgHeaders[MAX_HEADER_NUMBER]; /* RTSP Req headers */
} T_RTSP_REQUEST_MSG, *PT_RTSP_REQUEST_MSG;


typedef struct _T_RTSP_BUFF
{
    INT32 iDataLen;
    char *pcData;
} T_RTSP_BUFF, *PT_RTSP_BUFF;

typedef struct _T_RTP_CONN
{
	int iRtpSocket;
	int iRtpPort;
	int iRtpPacketCount;
	int iRtpThreadRunFlag;
    THREAD_HANDLE RtpRecvThread;
	void *ptRtspConn;
	struct _T_RTP_CONN *next;
}T_RTP_CONN, *PT_RTP_CONN, *PT_RTP_CONN_LIST;

typedef struct _T_RTCP_CONN
{
	int iRtcpSocket;
	int iRtcpPort;
	int iRtcpThreadRunFlag;
	THREAD_HANDLE RtcpRecvThread;
	void *ptRtspConn;
	PT_RTP_CONN ptRtpConn;
	struct _T_RTCP_CONN *next;

}T_RTCP_CONN, *PT_RTCP_CONN, *PT_RTCP_CONN_LIST;


typedef struct _T_RTSP_CONN
{
    int iRtspSocket;
    int iRtspPort;
    char acServHost[20];
    int iRtpTransportProtocol;                 // 1:TCP, 2:UDP
    int iRtspTimeout;              // ms
    int iRtspCseq;
    int iRtspState;
	int iVideoStreamType;
	int iAudioStreamType;
	int iAuthorizeFlag;      // �Ƿ�Ҫ��֤
	char acUser[64];         // �û���
	char acPasswd[64];       // ����
	char acAuthRealm[32];    // ��֤ʱ����������
	char acAuthNonce[64];    // ��֤ʱ����������
	char acNewUri[256];      // �ع�setup���uri
    char acSession[64];     
	char acUri[256];
	char acVideoTrackId[256];
	char acAudioTrackId[256];
    T_RTSP_REQUEST_MSG tRequestMsg;
    THREAD_HANDLE RtspRecvThread;
    int iRtspThreadRunFlag;
    PF_RTP_SET_DATA_CALLBACK pfRtpSetDataCB;
    void *pRtpCbArg;
	Mutex tMutex;
    PT_RTP_CONN_LIST ptRtpList;
    PT_RTCP_CONN_LIST ptRtcpList;
	char acSpsPpsData[512];
	int iSpsPpsLen;
	int iPlayRange;
    int iEnableCBFunFlag;
} T_RTSP_CONN, *PT_RTSP_CONN;


INT32 AddRequestHeader (T_RTSP_REQUEST_MSG *ptRequestMsg, char *HeaderName, char *HeaderValue);

#ifdef WIN
DWORD WINAPI RtspRecvThread(void *arg);
#else
void *RtspRecvThread(void *arg);
#endif

int RtspConnect(PT_RTSP_CONN ptRtspConn, const char *pcUri);

int RtspRequestOptions(PT_RTSP_CONN ptRtspConn);

int RtspRequestDescribe(PT_RTSP_CONN ptRtspConn);

int RtspRequestSetup(PT_RTSP_CONN ptRtspConn, int iTrackID);

int RtspRequestPlay(PT_RTSP_CONN ptRtspConn);

int RtspRequestPause(PT_RTSP_CONN ptRtspConn);

int RtspRequestTeardown(PT_RTSP_CONN ptRtspConn);

int RtspRequestGetParameter(PT_RTSP_CONN ptRtspConn);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif
