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

#include "NVRMsgProc.h"
#include "state.h"
#include "pmsgcli.h"
#include <list>
#include "log.h"

using namespace std;

#define NVR_PORT 10100

typedef struct _T_CMD_PACKET_LIST
{
    T_CMD_PACKET tPkt;
    struct _T_CMD_PACKET_LIST *next;
} __attribute__((packed))T_CMD_PACKET_LIST;

typedef struct _T_CMD_QUEUE
{
    T_CMD_PACKET_LIST *ptFirst, *ptLast;
    int iPktCount;
    pthread_mutex_t tMutex;
				
}__attribute__((packed)) T_CMD_QUEUE, *PT_CMD_QUEUE;

typedef struct _T_NVR_NET_INFO
{
	PMSG_HANDLE  NVRMsgHandle;
	PT_CMD_QUEUE PtCmdQueue;
	PT_CMD_QUEUE ptFileQueue;
	int			 iThreadRun;
}__attribute__((packed))T_NVR_NET_INFO;

static T_NVR_NET_INFO g_tNVRNetnfo[6];
static pthread_t    g_cmdProcessThreadHandle;
static int 			g_iCmdProcessThreadRunFlag =0;


PT_CMD_QUEUE CreateCmdQueue()
{
    T_CMD_QUEUE *ptCmdQueue = NULL;
    pthread_mutexattr_t	mutexattr;
    ptCmdQueue = (PT_CMD_QUEUE)malloc(sizeof(T_CMD_QUEUE));
    if (NULL == ptCmdQueue)
    {
        return NULL;
    }
    memset(ptCmdQueue, 0, sizeof(T_CMD_QUEUE));
	
	pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_TIMED_NP);
    pthread_mutex_init(&ptCmdQueue->tMutex, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);

    ptCmdQueue->ptLast = NULL;
    ptCmdQueue->ptFirst = NULL;
    ptCmdQueue->iPktCount= 0;

    return ptCmdQueue;
}

int CleanNodeFromCmdQueue(PT_CMD_QUEUE ptCmdQueue)
{
	T_CMD_PACKET_LIST *ptPktList = NULL, *ptTmp;

    if (NULL == ptCmdQueue)
    {
        return -1;
    }
  
    pthread_mutex_lock(&ptCmdQueue->tMutex);
   
    ptPktList = ptCmdQueue->ptFirst;
    while (ptPktList)
    {
        ptTmp = ptPktList;
		free(ptPktList->tPkt.pData);
        ptPktList = ptPktList->next;
        free(ptTmp);
    }
	
    ptCmdQueue->ptLast = NULL;
    ptCmdQueue->ptFirst = NULL;
    ptCmdQueue->iPktCount= 0;   
    pthread_mutex_unlock(&ptCmdQueue->tMutex);
    return 0;
}
static int FindHandleNo(PMSG_HANDLE PHandle)
{
	for(int i=0;i<6;i++)
	{
		if(g_tNVRNetnfo[i].NVRMsgHandle == PHandle)
		{
			return i;
		}
	}
	return -1;
}


int DestroyCmdQueue(PT_CMD_QUEUE ptCmdQueue)
{
    if (NULL == ptCmdQueue)
    {
        return -1;
    }
    CleanNodeFromCmdQueue(ptCmdQueue);
    free(ptCmdQueue);
    ptCmdQueue = NULL;
    return 0;
}

int PutNodeToCmdQueue(PT_CMD_QUEUE ptCmdQueue, PT_CMD_PACKET ptPkt)
{
    T_CMD_PACKET_LIST *ptPktList = NULL;


    if ((NULL == ptCmdQueue) || (NULL == ptPkt))
    {
        return -1;
    }
    ptPktList = (T_CMD_PACKET_LIST *)malloc(sizeof(T_CMD_PACKET_LIST));
    if (NULL == ptPktList)
    {
        return -1;
    }

    memset(ptPktList, 0, sizeof(T_CMD_PACKET_LIST));
    ptPktList->tPkt = *ptPkt;
    pthread_mutex_lock(&ptCmdQueue->tMutex);
    
    if (NULL == ptCmdQueue->ptLast)
    {
	    ptCmdQueue->ptFirst = ptPktList;
    }
    else
    {
	    ptCmdQueue->ptLast->next = ptPktList;
    }
    ptCmdQueue->ptLast = ptPktList;
    ptCmdQueue->iPktCount++;
    pthread_mutex_unlock(&ptCmdQueue->tMutex);
   
    return 0;
}

int GetNodeFromCmdQueue(PT_CMD_QUEUE ptCmdQueue, PT_CMD_PACKET ptPkt)
{
    T_CMD_PACKET_LIST *ptTmp = NULL;

    if ((NULL == ptCmdQueue) || (NULL == ptPkt))
    {
        return 0;
    }

    pthread_mutex_lock(&ptCmdQueue->tMutex);
    
    if (NULL == ptCmdQueue->ptFirst)
    {        
        pthread_mutex_unlock(&ptCmdQueue->tMutex);
        return 0;
    }
	

    pthread_mutex_unlock(&ptCmdQueue->tMutex);
    ptTmp = ptCmdQueue->ptFirst;
    ptCmdQueue->ptFirst = ptCmdQueue->ptFirst->next;
    if (NULL == ptCmdQueue->ptFirst)
    {
        ptCmdQueue->ptLast= NULL;
    }
    ptCmdQueue->iPktCount--;
    *ptPkt = ptTmp->tPkt;
    free(ptTmp);
    
    return 1;
}


int PmsgProc(PMSG_HANDLE PHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen)
{
	int iIPCNum = 0;
	int iNvrNo = FindHandleNo(PHandle);
	T_CMD_PACKET tPkt;
	int iNum =0;

    switch(ucMsgCmd)
    {
    case SERV_CLI_MSG_TYPE_HEART:        
    	break;
	case SERV_CLI_MSG_TYPE_GET_HDISK_STATUS_RESP:
		if(iNvrNo >=0)
		{
			SetNvrDiskState(iNvrNo, (T_DISK_STATE *)pcMsgData);
		}	
		break;
	case SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP:
		iIPCNum = (pcMsgData[0]<<8 )| pcMsgData[1];
		iNum = GetNvrVideoNum(iNvrNo);
		for(int i=0; i<iNum;i++)
		{
			int iIndex = GetNvrVideoIdx(iNvrNo, i);
			SetVideoOnlineState(iIndex,pcMsgData[2+iIndex]); //前面iIndex个为在线状态
			SetVideoWarnState(iIndex, pcMsgData[2 +iIndex +iIPCNum]);//后面为遮挡状态
		}
		break;
	case SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP:
		{			
			E_FILE_SEARCH_STATE eFileState;
			eFileState = GetFileSearchState();		
			if((iNvrNo >= 0) && (E_FILE_SEARCHING == eFileState))
			{
				tPkt.iMsgCmd = SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP;

				if (iMsgDataLen > 2)
				{
					tPkt.pData = new char[iMsgDataLen-1]; //前2个字节为包的序号
					tPkt.iDataLen = iMsgDataLen-2;
					strncpy(tPkt.pData,pcMsgData+2,iMsgDataLen-2);
					tPkt.pData[iMsgDataLen-2] = 0;
					PutNodeToCmdQueue(g_tNVRNetnfo[iNvrNo].ptFileQueue, &tPkt);
				}
			}
		}
		break;    
    }
	return 0;
}

void *CmdProcessThread(void *arg)
{	
	while(g_iCmdProcessThreadRunFlag)
	{
		for(int i=0;i<6;i++)
		{
			T_NVR_NET_INFO *ptNvrInfo = (T_NVR_NET_INFO *) &g_tNVRNetnfo[i];
			T_CMD_PACKET tPkt;

			memset(&tPkt, 0, sizeof(tPkt));
			while(GetNodeFromCmdQueue(ptNvrInfo->PtCmdQueue, &tPkt))
			{
				PMSG_SendPmsgData(ptNvrInfo->NVRMsgHandle,tPkt.iMsgCmd,tPkt.pData,tPkt.iDataLen);
				if(tPkt.iDataLen >0)
				{
					free(tPkt.pData);
					tPkt.pData = NULL;
				}
			}
		}
		usleep(30000);
	}
	
	return NULL;
}

int NVR_init()
{
	g_iCmdProcessThreadRunFlag = 1;
	PMSG_Init();
	memset(g_tNVRNetnfo,0,sizeof(g_tNVRNetnfo));

	for(int i=0;i<6;i++)
	{
		NVR_Connect(i);
	}
	
	pthread_create(&g_cmdProcessThreadHandle, NULL, CmdProcessThread, NULL);
	return 0;

}

int NVR_Uninit()
{
	g_iCmdProcessThreadRunFlag =0;
	pthread_join(g_cmdProcessThreadHandle,NULL);
	PMSG_Uninit();
	return 0;
}


int NVR_Connect(int iNvrNo)
{
	char acIp[24] = {0};
	if(iNvrNo <0 || iNvrNo >5 || g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}

	memset(acIp, 0, sizeof(acIp));
	GetNvrIpAddr(iNvrNo, acIp);
	if(acIp[0] == 0)
	{
		return -1;
	}
	g_tNVRNetnfo[iNvrNo].iThreadRun = 1;
	g_tNVRNetnfo[iNvrNo].PtCmdQueue = CreateCmdQueue();
	g_tNVRNetnfo[iNvrNo].ptFileQueue = CreateCmdQueue();
	g_tNVRNetnfo[iNvrNo].NVRMsgHandle = PMSG_CreateConnect(acIp,NVR_PORT,PmsgProc);
	return 0;
}

int NVR_DisConnect(int iNvrNo)
{
	if(iNvrNo <0 || iNvrNo >5 || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}
		
	g_tNVRNetnfo[iNvrNo].iThreadRun = 0;
	PMSG_DestroyConnect(g_tNVRNetnfo[iNvrNo].NVRMsgHandle);
	g_tNVRNetnfo[iNvrNo].NVRMsgHandle = 0;
	DestroyCmdQueue(g_tNVRNetnfo[iNvrNo].PtCmdQueue);
	g_tNVRNetnfo[iNvrNo].PtCmdQueue = NULL;
	return 0;
}

int NVR_SendCmdInfo(int iNvrNo,int iCmd,char *pData,int iDataLen)
{
	if(iNvrNo <0 || iNvrNo >5 ||0 == g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}
	T_CMD_PACKET tPkt;

	memset(&tPkt, 0, sizeof(tPkt));
	tPkt.iMsgCmd = iCmd;
	if (iDataLen > 0)
	{
		tPkt.pData = (char *)malloc(iDataLen+1);
		memcpy(tPkt.pData,pData,iDataLen);
		tPkt.iDataLen = iDataLen;
	}

	PutNodeToCmdQueue(g_tNVRNetnfo[iNvrNo].PtCmdQueue, &tPkt);
	return 0;
}

int NVR_GetFileInfo(int iNvrNo,T_CMD_PACKET *ptPkt)
{
	if(iNvrNo <0 || iNvrNo >5 || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}
	if(GetNodeFromCmdQueue(g_tNVRNetnfo[iNvrNo].ptFileQueue, ptPkt))
	{
		return 1;
	}
	
	return 0;
}

int NVR_CleanFileInfo(int iNvrNo)
{
	if(iNvrNo <0 || iNvrNo >5 || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return -1;
	}
	return CleanNodeFromCmdQueue(g_tNVRNetnfo[iNvrNo].ptFileQueue);	
}


int NVR_GetConnectStatus(int iNvrNo)
{
	if(iNvrNo <0 || iNvrNo >5 || 0 ==  g_tNVRNetnfo[iNvrNo].NVRMsgHandle)
	{
		return E_SERV_STATUS_UNCONNECT;
	}
	return PMSG_GetConnectStatus(g_tNVRNetnfo[iNvrNo].NVRMsgHandle);
}


