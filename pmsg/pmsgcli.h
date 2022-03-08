#ifndef  __PMSG_CLI_H__
#define __PMSG_CLI_H__

#include "types.h"

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define MSG_START_FLAG 0xFF

typedef  unsigned long PMSG_HANDLE;

typedef int (*PF_MSG_PROC_CALLBACK)(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen);

//enum _E_SERV_CONNECT_STATUS
//{
//    E_SERV_STATUS_UNCONNECT	= 0,
//    E_SERV_STATUS_CONNECT = 1
//};

#if 0
//client -> nvr
typedef enum _E_MSG_CLI2SERV
{
    CLI_SERV_MSG_TYPE_HEART = 0x51,
    CLI_SERV_MSG_TYPE_SET_PIC_ATTRIBUTE = 0x02,
    CLI_SERV_MSG_TYPE_GET_PIC_ATTRIBUTE = 0x03,
    CLI_SERV_MSG_TYPE_SET_OSD = 0x05,
    CLI_SERV_MSG_TYPE_CHECK_TIME = 0x06,
    CLI_SERV_MSG_TYPE_GET_NVR_STATUS = 0x07,
    CLI_SERV_MSG_TYPE_GET_IPC_STATUS = 0x08,
    CLI_SERV_MSG_TYPE_GET_IPC_STATUS_NEW = 0xA8,
    CLI_SERV_MSG_TYPE_GET_RECORD_FILE = 0x11,
    CLI_SERV_MSG_TYPE_SET_PTZ = 0x12,
    CLI_SERV_MSG_TYPE_SET_PRESETS = 0x13,
    CLI_SERV_MSG_TYPE_PVMS_IPC_CTRL = 0x14,
    CLI_SERV_MSG_TYPE_PVMS_LIGHT_CTRL = 0x15,
    CLI_SERV_MSG_TYPE_PVMS_SEARCH_IMAGE_CTRL = 0x17,
    CLI_SERV_MSG_TYPE_PVMS_UP_DOWN_RESP = 0x66,
    CLI_SERV_MSG_TYPE_SEND_PVMS_GPS = 0x71,
    CLI_SERV_MSG_TYPE_GET_LIGHT_STATUS = 0x22, /* ??ò??????*/
    CLI_SERV_MSG_TYPE_GET_RECORD_TIME_LEN = 0x42,
    CLI_SERV_MSG_TYPE_SET_PVMS_INFO = 0x43,
    CLI_SERV_MSG_TYPE_GET_CARRAGE_NVR_STATUS = 0x44,
    CLI_SERV_MSG_TYPE_SEND_TEMPORARY_SAVE = 0X45,
    
} E_MSG_CLI2SERV;

// nvr -> client
typedef enum _E_MSG_SERV_CLI
{
    SERV_CLI_MSG_TYPE_HEART = 0x01,
    SERV_CLI_MSG_TYPE_GET_PIC_ATTRIBUTE_RESP = 0x53,
    SERV_CLI_MSG_TYPE_GET_NVR_STATUS_RESP = 0x57,
    SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP = 0x58,
    SERV_CLI_MSG_TYPE_GET_IPC_STATUS_NEW_RESP = 0xB8,
    SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP = 0x61,
    SERV_CLI_MSG_TYPE_SET_PTZ_RESP = 0x62,
    SERV_CLI_MSG_TYPE_SET_PRESETS_RESP = 0x63,
    SERV_CLI_MSG_TYPE_PVMS_IPC_CTRL_RESP = 0x64,
    SERV_CLI_MSG_TYPE_PVMS_LIGHT_CTRL_RESP = 0x65,
    SERV_CLI_MSG_TYPE_PVMS_UP_DOWN_CTRL = 0x16,
    SERV_CLI_MSG_TYPE_PVMS_SEARCH_IMAGE = 0x67,
    SERV_CLI_MSG_TYPE_VIDEO_ALARM_REPORT = 0x09,
    SERV_CLI_MSG_TYPE_HDISK_ALARM_REPORT = 0x10,
    SERV_CLI_MSG_TYPE_PVMS_GPS_REPORT = 0x21,
    SERV_CLI_MSG_TYPE_GET_LIGHT_STATUS_RESP = 0x72,
    SERV_CLI_MSG_TYPE_GET_RECORD_TIME_LEN_RESP = 0x82,
    SERV_CLI_MSG_TYPE_SET_PVMS_INFO_RESP = 0x83,
	SERV_CLI_MSG_TYPE_SEND_TEMPORARY_SAVE_RESP = 0X85,
	SERV_CLI_MSG_TYPE_PISMSG_REPORT = 0x04,   //nvr -> client or pis -> client
    SERV_CLI_MSG_TYPE_PVMS_UPDOWN_REPORT = 0x22,  //nvr -> client or pis -> client
} E_MSG_SERV_CLI;
#endif

//client -> nvr
typedef enum _E_MSG_CLI2SERV
{
    CLI_SERV_MSG_TYPE_HEART = 0x51,
    CLI_SERV_MSG_TYPE_SET_PIC_ATTRIBUTE = 0x02,
    CLI_SERV_MSG_TYPE_GET_PIC_ATTRIBUTE = 0x03,
    CLI_SERV_MSG_TYPE_CHECK_TIME = 0x05,
    CLI_SERV_MSG_TYPE_GET_NVR_STATUS = 0x06,
    CLI_SERV_MSG_TYPE_GET_IPC_STATUS = 0x07,
    CLI_SERV_MSG_TYPE_GET_RECORD_FILE = 0x10, // 搜索视频报文
    CLI_SERV_MSG_TYPE_SET_CARRIAGE_NUM = 0x11,//设置车号信息报文
    CLI_SERV_MSG_TYPE_SET_PTZ = 0x12,
    CLI_SERV_MSG_TYPE_SET_PRESETS = 0x13,
    CLI_SERV_MSG_TYPE_PVMS_IPC_CTRL = 0x14,
    CLI_SERV_MSG_TYPE_PVMS_LIGHT_CTRL = 0x15,
    CLI_SERV_MSG_TYPE_PVMS_UP_DOWN_RESP = 0x66,
    CLI_SERV_MSG_TYPE_SEND_PVMS_GPS = 0x71,
    CLI_SERV_MSG_TYPE_GET_LIGHT_STATUS = 0x22, /* ��ò����״̬*/
    CLI_SERV_MSG_TYPE_GET_RECORD_TIME_LEN = 0x42,
    CLI_SERV_MSG_TYPE_SET_PVMS_INFO = 0x43,
    CLI_SERV_MSG_TYPE_GET_CARRAGE_NVR_STATUS = 0x44

} E_MSG_CLI2SERV;

// nvr -> client
typedef enum _E_MSG_SERV_CLI
{
    SERV_CLI_MSG_TYPE_HEART = 0x01,
    SERV_CLI_MSG_TYPE_GET_PIC_ATTRIBUTE_RESP = 0x53,
    SERV_CLI_MSG_TYPE_GET_NVR_STATUS_RESP = 0x56,
    SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP = 0x57,
    SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP = 0x60,
    SERV_CLI_MSG_TYPE_SET_CARRIAGE_NUM_RESP = 0x61,
    SERV_CLI_MSG_TYPE_SET_PTZ_RESP = 0x62,
    SERV_CLI_MSG_TYPE_SET_PRESETS_RESP = 0x63,
    SERV_CLI_MSG_TYPE_PVMS_IPC_CTRL_RESP = 0x64,
    SERV_CLI_MSG_TYPE_PVMS_LIGHT_CTRL_RESP = 0x65,
    SERV_CLI_MSG_TYPE_PVMS_UP_DOWN_CTRL = 0x16,
    SERV_CLI_MSG_TYPE_IPC_ALARM_REPORT = 0x08,//图像报警报文
    SERV_CLI_MSG_TYPE_NVR_ALARM_REPORT = 0x09,//硬盘报警报文
    SERV_CLI_MSG_TYPE_PVMS_GPS_REPORT = 0x21,
    SERV_CLI_MSG_TYPE_GET_LIGHT_STATUS_RESP = 0x72,
    SERV_CLI_MSG_TYPE_GET_RECORD_TIME_LEN_RESP = 0x82,
    SERV_CLI_MSG_TYPE_SET_PVMS_INFO_RESP = 0x83,
    SERV_CLI_MSG_TYPE_OFFLINE=0x11,
    SERV_CLI_MSG_TYPE_ONLINE=0x12,
} E_MSG_SERV_CLI;

typedef struct _T_PMSG_HEAD
{
    BYTE cMsgStartFLag;
    BYTE cMsgType;
    UINT16 sMsgLen;	
} T_PMSG_HEAD, *PT_PMSG_HEAD;


typedef struct _T_TIME_INFO
{
    INT16 year;
    INT8 mon;
    INT8 day;
    INT8 hour;
    INT8 min;
    INT8 sec; 	
} __attribute__((packed)) T_TIME_INFO;

typedef struct _T_PVMS_OSD_INFO
{
    T_TIME_INFO tTImeInfo;
    INT16 i16Speed;
    INT8 i8TrainNum[7];
    INT8 i8CarriageNo;
    INT8 i8PvmsPos;
    INT16 i16SectionRange;
    INT8 i8SectionInfo[30];
} __attribute__((packed)) T_PVMS_OSD_INFO, *PT_PVMS_OSD_INFO;

typedef struct _T_NVR_STATUS
{
    INT8 i8CarriageNo;
    INT8 i8DevType;
    INT8 acFactory[10];
    INT16 i16Version;
    INT16 i16HdiskTotalSize;    
    INT16 i16HdiskUsedSize;
} __attribute__((packed)) T_NVR_STATUS, *PT_NVR_STATUS;


typedef struct _T_IPC_STATUS
{
    INT8 i8CarriageNo;
    INT8 i8DevPos;
    INT8 i8OnLine;
    INT8 i8DevType;   
    INT8 acFactory[10];
    INT16 i16Version;

} __attribute__((packed)) T_IPC_STATUS, *PT_IPC_STATUS;	

typedef struct T_VIDEO_ALARM_STATUS
{
	INT8 iCarriageNO;   //服务器车厢号
	INT8 i8DevPos;
    INT8 i8VideoShade;      // 0:normal, 1:hade
    INT8 i8VideoLost;       // 0:normal, 1:lost
} __attribute__((packed)) T_VIDEO_ALARM_STATUS, *PT_VIDEO_ALARM_STATUS;

typedef struct T_HDISK_ALARM_STATUS
{
    INT8 i8HdiskLost;      // 0:normal, 1:lost
    INT8 i8HdiskBad;       // 0:normal, 1:bad
} __attribute__((packed)) T_HDISK_ALARM_STATUS, *PT_HDISK_ALARM_STATUS;


enum E_PTZ_CTRL_TYPE
{
    E_PTZ_UP     = 1,
    E_PTZ_DOWN,
    E_PTZ_LEFT,
    E_PTZ_RIGHT,
    E_ZOOM_IN,
    E_ZOOM_OUT,
    E_FOCUS_FAR,
    E_FOCUS_NEAR,

};

enum E_PTZ_MOVE_TYPE
{
    E_START_MOVE = 1,
    E_STOP_MOVE
};

enum E_PRESET_CTRL_TYPE
{
    E_PRESET_SET = 1,
    E_PRESET_GET
};

typedef struct _T_PTZ_OPT
{
    INT8 i8CtrlType;
    INT8 i8MoveType;
} __attribute__((packed)) T_PTZ_OPT, *PT_PTZ_OPT;

typedef struct _T_PRESET_OPT
{
    INT8 i8CtrlType;
    INT8 i8PresetNo;
} __attribute__((packed)) T_PRESET_OPT, *PT_PRESET_OPT;

typedef struct _T_NVR_TIME
{
    INT16 i16Year;
    INT8 i8Mon;
    INT8 i8day;
    INT8 i8Hour;
    INT8 i8Min;
    INT8 i8Sec;
}  __attribute__((packed)) T_NVR_TIME, *PT_NVR_TIME;

typedef struct _T_NVR_SEARCH_RECORD
{
    T_NVR_TIME tStartTime;
    T_NVR_TIME tEndTime;
    INT8 iCarriageNo;
    INT8 iIpcPos;
}  __attribute__((packed)) T_NVR_SEARCH_RECORD, *PT_NVR_SEARCH_RECORD;

typedef struct _T_PIC_ATTRIBUTE
{
    INT8 iBrightness;
    INT8 iSaturation;
    INT8 iContrast;
    INT8 iChroma;
} T_PIC_ATTRIBUTE, *PT_PIC_ATTRIBUTE;

typedef struct _T_OSD_INFO
{
	INT16 i16Year;
    INT8 i8Mon;
    INT8 i8day;
    INT8 i8Hour;
    INT8 i8Min;
    INT8 i8Sec;
    INT16 i16Speed;
    INT8 acTrainNum[7];
    INT8 i8CarriageNo;
    INT8 i8PvmsCarriageNO;
    INT16 i16Mileage;
    INT8 acInterval[30];
} __attribute__((packed))T_OSD_INFO, *PT_OSD_INFO;

typedef struct _T_PVMS_INFO
{
    INT8 i8PvmsVideoNum;
    INT8 i8PvmsCarriageNo;
} __attribute__((packed))T_PVMS_INFO, *PT_PVMS_INFO;

typedef struct _T_PVMS_UPDOWN_INFO
{
    INT8 i8PvmsUpdownFlag[4];
    INT8 acInterval[2];
} __attribute__((packed))T_PVMS_UPDOWN_INFO, *PT_PVMS_UPDOWN_INFO;

typedef struct _T_PISMSG_INFO
{
    INT16 i16Year;
    INT8 i8Mon;
    INT8 i8day;
    INT8 i8Hour;
    INT8 i8Min;
    INT8 i8Sec;
    INT16 i16Speed;
    INT8 acTrainNum[11];
    INT16 i16Mileage;
    INT8 acInterval[30];
} __attribute__((packed))T_PISMSG_INFO, *PT_PISMSG_INFO;

typedef struct _T_PMSG_PACKET
{
    PMSG_HANDLE PHandle;
    unsigned char ucMsgCmd;
    int iMsgDataLen;
    char *pcMsgData;
} __attribute__((packed)) T_PMSG_PACKET, *PT_PMSG_PACKET;

int PMSG_Init(void);
int PMSG_Uninit(void);
PMSG_HANDLE PMSG_CreateConnect(char *pcIpAddr, int iPort);
int PMSG_DestroyConnect(PMSG_HANDLE pMsgHandle);
int PMSG_GetConnectStatus(PMSG_HANDLE pMsgHandle);
int PMSG_SendRawData(PMSG_HANDLE pMsgHandle, char *pcData, int iDataLen);
int PMSG_SendPmsgData(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcData, int iDataLen);
int PMSG_GetDataFromPmsgQueue(PMSG_HANDLE pMsgHandle, PT_PMSG_PACKET ptPkt);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */


#endif
