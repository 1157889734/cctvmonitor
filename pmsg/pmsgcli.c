#include "pmsg/pmsgcli.h"
#include "debugout/debug.h"
#include "log/log.h"
#include "state/state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>

#define RECV_BUF_LEN 10*1024

typedef struct _T_PMSG_PACKET_LIST
{
    T_PMSG_PACKET tPkt;
    struct _T_PMSG_PACKET_LIST *next;
} __attribute__((packed)) T_PMSG_PACKET_LIST;

typedef struct _T_PMSG_QUEUE
{
    T_PMSG_PACKET_LIST *ptFirst, *ptLast;
    INT32 iQueueType;
    INT32 iPktCount;
    pthread_mutex_t *pMutex;
} __attribute__((packed)) T_PMSG_QUEUE, *PT_PMSG_QUEUE;

typedef struct _T_PMSG_CONN_INFO
{
    int iSockfd;
    int iConnectStatus;
    int iThreadRunFlag;
    int iServPort;
    char acIpAddr[20];

    pthread_t ThreadHandle;
    pthread_mutex_t	tPmsgMutex;
    pthread_mutex_t tPmsgQueueMutex;

    PT_PMSG_QUEUE ptPmsgQueue;
} T_PMSG_CONN_INFO, *PT_PMSG_CONN_INFO;

BYTE GetMsgDataEcc(BYTE *pcData, INT32 iLen)
{
    int i = 0;
    BYTE ucEcc = 0;
    
    if ((NULL == pcData) || (0 == iLen))	
    {
        return 0;	
    }
    
    for (i = 0; i < iLen; i++)
    {
        ucEcc ^= pcData[i];
    }
    
    return ucEcc;
}

int CreateTcpSocket(char *pcIpaddr, unsigned short u16Port)
{
    int iSockfd = 0;
    int iRet = 0;
    struct sockaddr_in servaddr;
    struct timeval tv_out;

    iSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (iSockfd < 0)
    {
        perror("socket error:");
        return -1;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;

    servaddr.sin_addr.s_addr = inet_addr(pcIpaddr);
    servaddr.sin_port = htons(u16Port);
   

    iRet = connect(iSockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (iRet < 0)
    {
        //printf("sockfd %d connect addr %s  port %d error\n", iSockfd, tSysAddr.acCameraAddr, tSysAddr.sCameraPort);
       // perror(":");
        close(iSockfd);        
        return -1;
    }
    
    tv_out.tv_sec = 5;
    tv_out.tv_usec = 500;
    setsockopt(iSockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
    setsockopt(iSockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out));

    return iSockfd;
}

void DestroyTcpSocket(int iSockfd)
{   
   close(iSockfd);		
}

PT_PMSG_QUEUE CreatePmsgQueue(pthread_mutex_t *pMutex, int iQueueType)
{
    PT_PMSG_QUEUE ptPmsgQueue = NULL;

    ptPmsgQueue = (PT_PMSG_QUEUE)malloc(sizeof(T_PMSG_QUEUE));
    if (NULL == ptPmsgQueue)
    {
        return NULL;
    }
    memset(ptPmsgQueue, 0, sizeof(T_PMSG_QUEUE));
    ptPmsgQueue->pMutex = pMutex;
    ptPmsgQueue->iQueueType = iQueueType;
    ptPmsgQueue->ptLast = NULL;
    ptPmsgQueue->ptFirst = NULL;
    ptPmsgQueue->iPktCount= 0;

    return ptPmsgQueue;
}

int DestroyPmsgQueue(PT_PMSG_QUEUE ptPmsgQueue)
{
    T_PMSG_PACKET_LIST *ptPktList = NULL, *ptTmp;

    if (NULL == ptPmsgQueue)
    {
        return -1;
    }

    if (ptPmsgQueue->pMutex)
    {
        pthread_mutex_lock(ptPmsgQueue->pMutex);
    }

    ptPktList = ptPmsgQueue->ptFirst;
    while (ptPktList)
    {
        ptTmp = ptPktList;
        #if 1
        if (ptTmp->tPkt.pcMsgData)
        {
        	free(ptTmp->tPkt.pcMsgData);
        	ptTmp->tPkt.pcMsgData = NULL;
        }
        #endif
        ptPktList = ptPktList->next;
        free(ptTmp);
    }

    ptPmsgQueue->ptLast = NULL;
    ptPmsgQueue->ptFirst = NULL;
    ptPmsgQueue->iPktCount= 0;

    if (ptPmsgQueue->pMutex)
    {
        pthread_mutex_unlock(ptPmsgQueue->pMutex);
    }

    free(ptPmsgQueue);
    ptPmsgQueue = NULL;

    return 0;
}

int PutNodeToPmsgQueue(PT_PMSG_QUEUE ptPmsgQueue, PT_PMSG_PACKET ptPkt)
{
    T_PMSG_PACKET_LIST *ptPktList = NULL;


    if ((NULL == ptPmsgQueue) || (NULL == ptPkt))
    {
        return -1;
    }
    ptPktList = (T_PMSG_PACKET_LIST *)malloc(sizeof(T_PMSG_PACKET_LIST));
    if (NULL == ptPktList)
    {
        return -1;
    }

    memset(ptPktList, 0, sizeof(T_PMSG_PACKET_LIST));
    ptPktList->tPkt = *ptPkt;

    if (ptPmsgQueue->pMutex)
    {
        pthread_mutex_lock(ptPmsgQueue->pMutex);
    }

    if (NULL == ptPmsgQueue->ptLast)
    {
        ptPmsgQueue->ptFirst = ptPktList;
    }
    else
    {
        ptPmsgQueue->ptLast->next = ptPktList;
    }
    ptPmsgQueue->ptLast = ptPktList;
    ptPmsgQueue->iPktCount++;

    if (ptPmsgQueue->pMutex)
    {
        pthread_mutex_unlock(ptPmsgQueue->pMutex);
    }

    return 0;
}

int GetNodeFromPmsgQueue(PT_PMSG_QUEUE ptPmsgQueue, PT_PMSG_PACKET ptPkt)
{
    T_PMSG_PACKET_LIST *ptTmp = NULL;

    if ((NULL == ptPmsgQueue) || (NULL == ptPkt))
    {
        return 0;
    }

    if (ptPmsgQueue->pMutex)
    {
        pthread_mutex_lock(ptPmsgQueue->pMutex);
    }

    if (NULL == ptPmsgQueue->ptFirst)
    {
        if (ptPmsgQueue->pMutex)
        {
            pthread_mutex_unlock(ptPmsgQueue->pMutex);
        }

        return 0;
    }

    ptTmp = ptPmsgQueue->ptFirst;
    ptPmsgQueue->ptFirst = ptPmsgQueue->ptFirst->next;
    if (NULL == ptPmsgQueue->ptFirst)
    {
        ptPmsgQueue->ptLast= NULL;
    }
    ptPmsgQueue->iPktCount--;
    *ptPkt = ptTmp->tPkt;
    free(ptTmp);

    if (ptPmsgQueue->pMutex)
    {
        pthread_mutex_unlock(ptPmsgQueue->pMutex);
    }

    return 1;
}

void *CliProcessThread(void *arg)
{
    unsigned char *pcRecvBuf = NULL;
    unsigned char *pcLeaveBuf = NULL;
    unsigned char *pcMsgBuf = NULL;
    unsigned char ucMsgHead = 0;
    unsigned char ucEcc = 0;
    int iBufLen = RECV_BUF_LEN;
    int iSocket = 0;
    int iPreLeaveLen = 0, iLeaveLen = 0, iRecvLen = 0;
    int iMsgDataLen = 0;
    int iHearCount = 0;
    int iRet = 0;
    int iOffset = 0;
    fd_set	tAllSet, tTmpSet;
    struct timeval tv;
    time_t tCurTime = 0, tOldTime = 0;
    T_LOG_INFO tLogInfo;
    T_PMSG_PACKET tPkt;
    T_PMSG_CONN_INFO *ptPmsgConnInfo = (T_PMSG_CONN_INFO *)arg;
    PMSG_HANDLE pMsgHandle = (PMSG_HANDLE)arg;
    
    if (NULL == ptPmsgConnInfo)
    {
        return NULL;	
    }

    pcRecvBuf = (unsigned char *)malloc(iBufLen);
    if (NULL == pcRecvBuf)
    {
        return NULL;        	
    }
    memset(pcRecvBuf, 0, iBufLen);
    
    pcLeaveBuf = (unsigned char *)malloc(iBufLen);
    if (NULL == pcLeaveBuf)
    {
        free(pcRecvBuf);
        pcRecvBuf = NULL;
        return NULL;        	
    }
    memset(pcLeaveBuf, 0, iBufLen);
    
    while (ptPmsgConnInfo->iThreadRunFlag)
    {
        if (iSocket <= 0)
        {
            iSocket = CreateTcpSocket(ptPmsgConnInfo->acIpAddr, (unsigned short)ptPmsgConnInfo->iServPort);
            if (iSocket > 0)
            {
                FD_ZERO(&tAllSet);
                FD_SET(iSocket, &tAllSet);
                ptPmsgConnInfo->iSockfd = iSocket;
                ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_CONNECT;
				memset(&tLogInfo, 0, sizeof(T_LOG_INFO));
                tLogInfo.iLogType = 0;
				snprintf(tLogInfo.acLogDesc, sizeof(tLogInfo.acLogDesc), "server %s connected", ptPmsgConnInfo->acIpAddr);
				LOG_WriteLog(&tLogInfo);
            }
            
        }
        if (iSocket <= 0)
        {
            sleep(2);
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
            continue;
        }
    
        tv.tv_sec = 0;
        tv.tv_usec = 20000;
        
        tTmpSet = tAllSet;	//重新置位.
        if (select(iSocket + 1, &tTmpSet, NULL, NULL, &tv) > 0)
        {
            if (FD_ISSET(iSocket, &tTmpSet))
            {
                iHearCount = 0;
				memset(&pcRecvBuf[iPreLeaveLen], 0, iBufLen - iPreLeaveLen);
                iRecvLen = recv(iSocket, &pcRecvBuf[iPreLeaveLen], iBufLen - iPreLeaveLen - 1, 0);

                if (iRecvLen <= 0)
                {
                    perror("recv:");
                    //printf("nvr serv %s exit, recv error\n", ptPmsgConnInfo->acIpAddr);

                    DestroyTcpSocket(iSocket);
                    iSocket = -1;
                    ptPmsgConnInfo->iSockfd = -1;
                    ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
					memset(&tLogInfo, 0, sizeof(T_LOG_INFO));
                    tLogInfo.iLogType = 0;
					snprintf(tLogInfo.acLogDesc, sizeof(tLogInfo.acLogDesc), "server %s disconnected", ptPmsgConnInfo->acIpAddr);
					LOG_WriteLog(&tLogInfo);
                    continue;
                }
                
                if (iPreLeaveLen > 0)
                {
                    memcpy(pcRecvBuf, pcLeaveBuf, iPreLeaveLen);
                }
                
                iLeaveLen = iRecvLen + iPreLeaveLen;
                iOffset = 0;
                while (iLeaveLen > 0)
                {
                    if (iLeaveLen < 5)
                    {
                        memcpy(pcLeaveBuf, &pcRecvBuf[iOffset], iLeaveLen);
                        iPreLeaveLen = iLeaveLen;
                        break;    	
                    }
                        
                    pcMsgBuf = &pcRecvBuf[iOffset];

                    ucMsgHead = (unsigned char)pcMsgBuf[0];
                    
                    // 验证消息头的正确性
                    if (MSG_START_FLAG != ucMsgHead)
                    {
                        iPreLeaveLen = 0;
                        break;
                    }
                        
                    // 验证消息长度的正确性
                    iMsgDataLen = pcMsgBuf[2] << 8 | pcMsgBuf[3];
                    if (iMsgDataLen > 1024)
                    {
                        iPreLeaveLen = 0;
                        break;
                    }
                        
                    if (iMsgDataLen <= iLeaveLen - 5)
                    {
                        if (pcMsgBuf[1] != SERV_CLI_MSG_TYPE_HEART)
                        {
							ucEcc = GetMsgDataEcc((BYTE *)pcMsgBuf, 4);
						    if (iMsgDataLen > 0)
						    {
						        ucEcc ^= GetMsgDataEcc((BYTE *)&pcMsgBuf[4], iMsgDataLen);
						    }       
						    if (pcMsgBuf[4 + iMsgDataLen] != ucEcc)  //ECC校验
						    {
						    	iPreLeaveLen = 0;
						    	break;
						    }
                        	
                            tPkt.PHandle = pMsgHandle;
                            tPkt.ucMsgCmd = pcMsgBuf[1];
                            tPkt.iMsgDataLen = iMsgDataLen;
                            tPkt.pcMsgData = (char *)malloc(iMsgDataLen+16);
                            if (NULL == tPkt.pcMsgData)
                            {
                            	iPreLeaveLen = 0;
                                break;
                            }
                            memcpy(&(tPkt.pcMsgData[0]), (char *)&pcMsgBuf[4], iMsgDataLen);
                            PutNodeToPmsgQueue(ptPmsgConnInfo->ptPmsgQueue, &tPkt);
                        }
                        iLeaveLen -= iMsgDataLen + 5;
                        iOffset += iMsgDataLen + 5;
                        iPreLeaveLen = 0;
                        continue;
                    }
                    else
                    {
                        memcpy(pcLeaveBuf, &pcRecvBuf[iOffset], iLeaveLen);
                        iPreLeaveLen = iLeaveLen;
                        break;	
                    }
                }
            }
        }
        
        tCurTime = time(NULL);
        if (tCurTime != tOldTime)
        {
            iRet = PMSG_SendPmsgData((PMSG_HANDLE)ptPmsgConnInfo, CLI_SERV_MSG_TYPE_HEART, NULL, 0);
            if (iRet <= 0)
            {
                DestroyTcpSocket(iSocket);
                iSocket = -1;
                ptPmsgConnInfo->iSockfd = -1;
                ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
				memset(&tLogInfo, 0, sizeof(T_LOG_INFO));
                tLogInfo.iLogType = 0;
				snprintf(tLogInfo.acLogDesc, sizeof(tLogInfo.acLogDesc), "server %s disconnected", ptPmsgConnInfo->acIpAddr);
				LOG_WriteLog(&tLogInfo);
                iHearCount = 0;
            } 
        }
        tOldTime = tCurTime;
        
        iHearCount ++;

        if (iHearCount > 1000)
        {
            DestroyTcpSocket(iSocket);
            iSocket = -1;
            ptPmsgConnInfo->iSockfd = -1;
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
			memset(&tLogInfo, 0, sizeof(T_LOG_INFO));
            tLogInfo.iLogType = 0;
			snprintf(tLogInfo.acLogDesc, sizeof(tLogInfo.acLogDesc), "server %s disconnected", ptPmsgConnInfo->acIpAddr);
			LOG_WriteLog(&tLogInfo);
            iHearCount = 0;
        }
    }
    
    if (iSocket > 0)
    {
        DestroyTcpSocket(iSocket);	
        ptPmsgConnInfo->iSockfd = -1;
    }
    
    if (pcRecvBuf)
    {
        free(pcRecvBuf);
        pcRecvBuf = NULL;	
    }
    
    if (pcLeaveBuf)
    {
        free(pcLeaveBuf);
        pcLeaveBuf = NULL;	
    }
        
    return NULL;
}


int PMSG_Init(void)
{
#ifdef WIN32
	WSADATA wsa={0};
	WSAStartup(MAKEWORD(2,2),&wsa);    
#endif

    return 0;
}

int PMSG_Uninit(void)
{
#ifdef WIN32
	WSACleanup();
#endif

    return 0;
}

PMSG_HANDLE PMSG_CreateConnect(char *pcIpAddr, int iPort)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = NULL;
    pthread_mutexattr_t	mutexattr;
    int iRet = 0;

    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_CreateConnect server ip=%s, iPort=%d\n", pcIpAddr, iPort);
    
    ptPmsgConnInfo = (PT_PMSG_CONN_INFO)malloc(sizeof(T_PMSG_CONN_INFO));
    if (NULL == ptPmsgConnInfo)
    {		
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_CreateConnect error! malloc error\n");
        return 0;	
    }
    memset(ptPmsgConnInfo, 0, sizeof(T_PMSG_CONN_INFO));
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_TIMED_NP);
    pthread_mutex_init(&ptPmsgConnInfo->tPmsgMutex, &mutexattr);
    pthread_mutex_init(&ptPmsgConnInfo->tPmsgQueueMutex, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
    
    ptPmsgConnInfo->iThreadRunFlag = 1;
    strncpy(ptPmsgConnInfo->acIpAddr, pcIpAddr, sizeof(ptPmsgConnInfo->acIpAddr));
    ptPmsgConnInfo->iServPort = iPort;

    ptPmsgConnInfo->ptPmsgQueue = CreatePmsgQueue(&ptPmsgConnInfo->tPmsgQueueMutex, 0);
    
    iRet = pthread_create(&ptPmsgConnInfo->ThreadHandle, NULL, CliProcessThread, (void *)ptPmsgConnInfo);
    if (iRet < 0)
    {
        free(ptPmsgConnInfo);
        ptPmsgConnInfo = NULL;
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_CreateConnect error! CliProcessThread create error\n");
        return 0;	
    }
    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_CreateConnect create thread %d ok\n", (int)ptPmsgConnInfo->ThreadHandle);

    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_CreateConnect Ok\n");
    return (PMSG_HANDLE)ptPmsgConnInfo;
}

int PMSG_DestroyConnect(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    
    if (NULL == ptPmsgConnInfo)
    {	
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_DestroyConnect error! pmsg handle is NULL\n");
        return -1;	
    }
    ptPmsgConnInfo->iThreadRunFlag = 0;
    
    // join thread exit
    if (ptPmsgConnInfo->ThreadHandle)
    {
        DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_DestroyConnect, CliProcessThread join begin\n");
        pthread_join(ptPmsgConnInfo->ThreadHandle, NULL);
        DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_DestroyConnect, CliProcessThread join end\n");
    }
    pthread_mutex_destroy(&ptPmsgConnInfo->tPmsgMutex);
    DestroyPmsgQueue(ptPmsgConnInfo->ptPmsgQueue);
    pthread_mutex_destroy(&ptPmsgConnInfo->tPmsgQueueMutex);
    free(ptPmsgConnInfo);
    ptPmsgConnInfo = NULL;
    
    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_DestroyConnect Ok\n");
    return 0;
}

int PMSG_GetConnectStatus(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    
    if (NULL == ptPmsgConnInfo)
    {
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_GetConnectStatus error! pmsg handle is NULL\n");
        return -1;	
    }

    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_GetConnectStatus Ok, %s iConnectStatus=%d\n", ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iConnectStatus);
    return ptPmsgConnInfo->iConnectStatus;
}

int PMSG_SendRawData(PMSG_HANDLE pMsgHandle, char *pcData, int iDataLen)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    int iRet = 0;
    
    if ((NULL == ptPmsgConnInfo) || (NULL == pcData) || (iDataLen <= 0))
    {
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_SendRawData error! pmsg handle is NULL\n");
        return -1;	
    }
	
    if (E_SERV_STATUS_CONNECT != ptPmsgConnInfo->iConnectStatus)
    {
        return -1;
    }
    pthread_mutex_lock(&ptPmsgConnInfo->tPmsgMutex);
    iRet = send(ptPmsgConnInfo->iSockfd, pcData, iDataLen, 0);
    pthread_mutex_unlock(&ptPmsgConnInfo->tPmsgMutex);
    
    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_SendRawData Ok, send data len=%d\n",iRet);
    return iRet;
}

int PMSG_SendPmsgData(PMSG_HANDLE pMsgHandle, unsigned char ucMsgCmd, char *pcData, int iDataLen)
{
    char acMsg[1024];
    unsigned char ucEcc = 0;
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    PT_PMSG_HEAD ptMsgHead = NULL;
    short i16SendLen = 0;
    int iRet = 0;
    if (NULL == ptPmsgConnInfo)  
    {
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_SendPmsgData error! pmsg handle is NULL\n");
        return -1;	
    }

    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_SendPmsgData, ucMsgCmd=0x%x\n", (int)ucMsgCmd);

    if (E_SERV_STATUS_CONNECT != ptPmsgConnInfo->iConnectStatus)
    {
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_SendPmsgData error, not connected\n");
        return -1;
    }
    if (iDataLen > (1024 - 5))
    {
        iDataLen = 1024 -5;	
    }
    
    memset(acMsg, 0, sizeof(acMsg));
    ptMsgHead = (PT_PMSG_HEAD)acMsg;
    if (iDataLen > 0)
    {
        memcpy(&acMsg[sizeof(T_PMSG_HEAD)], pcData, iDataLen);
        i16SendLen = iDataLen;
    }
    ptMsgHead->cMsgStartFLag = MSG_START_FLAG;
    ptMsgHead->cMsgType = ucMsgCmd;
    ptMsgHead->sMsgLen = htons(i16SendLen);
            
    // 计算ECC校验
    ucEcc = GetMsgDataEcc((BYTE *)ptMsgHead, sizeof(T_PMSG_HEAD));
    if (i16SendLen > 0)
    {
        ucEcc ^= GetMsgDataEcc((BYTE *)&acMsg[sizeof(T_PMSG_HEAD)], i16SendLen);
    }
    acMsg[sizeof(T_PMSG_HEAD) + i16SendLen] = ucEcc;

    pthread_mutex_lock(&ptPmsgConnInfo->tPmsgMutex);
    iRet = send(ptPmsgConnInfo->iSockfd, acMsg, sizeof(T_PMSG_HEAD) + i16SendLen + 1, 0);

    pthread_mutex_unlock(&ptPmsgConnInfo->tPmsgMutex);
    
    DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "PMSG_SendPmsgData Ok send data len=%d\n", iRet);
    return iRet;
}

int PMSG_GetDataFromPmsgQueue(PMSG_HANDLE pMsgHandle, PT_PMSG_PACKET ptPkt)
{
    int iRet = 0;
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;

    if (NULL == ptPmsgConnInfo)
    {
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_GetDataFromPmsgQueue error! pmsg handle is NULL\n");
        return -1;
    }

    iRet = GetNodeFromPmsgQueue(ptPmsgConnInfo->ptPmsgQueue, ptPkt);
    if (0 == iRet)
    {
        DebugPrint(DEBUG_PMSG_ERROR_PRINT, "PMSG_GetDataFromPmsgQueue error! data get error\n");
        return -1;
    }

    return 0;
}
