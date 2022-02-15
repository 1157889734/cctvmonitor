#ifndef _MSGAPP_H_
#define _MSGAPP_H_



// port
#define LEDCTRL_AYNC_PORT                10020
#define LEDTEST_AYNC_PORT                10021

#define CCTV_CTRL_ASYNC_PORT        10024
//#define CCTV_CTRL_SYNC_PORT          10025

#define DHMI_CTRL_ASYNC_PORT        10026
//#define DHMI_CTRL_SYNC_PORT          10027




// dhmi -> cctv
enum E_DHMI2CCTV_MSG
{
    MSG_DHMI2CCTV_BEGIN = 0x4001,
    MSG_DHMI2CCTV_ASYNC_REQUEST_STATE = MSG_DHMI2CCTV_BEGIN,
    MSG_DHMI2CCTV_ASYNC_SWITCH_CCTV,
    MSG_DHMI2CCTV_ASYNC_SWITCH_DMI,
    MSG_DHMI2CCTV_ASYNC_SWITCH_HMI,
    MSG_DHMI2CCTV_ASYNC_RESP_CCTV_REQUEST_STATE,

};

// cctv -> dhmi
enum E_CCTV2DHMI_MSG
{
    MSG_CCTV2DHMI_BEGIN = 0x5001,
    MSG_CCTV2DHMI_ASYNC_REQUEST_STATE = MSG_CCTV2DHMI_BEGIN,
    MSG_CCTV2DHMI_ASYNC_SWITCH_DMI,
    MSG_CCTV2DHMI_ASYNC_SWITCH_HMI,
    MSG_CCTV2DHMI_ASYNC_RESP_DHMI_REQUEST_STATE,
       
};


typedef enum _E_MSG_TOKEN
{
    MSG_TOKEN_BIGIN                             = 0,
    MSG_TOKEN_CCTV_ASYNC_UDP                 = 0x01,
    MSG_TOKEN_DHMI_ASYNC_UDP                 = 0x02,
    MSG_TOKEN_END = MSG_TOKEN_DHMI_ASYNC_UDP
}E_MSG_TOKEN;

#define MAX_MSG_APP_DATA_LEN 1024

// #define MSG_ACK_NO_RESPONSE   0
// #define MSG_ACK_RESPONSE         1

typedef struct _T_MSG_APP_DATA
{
    int iCmd;
    int iLen;
  //  int iAck;         // 0:no response , 1: response
    char acData[MAX_MSG_APP_DATA_LEN];
}__attribute__((packed))T_MSG_APP_DATA, *PT_MSG_APP_DATA;



#endif

