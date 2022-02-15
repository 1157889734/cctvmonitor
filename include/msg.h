#ifndef _MSG_H_
#define _MSG_H_
#include <sys/poll.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef POLLRDNORM
#undef POLLRDNORM
#endif // POLLRDNORM

#ifdef POLLWRNORM
#undef POLLWRNORM
#endif // POLLWRNORM


#define POLLRDNORM 		                                     0x0040
#define POLLWRNORM 		                                     0x0004



#define MSG_SOCKET_TIMEOUT                              20


#define MSG_HANDLE_TYPE_NONE                          0x00
#define MSG_HANDLE_TYPE_SOCKET_DATAGRAM               0x01
#define MSG_HANDLE_TYPE_SOCKET_STREAM                 0x02
#define MSG_HANDLE_TYPE_SEND_ACK                      0x08

#define SINGLE_THREAD     0
#define MULTIPLE_THREAD   1

#define MSG_OPT_SUCCESS                                     0
#define MSG_OPT_FAIL_BAD_PARAM                     0xF0000001
#define MSG_OPT_FAIL_MAX_HANDLE                    0xF0000002

#define MAX_MSG_REG_ITEM_NUM                         10



#define MSG_TOKEN_NONE			 0x00


typedef struct _T_SOCK_DATA
{
    int  iLen;
    void *pData;
}__attribute__((packed))T_SOCK_DATA, *PT_SOCK_DATA;

typedef struct _T_SOCK_ADDR
{
	void *pAddr;
	int  iAddrLen;
}__attribute__((packed))T_SOCK_ADDR, *PT_SOCK_ADDR;

typedef struct _T_MSG
{
   int          iMsgToken;
   PT_SOCK_DATA ptSockData;
   PT_SOCK_ADDR ptSockAddr;
}__attribute__((packed))T_MSG, *PT_MSG;


typedef int (*PF_MSG)(PT_MSG);
typedef struct _T_MSG_REG_ITEM
{
    int iMsgToken;
    int	iHandle;
    int	iHandleType;
    PF_MSG	pfMsgProc;
} __attribute__((packed))T_MSG_REG_ITEM, *PT_MSG_REG_ITEM;

typedef struct _T_MSG_REG_ITEMS
{
    int              iCount;
    struct pollfd    tPollFds[MAX_MSG_REG_ITEM_NUM];
    T_MSG_REG_ITEM   tRegItem[MAX_MSG_REG_ITEM_NUM];
    pthread_mutex_t  *pMutex;
} __attribute__((packed))T_MSG_REG_ITEMS, *PT_MSG_REG_ITEMS;



int MSG_Init (int iThreadType);
int MSG_UnInit (void);
int MSG_RegHandle(PT_MSG_REG_ITEM pRegItem);
int MSG_UnRegHandle(T_MSG_REG_ITEM item);
int MSG_GetHandle(int iToken, PT_MSG_REG_ITEM pRegItem);
int MSG_SendMessage(PT_MSG pMsg);
int MSG_ReadMessage (PT_MSG pMsg);
int MSG_OnMessage (PT_MSG pMsg);

#ifdef __cplusplus
}
#endif

#endif
