#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "state/fileConfig.h"
#include "state/state.h"

#include "./log/log.h"
#define  VIDEO_CH_MAX_SIZE 32


int g_iPageNumber = 0;

typedef struct _T_CARRIAGE_WARN_INFO_
{
    u8int cFireWarn;
	u8int cDoorWarn;
    u8int cDoorClipWarn;
}__attribute__((packed))T_CARRIAGE_WARN_INFO, *PT_CARRIAGE_WARN_INFO;
#define MAX_SERVER_NUM 6    //最大服务器个数

static PMSG_HANDLE nvrServerPmsgHandle[MAX_SERVER_NUM] = {0, 0, 0, 0};    //nvr服务器pmsg句柄

static T_NVR_STATE g_atNVRState[6];

static E_FILE_SEARCH_STATE g_eFileSearch;

static E_FILE_DOWN_STATE g_eFileDownState;
static int g_iFileProgress;
static int g_iVideoNum;

static int g_iASCUMasSlaFlag;
static int g_iMoniEnable;
static int g_iNoiseEnable;
static int g_iDriverRoonSpeakerVol;
static int g_iCarriageSpeakVolum;
static int g_iLcdVolum;

static int g_iVideoSource;			//0x11 直播 0x12 数字电视 0x13 录播
static int g_iDynamicMapBrightness;
static u32int 	g_iPecuWarnValue;
static int 		g_iPecuFirstWarnVideoIdx;		//PECU当前优先报警的索引

static T_CARRIAGE_WARN_INFO g_atCarriageWarn[6];
static char		g_acPecuVideoIdx[24];
static char		g_acFireVideoIdx[6];
static char		g_acDoorVideoIdx[48];
static char		g_cResUpdateState;      //资源的更新状态 0:未连接 1:开始更新 2：更新中 3：更新成功 4：更新失败
static char		g_cResUpdateProgress;	//素材更新进度
static char		g_acCCTVRunDir[256] = {0};
static int 		g_iDispState = DISP_STATE_UNKOWN;
static char     g_acConfigFileDir[]="/home/data";
static char		g_acBroadcastConfFile[]="BroadcastInfoConfig.ini";
static char		g_acCCTVConfigFile[]="CCTVConfig.ini";
static char     g_acIpAddr[16] = {0};

typedef struct _T_IPC_INFO
{
    int iNvrNO;
	int iUiGroup;
	int iUiPos;
	char acRtspAddr[256];
	int iImgIndex;
	int iOnlineState;
	int iWarnState;
    char acMainRtspAddr[256];
}__attribute__((packed))T_IPC_INFO;

typedef struct _T_IPC_NAME_MAP
{
    int iImgIdx;
	char acIpcName[8];
} __attribute__((packed))T_IPC_NAME_MAP;

static T_IPC_INFO g_atIpcInfo[VIDEO_CH_MAX_SIZE];
static T_IPC_NAME_MAP g_atIpcNameMap[VIDEO_CH_MAX_SIZE] = 
{
	{1,  "1F"},{2,  "1A"},{3,  "1B"},{4,  "1C"},
	{5,  "1D"},{6,  "2A"},{7,  "2B"},{8,  "2C"},
	{9,  "2D"},{10, "3A"},{11, "3B"},{12, "3C"},
	{13, "3D"},{14, "4A"},{15, "4B"},{16, "4C"},
	{17, "4D"},{18, "5A"},{19, "5B"},{20, "5C"},
	{21, "5D"},{22, "6A"},{23, "6B"},{24, "6C"},
	{25, "6D"},{26, "6F"},{27, "1E"},{28, "6E"},
	{29, "1G"},{30, "6G"},{31, "G1"},{32, "G2"}
};

int FindVideoNameAccordImgIdx(int iImgIndex, char *pcIpcName, int iLen)
{
	int i=0;
	
    if (iImgIndex > VIDEO_CH_MAX_SIZE || iImgIndex <1)
    {
    	return -1;
    }
	for(;i<VIDEO_CH_MAX_SIZE;i++)
	{
		if(g_atIpcNameMap[i].iImgIdx == iImgIndex)
		{
			strncpy(pcIpcName,g_atIpcNameMap[i].acIpcName,iLen);
			return 0;
		}
	}
	return -1;
}

static int AddUserPasswordToRtsp(char *pRtsp,char *pRtspDst,int DstLen, char *pUser,char *pPassword)
{
	char * pstr1 = NULL;
  
    pstr1 = strstr(pRtsp, "://");
    if ( NULL == pstr1 ) 
    {
        return -1;
    }

	if( (0 == strlen(pUser)) || (0 == strlen(pPassword)))
	{
		snprintf(pRtspDst,DstLen-1,"rtsp://%s",(char *)(pstr1+3));
	}
	else
	{
		snprintf(pRtspDst,DstLen-1,"rtsp://%s:%s@%s",pUser,pPassword,(char *)(pstr1+3));
	}

   return 0;
}


static int ParseIpcInfo(char *pcParseStr, T_IPC_INFO *ptIpcInfo,char *pcUser,char *pcPswd)
{
	char *pcPos = NULL;//strtok(pcParseStr,"+");
	char *pcTmp = NULL;
	int iIndex = 0;
	int iRet = 0;

   /* pcPos = pcParseStr;
    
    // NvrNO
    pcTmp = strsep(&pcPos, "+");
    if (pcTmp)
    {
        ptIpcInfo->iNvrNO = atoi(pcTmp) - 1;
		
		if(ptIpcInfo->iNvrNO <0 || ptIpcInfo->iNvrNO >5)
		{
			iRet = -1;
			goto ERR;	
		}
    }
    else
    {
    	iRet = -2;
		goto ERR;	
    }

	if (pcPos)
	{
		pcTmp = strsep(&pcPos, "+");
	}
	else
	{
		iRet = -3;
		goto ERR;	
	}

	//group
    if (pcTmp)
    {
        ptIpcInfo->iUiGroup = atoi(pcTmp)-1;
		
		if(ptIpcInfo->iUiGroup <0 || ptIpcInfo->iUiGroup >7)
		{
			iRet = -4;
			goto ERR;
		}
    }
    else
    {
    	iRet = -5;
		goto ERR;
    }

	if (pcPos)
	{
		pcTmp = strsep(&pcPos, "+");
	}
	else
	{
	   	iRet = -6;
		goto ERR;
	}

	//pos
    if (pcTmp)
    {
		ptIpcInfo->iUiPos = atoi(pcTmp)-1;
		
		if(ptIpcInfo->iUiPos <0 || ptIpcInfo->iUiPos >3)
		{
			iRet = -7;
			goto ERR;
		}
    }
    else
    {
    	iRet = -8;
		goto ERR;
    }

	if (pcPos)
	{
		pcTmp = strsep(&pcPos, "+");
	}
	else
	{
	    iRet = -9;
		goto ERR;
	}

	//imgIdx
    if (pcTmp)
    {
		ptIpcInfo->iImgIndex = atoi(pcTmp);
		
		if(ptIpcInfo->iImgIndex <1 || ptIpcInfo->iImgIndex >32)
		{
			iRet = -10;
			goto ERR;
		}
    }
    else
    {
    	iRet = -11;
		goto ERR;	
    }

    if (pcPos)
	{
		pcTmp = strsep(&pcPos, "+");
	}
	
	//RtspAddr
	if (pcPos)
	{
		AddUserPasswordToRtsp(pcPos,ptIpcInfo->acRtspAddr,sizeof(ptIpcInfo->acRtspAddr),pcUser,pcPswd);
		//strcpy(ptIpcInfo->acRtspAddr,pcPos);
	}
	else
	{
		iRet = -12;
		goto ERR;
	}
	return 0;
ERR:
	printf("[%s]%d err iRet =%d\n",__FUNCTION__,__LINE__,iRet);
	return iRet;*/

    pcPos = strtok(pcParseStr,"+");

    //nvr+group+pos+imgindex+subrtsp+mainrtsp
	while(pcPos)
	{
		if(0 == iIndex)
		{
			ptIpcInfo->iNvrNO = atoi(pcPos)-1;
			if(ptIpcInfo->iNvrNO <0 || ptIpcInfo->iNvrNO >5)
			{
				iRet = -1;
			}
		}
		else if(1 == iIndex)
		{
			ptIpcInfo->iUiGroup = atoi(pcPos)-1;
			if(ptIpcInfo->iUiGroup <0 || ptIpcInfo->iUiGroup >7)
			{
				iRet = -2;
			}
		}
		else if(2 == iIndex)
		{
			ptIpcInfo->iUiPos = atoi(pcPos)-1;
			if(ptIpcInfo->iUiPos <0 || ptIpcInfo->iUiPos >3)
			{
				iRet = -3;
			}
		}
		else if(3 == iIndex)
		{
			ptIpcInfo->iImgIndex = atoi(pcPos);
			if(ptIpcInfo->iImgIndex <1 || ptIpcInfo->iImgIndex >32)
			{
				iRet = -4;
			}
		}
		else if(4 == iIndex)
		{	
			//strcpy(ptIpcInfo->acRtspAddr,pcPos);
            AddUserPasswordToRtsp(pcPos,ptIpcInfo->acRtspAddr,sizeof(ptIpcInfo->acRtspAddr),pcUser,pcPswd);	
		}
        else if(5 == iIndex)
        {
            AddUserPasswordToRtsp(pcPos,ptIpcInfo->acMainRtspAddr,sizeof(ptIpcInfo->acMainRtspAddr),pcUser,pcPswd);
            break;
        }
		pcPos = strtok(NULL,"+");
		iIndex ++;
	}
    
	if((iIndex != 4 && iIndex != 5) || iRet <0)
	{
		printf("parseIpcInfo err\n");
		return -1;
	}
	
	return 0;
}

static int ReadWarnRelate()
{
	char acParseStr[256] = {0};
	char acKeyValue[24] = {0};
	int iRet = 0;
	int i = 0;
	int j =0;
	char acIniPath[256] ={0};
	
	snprintf(acIniPath,sizeof(acIniPath)-1,"%s/%s",g_acConfigFileDir,g_acCCTVConfigFile);
	for(i=0;i<24;i++)
	{
		int iValue = 0xff;
		
		memset(acParseStr,0,sizeof(acParseStr));
		memset(acKeyValue,0,sizeof(acKeyValue));
		sprintf(acKeyValue,"PECU%d",i+1);		
		iRet = ReadParam(acIniPath, "[PECUCONFIG]", acKeyValue, acParseStr);
		if (iRet > 0)
		{
			iValue = atoi(acParseStr)-1;
			if(iValue < 0 || iValue >= g_iVideoNum)
			{
				iValue = 0xff;
			}		
			g_acPecuVideoIdx[i] = iValue;  //相机是从0开始存储的，配置文档里面对应的是从1开始
		}
	}
	
	for(i=0;i<6;i++)
	{
		int iValue = 0xff;
		memset(acParseStr,0,sizeof(acParseStr));
		memset(acKeyValue,0,sizeof(acKeyValue));
		sprintf(acKeyValue,"FIRE%d",i+1);		
		iRet = ReadParam(acIniPath, "[FIRECONFIG]", acKeyValue, acParseStr);
		if (iRet > 0)
		{
			iValue = atoi(acParseStr)-1;
			if(iValue < 0 || iValue  >= g_iVideoNum)
			{
				iValue = 0xff;
			}		
			g_acFireVideoIdx[i] = iValue;
		}
	}
	
	for(i=0;i<6;i++)
		for(j=0;j<8;j++)
	{
		int iValue = 0xff;
		
		memset(acParseStr,0,sizeof(acParseStr));
		memset(acKeyValue,0,sizeof(acKeyValue));
		sprintf(acKeyValue,"DOOR%d%d",i+1,j+1);		
		iRet = ReadParam(acIniPath, "[DOORCONFIG]", acKeyValue, acParseStr);
		if (iRet > 0)
		{
			iValue = atoi(acParseStr)-1;
			if(iValue < 0 || iValue  >= g_iVideoNum)
			{
				iValue = 0xff;
			}	
			g_acDoorVideoIdx[i*8+j] = iValue;
		}
	}
	return 0;
}

static int ReadNvrAndIpc()
{
	char acParseStr[256] = {0};
	char acKeyValue[24] = {0};
	int iRet = 0;
	int i;
	char acIniPath[256] ={0};
	char acUser[16] = {0};
	char acPassword[16] = {0};
	
	snprintf(acIniPath,sizeof(acIniPath)-1,"%s/%s",g_acConfigFileDir,g_acCCTVConfigFile);

	for(i=0;i<6;i++)
	{
		memset(acParseStr,0,sizeof(acParseStr));
		memset(acKeyValue,0,sizeof(acKeyValue));
		sprintf(acKeyValue,"NVRIP%d",i+1);		
		iRet = ReadParam(acIniPath, "[NVRIPAddr]", acKeyValue, acParseStr);
		if (iRet > 0)
		{
			SetNvrIpAddr(i,acParseStr);
		}
	}
		
	ReadParam(acIniPath, "[IPCACCOUNT]", "USER", acUser);
	
	ReadParam(acIniPath, "[IPCACCOUNT]", "PASSWORD", acPassword);
		
	memset(acParseStr,0,sizeof(acParseStr));
	memset(acKeyValue,0,sizeof(acKeyValue));
	iRet = ReadParam(acIniPath, "[IPCINFO]", "IPCNUM", acParseStr);
	if(iRet >0)
	{
		g_iVideoNum = atoi(acParseStr);
		if((g_iVideoNum) != 28 && (g_iVideoNum != 32))
		{
			g_iVideoNum = 32;
		}
		
		for(i=0;i<g_iVideoNum;i++)
		{	
			memset(acParseStr,0,sizeof(acParseStr));
			memset(acKeyValue,0,sizeof(acKeyValue));
			sprintf(acKeyValue,"IPC%d",i+1);
			ReadParam(acIniPath, "[IPCINFO]", acKeyValue, acParseStr);
			if(0 == ParseIpcInfo(acParseStr, &g_atIpcInfo[i],acUser,acPassword))
			{
				int iNvrNo = g_atIpcInfo[i].iNvrNO;
				int iNum = g_atNVRState[(iNvrNo/2)*2].iVideoNum;
				
				g_atNVRState[(iNvrNo/2)*2].acVideoIdx[iNum] = i;    //互为冗余
				g_atNVRState[(iNvrNo/2)*2+1].acVideoIdx[iNum] = i;
				iNum++;
				g_atNVRState[(iNvrNo/2)*2].iVideoNum = iNum;
				g_atNVRState[(iNvrNo/2)*2+1].iVideoNum = iNum;
			}
			else
			{
			    printf("%d %s i= %d\n",__LINE__,__FUNCTION__,i);
				return -1;
			}
		}
	}	
	return 0;
}

static int ParseVideoRele()
{
	int iRet = ReadNvrAndIpc();
	if(0 == iRet )
	{
		iRet = ReadWarnRelate();
	}
	return iRet;
}

int ReadBrodCastInfo()
{
	int iRet =0;
	char acParseStr[256] = {0};
	char acKeyValue[24] = {0};
	int iValue = 0;
	char acIniPath[256] ={0};
	
	snprintf(acIniPath,sizeof(acIniPath)-1,"%s/%s",g_acConfigFileDir,g_acBroadcastConfFile);
	memset(acParseStr,0,sizeof(acParseStr));
	memset(acKeyValue,0,sizeof(acKeyValue));
	sprintf(acKeyValue,"DriverMonitorFlag");
	iRet = ReadParam(acIniPath, "[BroadcastInfo]", acKeyValue, acParseStr);
	iValue = 1;
	if(iRet >0)
	{
		iValue = atoi(acParseStr);
		if((0 == iValue) || (1 == iValue))
		{
			g_iMoniEnable = iValue;
		}
	}
	SetDriverRoomMonitorFlag(iValue);	
	
	memset(acParseStr,0,sizeof(acParseStr));
	memset(acKeyValue,0,sizeof(acKeyValue));
	sprintf(acKeyValue,"NoiseMonitorFlag");
	iRet = ReadParam(acIniPath, "[BroadcastInfo]", acKeyValue, acParseStr);
	iValue = 1;
	if(iRet >0)
	{
		iValue = atoi(acParseStr);
		if((0 == iValue) || (1 == iValue))
		{
			g_iNoiseEnable = iValue;
		}
	}
	SetNoiseMonitorFlag(iValue);
	
	memset(acParseStr,0,sizeof(acParseStr));
	memset(acKeyValue,0,sizeof(acKeyValue));
	sprintf(acKeyValue,"LcdVolume");
	iRet = ReadParam(acIniPath, "[BroadcastInfo]", acKeyValue, acParseStr);
	iValue = 60;
	if(iRet >0)
	{
		iValue = atoi(acParseStr);
		if((iValue >=0) && (iValue <=100))
		{
			g_iLcdVolum = iValue;
		}
	}
	SetLcdVolumeValue(iValue);
	
	memset(acParseStr,0,sizeof(acParseStr));
	memset(acKeyValue,0,sizeof(acKeyValue));
	sprintf(acKeyValue,"DriverRoomVolume");
	iRet = ReadParam(acIniPath, "[BroadcastInfo]", acKeyValue, acParseStr);
	iValue = 3;
	if(iRet >0)
	{
		iValue = atoi(acParseStr);
		if(iValue >=0 && iValue <=5)
		{
			g_iDriverRoonSpeakerVol = iValue;
		}	
	}
	SetDriverRoomSpeakerVolume(iValue);
	
	memset(acParseStr,0,sizeof(acParseStr));
	memset(acKeyValue,0,sizeof(acKeyValue));
	sprintf(acKeyValue,"CarriageVolume");
	iRet = ReadParam(acIniPath, "[BroadcastInfo]", acKeyValue, acParseStr);
	iValue = 5;
	if(iRet >0)
	{
		iValue = atoi(acParseStr);
		if(iValue >=0 && iValue <=10)
		{
			g_iCarriageSpeakVolum = iValue;
		}
	}
	SetCarriageSpeakerVolume(iValue);
	
	memset(acParseStr,0,sizeof(acParseStr));
	memset(acKeyValue,0,sizeof(acKeyValue));
	sprintf(acKeyValue,"DynamicMapBright");
	iRet = ReadParam(acIniPath, "[BroadcastInfo]", acKeyValue, acParseStr);
	iValue = 60;
	if(iRet >0)
	{
		iValue = atoi(acParseStr);
		if(iValue >=0 && iValue <= 100)
		{
			g_iDynamicMapBrightness = iValue;
		}
	}
	SetDynamicMapBrightness(iValue);
	return 0;
}

int STATE_Init(void)
{
	int i=0;
	int iRet = 0;
	
	g_eFileDownState = E_FILE_DOWN_IDLE;
	g_iVideoNum = 0;	
	g_iMoniEnable = -1;
	g_iNoiseEnable = -1;
	g_iLcdVolum = -1;
	g_iDriverRoonSpeakerVol = -1;
	g_iCarriageSpeakVolum = -1;
	g_iDynamicMapBrightness = -1;
	g_iVideoSource = 0x13;
	g_iPecuWarnValue = 0;
	g_iPecuFirstWarnVideoIdx = -1;
	g_cResUpdateState = DISP_STATE_UNKOWN;
	g_cResUpdateProgress = 0;
	g_iASCUMasSlaFlag = 0;
	if(1 == GetDeviceCarriageNo())
	{
		g_iASCUMasSlaFlag = 1;
	}
	memset(g_atCarriageWarn,0,sizeof(g_atCarriageWarn));
	memset(g_atNVRState,0,sizeof(g_atNVRState));
	for(i=0;i<6;i++)
	{
		g_atNVRState[i].iNVRConnectState = E_SERV_STATUS_UNCONNECT;
	}
	memset(g_acPecuVideoIdx,0xff,sizeof(g_acPecuVideoIdx));  //g_atIpcInfo从0开始的 所以避开0 
	memset(g_acFireVideoIdx,0xff,sizeof(g_acFireVideoIdx));
	memset(g_acDoorVideoIdx,0xff,sizeof(g_acDoorVideoIdx));
	for(i=0;i<VIDEO_CH_MAX_SIZE;i++)
	{	
	   g_atIpcInfo[i].iNvrNO = -1;
	   g_atIpcInfo[i].iUiGroup = -1;
	   g_atIpcInfo[i].iUiPos = -1;
	   g_atIpcInfo[i].iImgIndex = -1;
	   g_atIpcInfo[i].iOnlineState = 0;
	   g_atIpcInfo[i].iWarnState = 0;
	   memset(g_atIpcInfo[i].acRtspAddr,0,sizeof(g_atIpcInfo[i].acRtspAddr));
       memset(g_atIpcInfo[i].acMainRtspAddr,0,sizeof(g_atIpcInfo[i].acMainRtspAddr));
	}
	iRet = ReadBrodCastInfo();
	if(0 == iRet)
	{
		iRet = ParseVideoRele();
        
	}
	return iRet;
}

int STATE_Uninit(void)
{
    return 0;
}

int GetDeviceIp(char *pcIp,int ilen)
{
    if(0 == g_acIpAddr[0])
    {
        int inet_sock;  
        struct ifreq ifr;  
        inet_sock = socket(AF_INET, SOCK_DGRAM, 0);  
        strcpy(ifr.ifr_name, "eth1");
        ioctl(inet_sock, SIOCGIFADDR, &ifr);  
        strncpy(pcIp, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr),ilen);
	    close(inet_sock);
        strncpy(g_acIpAddr,pcIp,16);
    }
    else
    {
        strncpy(pcIp,g_acIpAddr,strlen(g_acIpAddr));
    }
	
	return 0;
}

int GetDeviceCarriageNo()
{
	char acIp[32]={0};  
	int iPart1,iPart2,iPart3,iPart4;
	GetDeviceIp(acIp,16);
	sscanf(acIp,"%d.%d.%d.%d",&iPart1,&iPart2,&iPart3,&iPart4); 
    if(21 == iPart3)
    {
    	return 1;
    }else
    {
    	return 6;
    }
}


//设置CCTV的运行目录
int SetCCTVRunDir(const char *pcData,int iLen)
{
	memset(g_acCCTVRunDir,0,sizeof(g_acCCTVRunDir));
	strncpy(g_acCCTVRunDir,pcData,iLen);
	return 0;
}
//获取CCTV的运行目录
int GetCCTVRunDir(char *pcRunData,int iLen)
{
	strncpy(pcRunData,g_acCCTVRunDir,iLen);
	return 0;
}


int GetVideoNum()
{
	return g_iVideoNum;
}


PMSG_HANDLE STATE_GetNvrServerPmsgHandle(int iServerIdex)
{
    if (iServerIdex > MAX_SERVER_NUM || iServerIdex < 0)
    {
        return 0;
    }

    return nvrServerPmsgHandle[iServerIdex];
}

int STATE_SetNvrServerPmsgHandle(int iServerIdex, PMSG_HANDLE pmsgHandle)
{
    if (pmsgHandle <= 0 || iServerIdex > MAX_SERVER_NUM || iServerIdex < 0)
    {
        return -1;
    }

    nvrServerPmsgHandle[iServerIdex] = pmsgHandle;
    return 0;
}



int SetVideoRtspUrl(int iVideoIdx, char *pcUrl,int iUrlLen)
{
    if ((iVideoIdx < 0) || (iVideoIdx >= VIDEO_CH_MAX_SIZE))
    {
         return -1;
    }
	if(iUrlLen > sizeof(g_atIpcInfo[iVideoIdx].acRtspAddr))
	{
		iUrlLen = sizeof(g_atIpcInfo[iVideoIdx].acRtspAddr);
	}
    strncpy(g_atIpcInfo[iVideoIdx].acRtspAddr, pcUrl, iUrlLen);
    return 0;
}

int GetVideoRtspUrl(int iVideoIdx, char *pcUrl, int iUrlLen)
{
    if ((iVideoIdx < 0) || (iVideoIdx >= g_iVideoNum) || 0 == g_atIpcInfo[iVideoIdx].acRtspAddr[0])
    {
         return -1;
    }
	if(iUrlLen > sizeof(g_atIpcInfo[iVideoIdx].acRtspAddr))
	{
		iUrlLen = sizeof(g_atIpcInfo[iVideoIdx].acRtspAddr);
	}

	strncpy(pcUrl, g_atIpcInfo[iVideoIdx].acRtspAddr, iUrlLen);
    return 0;
}

int GetVideoMainRtspUrl(int iVideoIdx, char *pcUrl, int iUrlLen)
{
    if ((iVideoIdx < 0) || (iVideoIdx >= g_iVideoNum) )
    {
         return -1;
    }

    if(g_atIpcInfo[iVideoIdx].acMainRtspAddr[0])
    {
        if(iUrlLen > sizeof(g_atIpcInfo[iVideoIdx].acMainRtspAddr))
	    {
		    iUrlLen = sizeof(g_atIpcInfo[iVideoIdx].acMainRtspAddr);
	    }
        strncpy(pcUrl, g_atIpcInfo[iVideoIdx].acMainRtspAddr, iUrlLen);
        
        return 0;
    }
    else if(g_atIpcInfo[iVideoIdx].acRtspAddr[0])
    {
        if(iUrlLen > sizeof(g_atIpcInfo[iVideoIdx].acRtspAddr))
	    {
		    iUrlLen = sizeof(g_atIpcInfo[iVideoIdx].acRtspAddr);
	    }
        strncpy(pcUrl, g_atIpcInfo[iVideoIdx].acRtspAddr, iUrlLen);
        
        return 0;
    }
   
    return -1;
}


int SetVideoOnlineState(int iIndex, int iState)
{
    if ((iIndex < 0) || (iIndex >= VIDEO_CH_MAX_SIZE))
     {
         return -1;
     }
     g_atIpcInfo[iIndex].iOnlineState = iState;
     return 0;
}

int GetVideoOnlineState(int iIndex)
{
    if ((iIndex < 0) || (iIndex >= g_iVideoNum))
     {
         return -1;
     }
     
     return g_atIpcInfo[iIndex].iOnlineState;
}

int GetVideoIdxAccordBtnPose(int iGroup,int iPos)
{
	int i=0;
	if ((iGroup < 0) || (iGroup > 7) || iPos <0 || iPos >3)
     {
         return -1;
     }
	for(;i<g_iVideoNum;i++)
	{
		if(g_atIpcInfo[i].iUiGroup == iGroup && g_atIpcInfo[i].iUiPos == iPos)
		{
			return i;
		}
	}
	 return -1;
}

int GetBtnPoseAccordVideoIdx(int iIndex,int *pGroup,int *iPos)
{
	if ((iIndex < 0) || (iIndex >= g_iVideoNum) )
    {
    	*pGroup = -1;
		*iPos = -1;
        return -1;
    }
	*pGroup = g_atIpcInfo[iIndex].iUiGroup;
	*iPos = g_atIpcInfo[iIndex].iUiPos;
	return 0;
}

//获取Nvr相关的相机数量
int GetNvrVideoNum(int iNvrNo)
{
	if (iNvrNo < 0 || iNvrNo > 5)
    {
         return -1;
    }
	
	return g_atNVRState[iNvrNo].iVideoNum;
}


int GetNvrVideoIdx(int iNvrNo,int iVideoNo)
{
	int iNum = g_atNVRState[iNvrNo].iVideoNum;
	if ((iNvrNo < 0) || (iNvrNo > 5) || iVideoNo <0 || iVideoNo >=iNum)
    {
        return -1;
    }
	return g_atNVRState[iNvrNo].acVideoIdx[iVideoNo];
	return 0;
}


int GetVideoName(int iVideoIdx,char *pName,int iLen)
{
	int iImgIndex = 0;
	if (iVideoIdx < 0 || iVideoIdx >= g_iVideoNum)
    {
         return -1;
    }
	iImgIndex = g_atIpcInfo[iVideoIdx].iImgIndex;
	return FindVideoNameAccordImgIdx(iImgIndex, pName, iLen);
}


int SetVideoWarnState(int iVideoIdx, int iState)
{

	if ((iVideoIdx < 0) || (iVideoIdx >= VIDEO_CH_MAX_SIZE))
    {
        return -1;
    }
	g_atIpcInfo[iVideoIdx].iWarnState = iState;
	return 0;
}


//返回值 -1:失败      0:正常 1 遮挡报警
int GetVideoWarnState(int iIndex)
{

	if ((iIndex < 0) || (iIndex >= g_iVideoNum))
     {
         return -1;
     }
	return g_atIpcInfo[iIndex].iWarnState;
}


int GetVideoImgIdx(int iVideoIdx)
{
	if ((iVideoIdx < 0) || (iVideoIdx >= g_iVideoNum))
    {
        return -1;
    }
	return g_atIpcInfo[iVideoIdx].iImgIndex;
}

int GetVideoNvrNo(int iVideoIdx)
{
	if ((iVideoIdx < 0) || (iVideoIdx >= g_iVideoNum))
    {
        return -1;
    }
	return g_atIpcInfo[iVideoIdx].iNvrNO;
}

int SetNvrIpAddr(int iNvrNo,char *pIp)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return -1;


	}
	strncpy(g_atNVRState[iNvrNo].acIp,pIp,sizeof(g_atNVRState[iNvrNo].acIp));
	return 0;
}

int GetNvrIpAddr(int iNvrNo,char *pIp)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return -1;


	}
	strncpy(pIp,g_atNVRState[iNvrNo].acIp,sizeof(g_atNVRState[iNvrNo].acIp));
	return 0;
}

int GetAllNvrInfo(T_NVR_STATE *ptData,int iLen)
{
	if(iLen < sizeof(g_atNVRState))
	{
		return -1;
	}
	memcpy(ptData,g_atNVRState,sizeof(g_atNVRState));
	return 0;
}


int SetNvrDiskState(int iNvrNo,T_DISK_STATE *ptDiskState)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return -1;


	}
	T_LOG_INFO tLog;
	
	tLog.iLogType = LOG_TYPE_SYS;
	
	if(g_atNVRState[iNvrNo].tDiskState.cDiskNum != ptDiskState->cDiskNum)
	{
		memset(tLog.acLogDesc,0,sizeof(tLog.acLogDesc));
		sprintf(tLog.acLogDesc,"disk num from %d to %d",g_atNVRState[iNvrNo].tDiskState.cDiskNum
			,ptDiskState->cDiskNum);
		LOG_WriteLog(&tLog);
	}
	if(g_atNVRState[iNvrNo].tDiskState.cDiskLostWarn != ptDiskState->cDiskLostWarn)
	{
		memset(tLog.acLogDesc,0,sizeof(tLog.acLogDesc));
		sprintf(tLog.acLogDesc,"disk lost warn");
		LOG_WriteLog(&tLog);
	}
	if(g_atNVRState[iNvrNo].tDiskState.cDiskSizeWarn != ptDiskState->cDiskSizeWarn)
	{
		memset(tLog.acLogDesc,0,sizeof(tLog.acLogDesc));
		sprintf(tLog.acLogDesc,"disk size warn");
		LOG_WriteLog(&tLog);
	}
	g_atNVRState[iNvrNo].tDiskState =*ptDiskState ;
	int iValue = g_atNVRState[iNvrNo].tDiskState.sDiskTotalSize;
	int iChange = ((iValue&0xff)<<8)|((iValue&0xff00)>>8);
	g_atNVRState[iNvrNo].tDiskState.sDiskTotalSize = iChange;
	iValue = g_atNVRState[iNvrNo].tDiskState.sDiskUseSize;
	iChange = ((iValue&0xff)<<8)|((iValue&0xff00)>>8);
	g_atNVRState[iNvrNo].tDiskState.sDiskUseSize = iChange;
	return 0;
}

int GetNvrDiskState(int iNvrNo,T_DISK_STATE *ptDiskState)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return -1;
	}
	*ptDiskState = g_atNVRState[iNvrNo].tDiskState;
	return 0;
}

int GetNvrDiskNum(int iNvrNo)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return 0;


	}
	return g_atNVRState[iNvrNo].tDiskState.cDiskNum ;
}

int GetNvrDiskTotalSize(int iNvrNo)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return 0;


	}
	return g_atNVRState[iNvrNo].tDiskState.sDiskTotalSize ;
}

int GetNvrDiskUsedSize(int iNvrNo)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return 0;


	}
	return g_atNVRState[iNvrNo].tDiskState.sDiskUseSize ;
}

int GetNvrDiskWarnState(int iNvrNo)
{
	if(iNvrNo<0 || iNvrNo >5)
	{
		return -1;


	}
	if(g_atNVRState[iNvrNo].tDiskState.cDiskLostWarn)
	{
		return 1;
	}
	if(g_atNVRState[iNvrNo].tDiskState.cDiskSizeWarn)
	{
		return 2;
	}
	return  0;
}

int SetFileSearchState(E_FILE_SEARCH_STATE iState)
{
	g_eFileSearch = iState;
	return 0;
}


 E_FILE_SEARCH_STATE GetFileSearchState()
{
	return g_eFileSearch;
}


int SetFileDownState(E_FILE_DOWN_STATE eState)  //0 空闲  1 正在进行  2 已完成
{
	g_eFileDownState = eState;
	return 0;
}
E_FILE_DOWN_STATE GetFileDownState( )
{
	return g_eFileDownState;
}

int SetFileDownProgress(int iProgress)
{
	g_iFileProgress = iProgress;
	return 0;
}
int GetFileDownProgress()
{
	return g_iFileProgress;
}


int STATE_FindUsbDev()
{
	FILE *pFile = 0;
    char acBuf[256] = {0};

	pFile = fopen("/proc/partitions", "rb");
    if (NULL == pFile)
    {
        return 0;
    }

    while (fgets(acBuf, sizeof(acBuf), pFile))
    {
        if (strstr(acBuf, "sd") != NULL)
        {
			fclose(pFile);
        	return 1;
        }
    }

    fclose(pFile);
    return 0;
}



int SetASCUMasterSlaveFlag(int iFlag)
{
	g_iASCUMasSlaFlag = iFlag;
	return 0;
}
int GetASCUMasterSlaveFlag()
{
	return g_iASCUMasSlaFlag;
}


int SetDriverRoomMonitorFlag(int iFlag)
{
	if(g_iMoniEnable != iFlag)
	{
		char acParseStr[8] = {0};
		char acIniPath[256] = {0};

		sprintf(acIniPath,"%s/%s",g_acConfigFileDir,g_acBroadcastConfFile);
		sprintf(acParseStr,"%d",iFlag);
		ModifyParam(acIniPath, "[BroadcastInfo]", "DriverMonitorFlag", acParseStr);
		g_iMoniEnable = iFlag;
	}
	return 0;
}
int GetDriverRoomMonitorFlag()
{
	return g_iMoniEnable;
}

int SetNoiseMonitorFlag(int iFlag)
{
	if(iFlag != g_iNoiseEnable)
	{
		char acParseStr[8] = {0};
		char acIniPath[256] = {0};

		sprintf(acIniPath,"%s/%s",g_acConfigFileDir,g_acBroadcastConfFile);
		sprintf(acParseStr,"%d",iFlag);
		ModifyParam(acIniPath, "[BroadcastInfo]", "NoiseMonitorFlag", acParseStr);
		g_iNoiseEnable = iFlag;
	}
	return 0;
}
int GetNoiseMonitorFlag()
{
	return g_iNoiseEnable;
}

int SetLcdVolumeValue(int iValue)
{
	if(g_iLcdVolum != iValue)
	{
		char acParseStr[8] = {0};
		char acIniPath[256] = {0};

		sprintf(acIniPath,"%s/%s",g_acConfigFileDir,g_acBroadcastConfFile);
		sprintf(acParseStr,"%d",iValue);
		ModifyParam(acIniPath, "[BroadcastInfo]", "LcdVolume", acParseStr);
		g_iLcdVolum = iValue;
	}
	return 0;
}

int GetLcdVolumeValue()
{
	return g_iLcdVolum;
}

int SetCarriageSpeakerVolume(int iValue)
{
	if(g_iCarriageSpeakVolum != iValue)
	{
		char acParseStr[8] = {0};
		char acIniPath[256] = {0};

		sprintf(acIniPath,"%s/%s",g_acConfigFileDir,g_acBroadcastConfFile);
		sprintf(acParseStr,"%d",iValue);
	
		ModifyParam(acIniPath, "[BroadcastInfo]", "CarriageVolume", acParseStr);
		g_iCarriageSpeakVolum = iValue;
	}
	return 0;
}

int GetCarriageSpeakerVolume()
{
	return g_iCarriageSpeakVolum;
}

int SetDriverRoomSpeakerVolume(int iValue)
{
	if(g_iDriverRoonSpeakerVol != iValue)
	{
		char acParseStr[8] = {0};
		char acIniPath[256] = {0};

		sprintf(acIniPath,"%s/%s",g_acConfigFileDir,g_acBroadcastConfFile);
		sprintf(acParseStr,"%d",iValue);
		ModifyParam(acIniPath, "[BroadcastInfo]", "DriverRoomVolume", acParseStr);
		g_iDriverRoonSpeakerVol = iValue;
	}
	return 0;
}

int GetDriverRoomSpeakerVolume()
{
	return g_iDriverRoonSpeakerVol;
}

int SetDynamicMapBrightness(int iValue)
{
	if(g_iDynamicMapBrightness != iValue)
	{
		char acParseStr[8] = {0};
		char acIniPath[256] = {0};

		sprintf(acIniPath,"%s/%s",g_acConfigFileDir,g_acBroadcastConfFile);
		sprintf(acParseStr,"%d",iValue);
		ModifyParam(acIniPath, "[BroadcastInfo]", "DynamicMapBright", acParseStr);
		g_iDynamicMapBrightness = iValue;
	}	
	return 0;
}

int GetDynamicMapBrightness()
{
	return g_iDynamicMapBrightness;
}


int SetVideoSource(int iVideoType)
{
	g_iVideoSource = iVideoType;
	return 0;
}
int GetVideoSource()
{
	return g_iVideoSource;
}


int GetPecuVideoIndex(int iNo)
{
	int iVideoIdx = -1;
	if(iNo <0 || iNo >23)
	{
		return -1;
	}
	iVideoIdx = g_acPecuVideoIdx[iNo];
	if(iVideoIdx <0 || iVideoIdx >g_iVideoNum)
	{
		return -1;
	}
	return iVideoIdx;
}
int SetPecuWarnInfo(u32int iValue)
{
	g_iPecuWarnValue = iValue;
	return 0;
}
u32int GetPecuWarnInfo()
{
	return g_iPecuWarnValue;
}

int SetPecuFirstWarnVideoIdx(int iIndex)
{
	g_iPecuFirstWarnVideoIdx = iIndex;
	return 0;
}

int GetPecuFirstWarnVideoIdx()
{
	return g_iPecuFirstWarnVideoIdx;
}

int GetFireWarnVideoIdx(char iCarriageNo)
{
	int iVideoIdx = g_acFireVideoIdx[iCarriageNo];
	if(iCarriageNo <0 || iCarriageNo >5)
	{
		return -1;
	}
	if(iVideoIdx <0 || iVideoIdx >g_iVideoNum)
	{
		return -1;
	}
	return iVideoIdx;
}

int SetFireWarnInfo(char iCarriageNo,u8int cWarnState)
{
	if(iCarriageNo <0 || iCarriageNo >5)
	{
		return -1;
	}
	g_atCarriageWarn[iCarriageNo].cFireWarn = cWarnState;
	return 0;
}

int	GetFireWarnInfo(char iCarriageNo,u8int *pcWarnState)
{
	if(iCarriageNo <0 || iCarriageNo >5)
	{
		return -1;
	}
	*pcWarnState = g_atCarriageWarn[iCarriageNo].cFireWarn;
	return 0;
}


int GetAllFireWarnInfo(char *pFireWarn,int iDataLen)
{
	if(iDataLen <6)
	{
		return -1;
	}
	memset(pFireWarn,0,iDataLen);
	pFireWarn[0] = g_atCarriageWarn[0].cFireWarn;
	pFireWarn[1] = g_atCarriageWarn[1].cFireWarn;
	pFireWarn[2] = g_atCarriageWarn[2].cFireWarn;
	pFireWarn[3] = g_atCarriageWarn[3].cFireWarn;
	pFireWarn[4] = g_atCarriageWarn[4].cFireWarn;
	pFireWarn[5] = g_atCarriageWarn[5].cFireWarn;
	return 0;
}


int SetAllDoorClipInfo(char *pDoorClipWarn,int iDataLen)
{
	if(iDataLen <6)
	{
		return -1;
	}
	g_atCarriageWarn[0].cDoorClipWarn = pDoorClipWarn[0];
    g_atCarriageWarn[1].cDoorClipWarn = pDoorClipWarn[1];
    g_atCarriageWarn[2].cDoorClipWarn = pDoorClipWarn[2];
    g_atCarriageWarn[3].cDoorClipWarn = pDoorClipWarn[3];
    g_atCarriageWarn[4].cDoorClipWarn = pDoorClipWarn[4];
    g_atCarriageWarn[5].cDoorClipWarn = pDoorClipWarn[5];
    
	return 0;
}


int GetAllDoorClipInfo(char *pDoorClipWarn,int iDataLen)
{
	if(iDataLen <6)
	{
		return -1;
	}
    memset(pDoorClipWarn,0,iDataLen);    
	pDoorClipWarn[0] = g_atCarriageWarn[0].cDoorClipWarn ;
    pDoorClipWarn[1] = g_atCarriageWarn[1].cDoorClipWarn ;
    pDoorClipWarn[2] = g_atCarriageWarn[2].cDoorClipWarn ;
    pDoorClipWarn[3] = g_atCarriageWarn[3].cDoorClipWarn ;
    pDoorClipWarn[4] = g_atCarriageWarn[4].cDoorClipWarn ;
    pDoorClipWarn[5] = g_atCarriageWarn[5].cDoorClipWarn ;
    
	return 0;
}


int GetDoorWarnVideoIdx(char iCarriageNo,int iNo)
{
	int iVideoIdx =  -1;
	if(iCarriageNo <0 || iCarriageNo >5 || iNo <0 || iNo >7)
	{
		return -1;
	}
	iVideoIdx = g_acDoorVideoIdx[iCarriageNo*8 + iNo];
	if(iVideoIdx <0 || iVideoIdx >g_iVideoNum)
	{
		return -1;
	}
	return iVideoIdx;
}

int SetDoorWarnInfo(char iCarriageNo,u8int cWarnState)
{
	if(iCarriageNo <0 || iCarriageNo >5)
	{
		return -1;
	}
	g_atCarriageWarn[iCarriageNo].cDoorWarn = cWarnState;
	return 0;
}
int GetDoorWarnInfo(char iCarriageNo,u8int *pcWarnState)
{
	if(iCarriageNo <0 || iCarriageNo >5)
	{
		return -1;
	}
	*pcWarnState = g_atCarriageWarn[iCarriageNo].cDoorWarn;
	return 0;
}

int GetAllDoorWarnInfo(char *pDoorWarn,int iDataLen)
{
	if(iDataLen <6)
	{
		return -1;
	}
	memset(pDoorWarn,0,iDataLen);
	pDoorWarn[0] = g_atCarriageWarn[0].cDoorWarn;
	pDoorWarn[1] = g_atCarriageWarn[1].cDoorWarn;
	pDoorWarn[2] = g_atCarriageWarn[2].cDoorWarn;
	pDoorWarn[3] = g_atCarriageWarn[3].cDoorWarn;
	pDoorWarn[4] = g_atCarriageWarn[4].cDoorWarn;
	pDoorWarn[5] = g_atCarriageWarn[5].cDoorWarn;
	return 0;
}

int SetResUpdateState(char cState,char cProgress)
{
	g_cResUpdateState = cState;
	g_cResUpdateProgress = cProgress;
	return 0;
}

int GetResUpdateState(char *pcState,char *pcProgress)
{
	*pcState = g_cResUpdateState;
	*pcProgress = g_cResUpdateProgress;
	return 0;
}


void SetDisplayState(int iState)
{
	g_iDispState = iState;
}

int GetDisplayState(void)
{
    return g_iDispState;	
}

int GetDevDispMode()
{
	char acParseStr[8] = {0};
	int iRet  = 0;
	int iMode = 1;
	
	iRet = ReadParam("/mnt/mmc/dhmi/displayconfig.ini", "[DisplayConfig]", "DisplayMode", acParseStr);
	if(iRet >0)
	{
		iMode = atoi(acParseStr);
		if(iMode <1 || iMode >3)
		{
			iMode = 1;
		}
	}
	return iMode;
}

int GetTestEnableFlg(char * pcFlg)
{
    char acValue[128];
    int  iVal = 0;

    memset(acValue, 0x0, 128);
    if (ReadParam("/mnt/confs/SysRunConf.ini", NULL,"TestDemoRunFlg", acValue)<0 )
    {
        printf("%s %d\n ReadParam err",__FUNCTION__,__LINE__);
        return -1;
    }

    iVal = atoi(acValue);

    if ( (0 == iVal) || (1 == iVal) )
    {
        *pcFlg = iVal;
    }
    else
    {
        *pcFlg = 0;
    }
    return 0;
}

int GetCycTime()
{
	int iRet = 0;
	char acIniPath[256] ={0};
    char acValue[24] = {0};
    int iValue = 30;
	
	snprintf(acIniPath,sizeof(acIniPath)-1,"%s/%s",g_acConfigFileDir,g_acCCTVConfigFile);
    iRet = ReadParam(acIniPath, "[CYCTIME]","time",acValue);

    if (iRet > 0)
	{
		iValue = atoi(acValue);
        if(iValue <=0)
        {
            iValue = 30;
        }
    }  
    return iValue;
}

int ExecSysCmd(char *cmd, char *result, int len)
{
    FILE *fp = popen(cmd, "r");
    if(fp)
    {
        if(result != NULL && len > 0)
        {
            memset(result, 0, len);
            int i = fread(result, 1, len-1, fp);
            if(i == 0)
            {
                pclose(fp);
                return -1;
            }
        }
        pclose(fp);
    }
    else
    {
        return -1;
    }
    return 0;
}

