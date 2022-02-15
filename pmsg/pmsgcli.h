#ifndef  __PMSG_CLI_H__
#define __PMSG_CLI_H__



#define MSG_START_FLAG 0xCB

typedef  unsigned long PMSG_HANDLE;

typedef int (*PF_MSG_PROC_CALLBACK)(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen);



//client -> nvr
typedef enum _E_MSG_CLI2SERV
{
    CLI_SERV_MSG_TYPE_HEART = 0x02,
    CLI_SERV_MSG_TYPE_CHECK_TIME = 0x03,
    CLI_SERV_MSG_TYPE_GET_HDISK_STATUS = 0x05,//由NVR周期上报，也可以是收到查询请求立即返回结果
    CLI_SERV_MSG_TYPE_GET_IPC_STATUS = 0x07, //由NVR周期上报，也可以是收到查询请求立即返回结果
    CLI_SERV_MSG_TYPE_GET_RECORD_FILE = 0x09,

} E_MSG_CLI2SERV;

// nvr -> client
typedef enum _E_MSG_SERV_CLI
{
    SERV_CLI_MSG_TYPE_HEART = 0x01,
    SERV_CLI_MSG_TYPE_GET_HDISK_STATUS_RESP = 0x06,
    SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP = 0x08,
    SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP = 0x0A,
    SERV_CLI_MSG_TYPE_ALARM_LINKAGE_REPORT = 0x0D,
    
} E_MSG_SERV_CLI;

typedef struct _T_PMSG_HEAD
{
    char cMsgStartFLag;
    short sMsgCmd;
    short sDataLen;	
    char acReserve[4];
    char cDataEcc;
} __attribute__((packed)) T_PMSG_HEAD, *PT_PMSG_HEAD;

typedef struct _T_TIME_INFO
{
    short year;
    char mon;
    char day;
    char hour;
    char min;
    char sec; 	
} __attribute__((packed)) T_TIME_INFO;


typedef struct _T_NVR_STATUS
{
    char cHdiskNum;
    unsigned short cHdiskTotalSize; //G
    unsigned short cHdiskUsedSize;  //G
    char cHdiskLostAlarm;  // 1:alarm; 0:normal
    char cHdiskSpaceAlarm; // 1:alarm; 0:normal
} __attribute__((packed)) T_HDISK_STATUS, *PT_NVR_STATUS;


typedef struct _T_NVR_SEARCH_RECORD
{
    char iCarriageNo; // 车厢从01开始
    char cVideoType;  // 0:所有录像，1:普通录像，2:报警录像，3:紧急对讲录像
    char iIpcPos;     // 从1开始，0表示搜索所有相机
    T_TIME_INFO tStartTime;
    T_TIME_INFO tEndTime;
}  __attribute__((packed)) T_NVR_SEARCH_RECORD, *PT_NVR_SEARCH_RECORD;

#ifdef  __cplusplus
extern "C"
{
#endif


int PMSG_Init(void);
int PMSG_Uninit(void);
PMSG_HANDLE	PMSG_CreateResConn(int iPort);
int	PMSG_DestroyResConn(PMSG_HANDLE pMsgHandle);
PMSG_HANDLE PMSG_CreateConnect(char *pcIpAddr, int iPort, PF_MSG_PROC_CALLBACK pfMsgProcFunc);
int PMSG_DestroyConnect(PMSG_HANDLE pMsgHandle);
int PMSG_GetConnectStatus(PMSG_HANDLE pMsgHandle);
int PMSG_SendRawData(PMSG_HANDLE pMsgHandle, char *pcData, int iDataLen);
int PMSG_SendPmsgData(PMSG_HANDLE pMsgHandle, unsigned short usMsgCmd, char *pcData, int iDataLen);

#ifdef  __cplusplus
}
#endif

#endif
