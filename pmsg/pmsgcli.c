
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include <dirent.h>

#include "types.h"
#include "pmsgcli.h"
#include "./include/mutex.h"
#include "debug.h"
#include "state.h"


typedef struct _T_PMSG_CONN_INFO
{
    int iSockfd;
    int iConnectStatus;
    int iThreadRunFlag;
    int iServPort;
    char acIpAddr[20];

#ifdef WIN
    HANDLE ThreadHandle;
#else
    pthread_t ThreadHandle;
#endif
    Mutex tPmsgMutex;
    PF_MSG_PROC_CALLBACK pMsgProcFunc;
} __attribute__((packed))T_PMSG_CONN_INFO, *PT_PMSG_CONN_INFO;


typedef struct _T_RES_CONN_INFO
{
	int iListenSocket;
	int iThreadRunFlag;
	int iConnSocket;
	int iServPort;
	//Mutex	tPmsgMutex;
	#ifdef WIN
    HANDLE ThreadHandle;
#else
    pthread_t ThreadHandle;
#endif
} __attribute__((packed))T_RES_CONN_INFO, *PT_RES_CONN_INFO;


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
    char acIpAddr[20];

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
    
    tv_out.tv_sec = 3;
    tv_out.tv_usec = 500;
    setsockopt(iSockfd, SOL_SOCKET, SO_RCVTIMEO, &tv_out, sizeof(tv_out));
    setsockopt(iSockfd, SOL_SOCKET, SO_SNDTIMEO, &tv_out, sizeof(tv_out));

    return iSockfd;
}

void DestroyTcpSocket(iSockfd)
{
	DebugPrint(DEBUG_PMSG_PRINT,"[%s] enter, sockfd %d", __FUNCTION__, iSockfd);
   	close(iSockfd);		
}

#ifdef WIN
	DWORD WINAPI CliProcessThread(void *arg)
#else
  void *CliProcessThread(void *arg)
#endif
{
    unsigned char *pcRecvBuf = NULL;
    unsigned char *pcLeaveBuf = NULL;
    unsigned char *pcMsgBuf = NULL;
    unsigned char ucMsgHead = 0;
    int iBufLen = 2048;
    int iSocket = 0;
    int iPreLeaveLen = 0, iLeaveLen = 0, iRecvLen = 0;
    int iMsgDataLen = 0;
    short sMsgCmd = 0;
    int iHearCount = 0;
    int iRet = 0;
    int iOffset = 0;
    fd_set	tAllSet, tTmpSet;
    struct timeval tv;
    time_t tCurTime = 0, tOldTime = 0;
    T_PMSG_CONN_INFO *ptPmsgConnInfo = (T_PMSG_CONN_INFO *)arg;
    PMSG_HANDLE pMsgHandle = (PMSG_HANDLE)arg;

	pthread_detach (pthread_self ());
    if (NULL == ptPmsgConnInfo)
    {
        return NULL;	
    }

	DebugPrint(DEBUG_PMSG_PRINT,"[%s] enter", __FUNCTION__);
    
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
        return NULL;        	
    }
    memset(pcLeaveBuf, 0, iBufLen);
    while (ptPmsgConnInfo &&( 1 == ptPmsgConnInfo->iThreadRunFlag))
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
				DebugPrint(DEBUG_PMSG_PRINT,"[%s] create tcp socket succ %d", __FUNCTION__,iSocket);
            }
            
        }
        if (iSocket <= 0)
        {
            sleep(2);
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
			DebugPrint(DEBUG_PMSG_PRINT,"create tcp socket faile %s, %d"
				, ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iServPort);
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
                iRecvLen = recv(iSocket, &pcRecvBuf[iPreLeaveLen], iBufLen - iPreLeaveLen - 1, 0);
                if (iRecvLen <= 0)
                {
                	DebugPrint(DEBUG_PMSG_PRINT,"nvr serv %s exit,recv error:%s", ptPmsgConnInfo->acIpAddr,strerror(errno));
                    DestroyTcpSocket(iSocket);
                    iSocket = -1;
                    ptPmsgConnInfo->iSockfd = -1;
                    ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
                    
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
                    if (iLeaveLen < sizeof(T_PMSG_HEAD))
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
                    iMsgDataLen = pcMsgBuf[3] << 8 | pcMsgBuf[4];
                    if (iMsgDataLen > 1048)
                    {
                        iPreLeaveLen = 0;
                        break;
                    }
                    
                    sMsgCmd = pcMsgBuf[1] << 8 | pcMsgBuf[2];
                        
                    if (iMsgDataLen <= iLeaveLen - sizeof(T_PMSG_HEAD))
                    {
                        if (ptPmsgConnInfo->pMsgProcFunc)
                        {
                            ptPmsgConnInfo->pMsgProcFunc(pMsgHandle, sMsgCmd, &pcMsgBuf[sizeof(T_PMSG_HEAD)], iMsgDataLen);
                        }
                        iLeaveLen -= iMsgDataLen + sizeof(T_PMSG_HEAD);
                        iOffset += iMsgDataLen + sizeof(T_PMSG_HEAD);
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
                iHearCount = 0;
            } 
            tOldTime = tCurTime;
        }
        
        iHearCount ++;

        if (iHearCount > 1000)
        {
            DestroyTcpSocket(iSocket);
            iSocket = -1;
            ptPmsgConnInfo->iSockfd = -1;
            ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;
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
    
   DebugPrint(DEBUG_PMSG_PRINT,"[%s] exit\n", __FUNCTION__); 
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
#ifdef WIN
	DWORD WINAPI SrvProcessThread(void *arg)
#else
  void *SrvProcessThread(void *arg)
#endif
{
	T_RES_CONN_INFO *ptResConnInfo = (T_RES_CONN_INFO *)arg;
	PMSG_HANDLE pMsgHandle = (PMSG_HANDLE)arg;
	int iMaxFd = 0;
    int iResult = 0;
    int iConnFd = 0;
    fd_set	tReadSet;
    struct timeval tv;
    char acBuf[128];
    struct sockaddr_in cliaddr;
    int iCliLen = sizeof(cliaddr);
	int iHeartTimeout = 0;
	
    if (NULL == ptResConnInfo)
    {
        return NULL;	
    }
	
	while(ptResConnInfo->iThreadRunFlag)
	{
		iMaxFd = (ptResConnInfo->iConnSocket > ptResConnInfo->iListenSocket) ?
		ptResConnInfo->iConnSocket : ptResConnInfo->iListenSocket;
		FD_ZERO(&tReadSet);
		if (ptResConnInfo->iListenSocket > 0)
		{
		  	FD_SET(ptResConnInfo->iListenSocket, &tReadSet);
		}
		if (ptResConnInfo->iConnSocket > 0)
		{
		  	FD_SET(ptResConnInfo->iConnSocket, &tReadSet);	
		}
		
    	tv.tv_sec  = 0;
    	tv.tv_usec = 20000;
    
		iResult = select(iMaxFd + 1, &tReadSet, NULL, NULL, &tv);
		if(iResult <= 0)
		{
			iHeartTimeout++;  
			if (iHeartTimeout > 200 && ptResConnInfo->iConnSocket > 0)
			{
				//MUTEX_LOCK(&ptResConnInfo->tPmsgMutex);
            	strcpy(acBuf, "Update res Timeout!");
				close(ptResConnInfo->iConnSocket);
            	ptResConnInfo->iConnSocket = -1;
            	iHeartTimeout = 0;
            	//MUTEX_UNLOCK(&ptResConnInfo->tPmsgMutex);
            	DebugPrint(DEBUG_PMSG_PRINT,"%s", acBuf);
				SetResUpdateState(E_RES_UPDATE_UNKNOW, 0);
			}
		}
		else
		{
        	if(FD_ISSET(ptResConnInfo->iListenSocket, &tReadSet)) 
        	{
            	iConnFd = accept(ptResConnInfo->iListenSocket, (struct sockaddr *)&cliaddr, &iCliLen);
            	if (iConnFd > 0)
            	{
                	if (ptResConnInfo->iConnSocket > 0)
                	{
                    	//MUTEX_LOCK(&ptResConnInfo->tPmsgMutex);
                    	close(ptResConnInfo->iConnSocket);
                    	ptResConnInfo->iConnSocket = -1;
                    	//MUTEX_UNLOCK(&ptResConnInfo->tPmsgMutex);
                	}
					ptResConnInfo->iConnSocket = iConnFd;
                	iHeartTimeout = 0;
					tv.tv_sec = 1;
                	tv.tv_usec = 0;
                	if (setsockopt(ptResConnInfo->iConnSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)))
                	{
                		DebugPrint(DEBUG_PMSG_PRINT,"[%s] setsockopt err:%s"
							, __FUNCTION__,strerror(errno));
                	}
                }
            }
        	if (ptResConnInfo->iConnSocket > 0)
        	{
            	if(FD_ISSET(ptResConnInfo->iConnSocket, &tReadSet)) 
            	{
                	unsigned char acRecvBuf[64];
                	unsigned char ucType = 0;
                	unsigned int uiLevel = 0;
                	int iRecvLen = 0;
                
                	memset(acRecvBuf, 0, sizeof(acRecvBuf));
                	iRecvLen = recv(ptResConnInfo->iConnSocket, acRecvBuf, sizeof(acRecvBuf), 0);
                	if (iRecvLen <= 0)
                	{
                		char cState =0,cProgress = 0;
						GetResUpdateState(&cState, &cProgress);
					
                    	close(ptResConnInfo->iConnSocket);
                    	ptResConnInfo->iConnSocket = -1;
						if(iRecvLen <0)
						{
							DebugPrint(DEBUG_PMSG_PRINT,"[%s]Update res recv err:%s"
							, __FUNCTION__,strerror(errno));				
						}
						if(cState != 0x03 || cState != 0x04)
						{
							SetResUpdateState(E_RES_UPDATE_FAIL, 0); //非正常结束 默认为失败
						}
						
						usleep(20000);
						continue;
                	}
					iHeartTimeout = 0;
                	if (0xCA != acRecvBuf[0] || 9 != acRecvBuf[1] || 0x51 != acRecvBuf[2])
                	{
                		usleep(100000);
						DebugPrint(DEBUG_PMSG_PRINT,"[%s]Recv Data err:%s"
							, __FUNCTION__);
                   		continue;
                	}
					SetResUpdateState(acRecvBuf[3], acRecvBuf[4]);
            	}
        	}
		}
		usleep(20000);
	}
	return 0;
}

PMSG_HANDLE	PMSG_CreateResConn(int iPort)
{
	PT_RES_CONN_INFO ptResConnInfo = NULL;
    int iRet = 0;
    int flag = 1;
    int iSockFd = -1;
    struct sockaddr_in servaddr;
    pthread_mutexattr_t mutexattr;
	
    ptResConnInfo = (PT_RES_CONN_INFO)malloc(sizeof(T_RES_CONN_INFO));
    if (NULL == ptResConnInfo)
    {
        return 0;	
    }
	
    memset(ptResConnInfo, 0, sizeof(T_RES_CONN_INFO));
    iSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (iSockFd < 0)
    {
    	DebugPrint(DEBUG_PMSG_PRINT,"[%s]socket error:%s"
							, __FUNCTION__,strerror(errno));
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(iPort);

    iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int));
    if(iRet != 0)
    {
    	DebugPrint(DEBUG_PMSG_PRINT,"[%s]setsockopt socket err:%s"
							, __FUNCTION__,strerror(errno));
        close(iSockFd);
        return -1;
    }
	
    iRet = bind(iSockFd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (iRet < 0)
    {
        close(iSockFd);
        return -1;
    }
    listen(iSockFd, 1);    
    ptResConnInfo->iListenSocket = iSockFd;	
	ptResConnInfo->iThreadRunFlag = 1;
	//MUTEX_INIT(&ptResConnInfo->tPmsgMutex);
    iRet = pthread_create(&ptResConnInfo->ThreadHandle, NULL, SrvProcessThread, (void *)ptResConnInfo);
    return 0;
}

int	PMSG_DestroyResConn(PMSG_HANDLE pMsgHandle)
{
	T_RES_CONN_INFO *ptResConnInfo = (T_RES_CONN_INFO *)pMsgHandle;
	ptResConnInfo->iThreadRunFlag = 0;
	pthread_join(ptResConnInfo->ThreadHandle, NULL);
	free(ptResConnInfo);
    ptResConnInfo = NULL;
	return 0;
}


PMSG_HANDLE PMSG_CreateConnect(char *pcIpAddr, int iPort, PF_MSG_PROC_CALLBACK pfMsgProcFunc)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = NULL;
    int iRet = 0;
    
    ptPmsgConnInfo = (PT_PMSG_CONN_INFO)malloc(sizeof(T_PMSG_CONN_INFO));
    if (NULL == ptPmsgConnInfo)
    {
        return 0;	
    }
    memset(ptPmsgConnInfo, 0, sizeof(T_PMSG_CONN_INFO));
    
    MUTEX_INIT(&ptPmsgConnInfo->tPmsgMutex);
    
    ptPmsgConnInfo->iThreadRunFlag = 1;
    ptPmsgConnInfo->pMsgProcFunc = pfMsgProcFunc;
    strncpy(ptPmsgConnInfo->acIpAddr, pcIpAddr, sizeof(ptPmsgConnInfo->acIpAddr));
    ptPmsgConnInfo->iServPort = iPort;
	ptPmsgConnInfo->iConnectStatus = E_SERV_STATUS_UNCONNECT;

	DebugPrint(DEBUG_PMSG_PRINT,"[%s]serv conn ip:%s, port:%d"
							, __FUNCTION__,ptPmsgConnInfo->acIpAddr, ptPmsgConnInfo->iServPort);
    #ifdef WIN
    ptRtpConn->ThreadHandle = CreateThread(NULL, 0, CliProcessThread, ptPmsgConnInfo, 0, NULL);
#else
    iRet = pthread_create(&ptPmsgConnInfo->ThreadHandle, NULL, CliProcessThread, (void *)ptPmsgConnInfo);
    if (iRet < 0)
    {
        free(ptPmsgConnInfo);
        ptPmsgConnInfo = NULL;
        return 0;	
    }
#endif

    return (PMSG_HANDLE)ptPmsgConnInfo;
}

int PMSG_DestroyConnect(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    
    if (NULL == ptPmsgConnInfo)
    {
        return -1;	
    }
    ptPmsgConnInfo->iThreadRunFlag = 0;
    
    // join thread exit
    if (ptPmsgConnInfo->ThreadHandle)
    {
    #ifdef WIN
        if (WAIT_OBJECT_0 == WaitForSingleObject(ptPmsgConnInfo->ThreadHandle, 100))
        {
            CloseHandle(ptPmsgConnInfo->ThreadHandle);
        } 
        else
        {
            DWORD nExitCode;
            GetExitCodeThread(ptPmsgConnInfo->ThreadHandle, &nExitCode);
            if (STILL_ACTIVE == nExitCode)
            {
                TerminateThread(ptPmsgConnInfo->ThreadHandle, nExitCode);
                CloseHandle(ptPmsgConnInfo->ThreadHandle);
            }						
        }
    #else
        //pthread_join(ptPmsgConnInfo->ThreadHandle, NULL);
    #endif
    }
    //pthread_mutex_destroy(&ptPmsgConnInfo->tPmsgMutex);
    MUTEX_DESTROY(&ptPmsgConnInfo->tPmsgMutex);
    free(ptPmsgConnInfo);
    ptPmsgConnInfo = NULL;
    
    return 0;
}

int PMSG_GetConnectStatus(PMSG_HANDLE pMsgHandle)
{
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    
    if (NULL == ptPmsgConnInfo)
    {
        return E_SERV_STATUS_UNCONNECT;	
    }

    return ptPmsgConnInfo->iConnectStatus;
}

int PMSG_SendRawData(PMSG_HANDLE pMsgHandle, char *pcData, int iDataLen)
{
	  char acMsg[64];
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    int iRet = 0;
    
    if ((NULL == ptPmsgConnInfo) || (NULL == pcData) || (iDataLen <= 0))
    {
        return -1;	
    }
    if (E_SERV_STATUS_CONNECT != ptPmsgConnInfo->iConnectStatus)
    {
        return -1;
    }

    MUTEX_LOCK(&ptPmsgConnInfo->tPmsgMutex);
    iRet = send(ptPmsgConnInfo->iSockfd, pcData, iDataLen, 0);
    MUTEX_UNLOCK(&ptPmsgConnInfo->tPmsgMutex);
    
    return iRet;
}

int PMSG_SendPmsgData(PMSG_HANDLE pMsgHandle, unsigned short usMsgCmd, char *pcData, int iDataLen)
{
    char acMsg[1024];
    unsigned char ucEcc = 0;
    PT_PMSG_CONN_INFO ptPmsgConnInfo = (PT_PMSG_CONN_INFO)pMsgHandle;
    PT_PMSG_HEAD ptMsgHead = NULL;
    short i16SendLen = 0;
    int iRet = 0;
    
    if (NULL == ptPmsgConnInfo)
    {
        return -1;	
    }
    if (E_SERV_STATUS_CONNECT != ptPmsgConnInfo->iConnectStatus)
    {
        return -1;
    }
    if (iDataLen > (1024 - sizeof(T_PMSG_HEAD)))
    {
        DebugPrint(DEBUG_PMSG_PRINT,"[%s]Msglen %d is too long\n", __FUNCTION__, iDataLen);
        iDataLen = 1024 -sizeof(T_PMSG_HEAD);	
    }
    
    memset(acMsg, 0, sizeof(acMsg));
    ptMsgHead = (PT_PMSG_HEAD)acMsg;
    if (iDataLen > 0)
    {
        memcpy(&acMsg[sizeof(T_PMSG_HEAD)], pcData, iDataLen);
        i16SendLen = iDataLen;
    }
    ptMsgHead->cMsgStartFLag = MSG_START_FLAG;
    ptMsgHead->sMsgCmd = htons(usMsgCmd);
    ptMsgHead->sDataLen = htons(i16SendLen);
            
    // 计算ECC校验
    //ucEcc = GetMsgDataEcc((BYTE *)ptMsgHead, sizeof(T_PMSG_HEAD));
    if (i16SendLen > 0)
    {
        ucEcc = GetMsgDataEcc(&acMsg[sizeof(T_PMSG_HEAD)], i16SendLen);
    }       
    ptMsgHead->cDataEcc = ucEcc;

    MUTEX_LOCK(&ptPmsgConnInfo->tPmsgMutex);
    iRet = send(ptPmsgConnInfo->iSockfd, acMsg, sizeof(T_PMSG_HEAD) + i16SendLen, 0);
    MUTEX_UNLOCK(&ptPmsgConnInfo->tPmsgMutex);
    
    return iRet;
}
