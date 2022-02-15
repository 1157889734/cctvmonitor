
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <errno.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include "pmsgproc.h"
#include "multicast.h"
#include "state.h"
#include "log.h"
#include "debug.h"



int g_iBroadcastSysSocket = 0;
int g_iVideoSwitchSocket = 0;
int g_iDynamicMapSocket = 0;
int g_iHeartSockt       = 0;
int g_iLcdSocket = 0;

static pthread_t  g_tid;
static int g_iMultThreadRun = 0;
static int g_iHaveSendFirstVol = 0;

static const uint8_t crc8_table[256] = {
	0x00,0x31,0x62,0x53,0xc4,0xf5,0xa6,0x97,0xb9,0x88,0xdb,0xea,0x7d,0x4c,0x1f,0x2e,
	0x43,0x72,0x21,0x10,0x87,0xb6,0xe5,0xd4,0xfa,0xcb,0x98,0xa9,0x3e,0x0f,0x5c,0x6d,
	0x86,0xb7,0xe4,0xd5,0x42,0x73,0x20,0x11,0x3f,0x0e,0x5d,0x6c,0xfb,0xca,0x99,0xa8,
	0xc5,0xf4,0xa7,0x96,0x01,0x30,0x63,0x52,0x7c,0x4d,0x1e,0x2f,0xb8,0x89,0xda,0xeb,
	0x3d,0x0c,0x5f,0x6e,0xf9,0xc8,0x9b,0xaa,0x84,0xb5,0xe6,0xd7,0x40,0x71,0x22,0x13,
	0x7e,0x4f,0x1c,0x2d,0xba,0x8b,0xd8,0xe9,0xc7,0xf6,0xa5,0x94,0x03,0x32,0x61,0x50,
	0xbb,0x8a,0xd9,0xe8,0x7f,0x4e,0x1d,0x2c,0x02,0x33,0x60,0x51,0xc6,0xf7,0xa4,0x95,
	0xf8,0xc9,0x9a,0xab,0x3c,0x0d,0x5e,0x6f,0x41,0x70,0x23,0x12,0x85,0xb4,0xe7,0xd6,
	0x7a,0x4b,0x18,0x29,0xbe,0x8f,0xdc,0xed,0xc3,0xf2,0xa1,0x90,0x07,0x36,0x65,0x54,
	0x39,0x08,0x5b,0x6a,0xfd,0xcc,0x9f,0xae,0x80,0xb1,0xe2,0xd3,0x44,0x75,0x26,0x17,
	0xfc,0xcd,0x9e,0xaf,0x38,0x09,0x5a,0x6b,0x45,0x74,0x27,0x16,0x81,0xb0,0xe3,0xd2,
	0xbf,0x8e,0xdd,0xec,0x7b,0x4a,0x19,0x28,0x06,0x37,0x64,0x55,0xc2,0xf3,0xa0,0x91,
	0x47,0x76,0x25,0x14,0x83,0xb2,0xe1,0xd0,0xfe,0xcf,0x9c,0xad,0x3a,0x0b,0x58,0x69,
	0x04,0x35,0x66,0x57,0xc0,0xf1,0xa2,0x93,0xbd,0x8c,0xdf,0xee,0x79,0x48,0x1b,0x2a,
	0xc1,0xf0,0xa3,0x92,0x05,0x34,0x67,0x56,0x78,0x49,0x1a,0x2b,0xbc,0x8d,0xde,0xef,
	0x82,0xb3,0xe0,0xd1,0x46,0x77,0x24,0x15,0x3b,0x0a,0x59,0x68,0xff,0xce,0x9d,0xac
};

typedef struct {	
	uint8_t res1:2;
	uint8_t carfires:1; 
	//uint8_t res12:4;
	/*modify at 20180702:add door fangjia*/
	uint8_t zudang1_car:4;
	uint8_t carpower:1;
	uint8_t cardoors;
}__attribute__((packed)) T_FIRE_DOOR_UNIT;


typedef struct {	
	T_FIRE_DOOR_UNIT atFierDoor[6];	
	uint8_t zudang_car4;
	uint8_t zudang_car3;
	uint8_t zudang_car6;
	uint8_t zudang_car5;

}__attribute__((packed)) T_FIRE_DOOR_INFO;

typedef struct _T_tagBeaconMsg
{
    unsigned char head;/*固定为0xc0*/
    unsigned char heads; /*固定为0xf0*/
    unsigned char ip[16]; /*自身设备的IP，字符串形式：“10.1.41.4”*/
}__attribute__((packed)) T_tagBeaconMsg;


typedef struct{
	int8_t	resv;	 
	int8_t	AscuMaster;	//0x01:本ASCU是主端	0x00:本ASCU是从端
	int8_t	AudioType;
	int8_t	Key;		//司机室钥匙信号，0x00:无钥匙  0x01:有钥匙
	int8_t	EmergeId;
	int8_t	CarLocation;
	int16_t	StartId;
	int16_t	StopId;
	int16_t	NowId;
	int16_t	NextId;
	int8_t	DoorOpen;
	int8_t	RunDir;
	int8_t	ReportMode;
	int8_t	PACaller;
	int8_t	PisFlag;
	int8_t	CCCaller;
	int8_t	CCStatus;
}__attribute__((packed)) T_KEY_INFO;

uint8_t GetCRC8(uint8_t *pdata,uint16_t len)
{
	uint8_t  crc = 0x00;

	while (len--){
		crc = crc8_table[crc ^ *pdata++];
	}
	return crc;
}

static int CreateUdpSocket(unsigned short usPort)
{
    int iSockFd = 0; 
    int iRet = 0;
    int iFlag = 1;
    struct sockaddr_in	servaddr;
	
    
    iSockFd = socket(PF_INET, SOCK_DGRAM, 0);
    if (iSockFd < 0)
    {
        printf("Opening mutlsocket error\n");
        return -1;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family        = AF_INET;
    servaddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    servaddr.sin_port          = htons(usPort);
    
    iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (char*)&iFlag, sizeof(int));
    if(iRet != 0)
    {
        printf("ERR: setsockopt socket error. err = %d,errno = %d[%s]\n", iRet, errno, strerror(errno));
        close(iSockFd);
        return -1;
    }
    if (bind(iSockFd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind fail");
        close(iSockFd);
        return -1;
    }
    
    return iSockFd;
}

int ParseBroadcastSysData(char *pcBuf, int iLen)
{
    unsigned short usCmd = pcBuf[5]<<8 | pcBuf[4];
    unsigned char ucHead = pcBuf[0];
	unsigned char acBuf[1024] = {0};
    int iRelLen = pcBuf[7]<<8 | pcBuf[6];

    
    if (ucHead != 0xFA)
    {
        DebugPrint(DEBUG_PIS_WARN,"ParseBroadcastSysDatas:err!(ucHead != 0xFA)");
        return -1;	
    }

    if(iLen != iRelLen +10)
    {
        return -1;	
    }
    
    DebugPrint(DEBUG_PIS_WARN,"ParseBroadcastSysDatas:Len = %d,usCmd =%d",iLen,usCmd);
    if (0x500 == usCmd)  //PECU报警联动
    {
       int iPecu =0;
	   int iNextWarnIndex = -1;
	   int i=0;
	   
	   for(;i<iRelLen;i++)
	   {
	   		int iValue = pcBuf[9+i];  //PECU的最后1byte的IP地址，0表示无报警，无效 ip4:20-43
			int iVideoIndex = -1;
			if(iValue >19 &&iValue <44 )
			{
				iVideoIndex = GetPecuVideoIndex(iValue-20);
				iPecu |= (1 <<(iValue-20));
			}
			if(iVideoIndex>=0 && iNextWarnIndex <0)
			{
				iNextWarnIndex = iVideoIndex;
			}
	   }
	   if(iNextWarnIndex >=0)
	   {
	   		SetPecuFirstWarnVideoIdx(iNextWarnIndex);
	   }
	   else
	   {
	   		SetPecuFirstWarnVideoIdx(-1);   //没有报警	
	   }
	   SetPecuWarnInfo(iPecu);
       DebugPrint(DEBUG_PIS_WARN,"SetPecuWarnInfo:iPecu =%d",iPecu);
    }
    else if (0x1800 == usCmd)  //烟火报警、门紧急解锁报警联动
    {
       T_FIRE_DOOR_INFO *ptFireDoorInfo = (T_FIRE_DOOR_INFO *)&pcBuf[9];
	   int i =0;
       unsigned char acValue[12] = {0};
       char acDoorClip[6] = {0};
	   
	   for(;i<6;i++)
	   {
	   		SetFireWarnInfo(i,ptFireDoorInfo->atFierDoor[i].carfires);
			SetDoorWarnInfo(i,ptFireDoorInfo->atFierDoor[i].cardoors);
            acValue[i] = ptFireDoorInfo->atFierDoor[i].carfires;
            acValue[i+6] = ptFireDoorInfo->atFierDoor[i].cardoors;
            if(i<4)
            {
                 T_FIRE_DOOR_UNIT *pUnit = &(ptFireDoorInfo->atFierDoor[i]);
                 if(0 == i%2)
                 {
                    acDoorClip[i/2] = pUnit->zudang1_car;
                 }
                 else
                 {
                    acDoorClip[i/2] =((pUnit->zudang1_car<<4)&0xF0)|acDoorClip[i/2];
                 }  
            }
	   }

       acDoorClip[2] = ptFireDoorInfo->zudang_car3;
       acDoorClip[3] = ptFireDoorInfo->zudang_car4;
       acDoorClip[4] = ptFireDoorInfo->zudang_car5;
       acDoorClip[5] = ptFireDoorInfo->zudang_car6;

       SetAllDoorClipInfo(acDoorClip,sizeof(acDoorClip));
       
       DebugPrint(DEBUG_PIS_WARN,"Fire(0-5):%d %d %d %d %d %d" ,acValue[0]
       ,acValue[1],acValue[2],acValue[3],acValue[4],acValue[5]);
       DebugPrint(DEBUG_PIS_WARN,"door(0-5):%d %d %d %d %d %d" ,acValue[6]
       ,acValue[7],acValue[8],acValue[9],acValue[10],acValue[11]);
    }
	else if (0x300 == usCmd )  //钥匙端信号
	{
		
		T_KEY_INFO *ptKeyInfo = (T_KEY_INFO *)&pcBuf[9];
		int iValid = 0;

        //司机头接收192.168.1.4发送的钥匙端信号
        //司机尾介绍192.168.1.5发送的钥匙端信号
		if((1 == GetDeviceCarriageNo() && 4 == pcBuf[2])
			|| (6 == GetDeviceCarriageNo() && 5 == pcBuf[2]))
		{
			iValid = 1;
		}
		if(iValid)
		{
			if(0x1 == ptKeyInfo->AscuMaster)
			{
			    T_CARRIAGE_SPEAKER_VOL tSpeakerVol ;
                
				SetASCUMasterSlaveFlag(1);
                //上电后 默认将监控室音量设置为自动
                //噪检关掉
                if(0 == g_iHaveSendFirstVol)
                {
                    int j =0;
                    g_iHaveSendFirstVol = 1;
                    
                    tSpeakerVol.value_train = GetCarriageSpeakerVolume();
                    for(j=0;j<6;j++)
                    {
                        tSpeakerVol.value[j] = tSpeakerVol.value_train;
                    }
 
                    tSpeakerVol.ascu_mute[0] = 0;
                    tSpeakerVol.ascu_mute[1] = 0;
                    tSpeakerVol.value_monitor[0] = 0xff;
                    tSpeakerVol.value_monitor[1] = 0xff;
                    adjustCarriageSpeakerVolume(&tSpeakerVol);
                    StartNoiseMonitor(1);
                } 
			}
			else
			{
				SetASCUMasterSlaveFlag(0);
			}
            DebugPrint(DEBUG_PIS_WARN,"SetASCUMasterSlaveFlag:Flag = %d", ptKeyInfo->AscuMaster);
		}
        else
        {
           DebugPrint(DEBUG_PIS_WARN,"SetASCUMasterSlaveFlag:No = %d,pcBuf[2]= %d",
           GetDeviceCarriageNo(),pcBuf[2]);
		} 
	}
}

int ParseVideoSourceData(char *pcBuf, int iLen)
{
	unsigned char ucCmd = pcBuf[2];

    if (pcBuf[0] != 0xB0 || pcBuf[1] != 0xFB)
    {
        DebugPrint(DEBUG_PIS_SOURCE,"ParseVideoSourceData err");
        return -1;	
    }
    DebugPrint(DEBUG_PIS_SOURCE,"ParseVideoSourceData ucCmd =%d, ilen = %d, No = %d, pcBuf[5] = %x\
    , pcBuf[6] = %x",ucCmd,iLen,pcBuf[5],pcBuf[6]);
    if (0x81 == ucCmd)  //视频源设置回复参数
    {
    	
    }
	else if(0x82 == ucCmd)  //获取播放模式回复
	{
		unsigned char ucCarriageNo = GetDeviceCarriageNo();

        //第一个字节：设置源：
        //0x01 1车
        //0x02 6车
        //第二个字节表示回复参数：
        //如果当前播放模式为直播，返回0x21 
        //如果当前播放模式为数字电视，返回0x22
        //如果当前播放模式为本地播放，返回0x23
		if((6 == ucCarriageNo && 2 == pcBuf[5])
			|| (1 == ucCarriageNo && 1 == pcBuf[5]))
		{
			switch(pcBuf[6])
			{
				case 0x21:
					SetVideoSource(0x11);
					break;
				case 0x22:
					SetVideoSource(0x12);
					break;
				default:
					SetVideoSource(0x13);
			}
            return 1;
		}
	}
    return 0;
}

void *MultcastRecvThread(void *argv)
{
    fd_set	tAllSet, tTmpSet;
    struct timeval tv;
    int iMaxFd;
    struct sockaddr_in tSockAddr;
    int iSockAddrLen = sizeof(tSockAddr);
    int iRecvLen = 0;
    char acBuf[1024];
	        
    int iOldLcdVolumeValue = -1; 
    int iNewLcdVolumeValue = 0;
    int iLcdVolumeCount = 1;
    int iHeartCount = 0;
    T_CARRIAGE_SPEAKER_VOL tSpeakerVol;
        
    int iOldVideoSource = -1;
    int iNewVideoSource = 0;
	int iVideoSourceCount = 1;
	int iRet = 0;    
    int i = 0;
    int iSysRecvCount = 0;
    int iVideoRecvCount = 0;
    
    FD_ZERO(&tAllSet);
    FD_SET(g_iBroadcastSysSocket, &tAllSet);
    FD_SET(g_iVideoSwitchSocket, &tAllSet);
    iMaxFd = g_iBroadcastSysSocket > g_iVideoSwitchSocket?g_iBroadcastSysSocket:g_iVideoSwitchSocket;

    while (g_iMultThreadRun)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 1000;
        tTmpSet = tAllSet;	//重新置位.

        memset(acBuf,0,sizeof(acBuf));
        if (select(iMaxFd + 1, &tTmpSet, NULL, NULL, &tv) > 0)
        {
            if (FD_ISSET(g_iBroadcastSysSocket, &tTmpSet))
            {
                iRecvLen = recvfrom(g_iBroadcastSysSocket, acBuf, sizeof(acBuf), 0, 
                                   (struct sockaddr *)&tSockAddr, (socklen_t*)&iSockAddrLen);
                if (iRecvLen > 0)
                {
                   ParseBroadcastSysData(acBuf, iRecvLen);
                   iSysRecvCount = 0;
                }
            }
            else if (FD_ISSET(g_iVideoSwitchSocket, &tTmpSet))
            {
                iRecvLen = recvfrom(g_iVideoSwitchSocket, acBuf, sizeof(acBuf), 0, 
                                   (struct sockaddr *)&tSockAddr, (socklen_t*)&iSockAddrLen);
                iRet = ParseVideoSourceData(acBuf, iRecvLen);
                if(1 == iRet)
                {
                    iVideoRecvCount = 0;
                }
            }
        }
        
        iSysRecvCount++;
        iVideoRecvCount ++;

        if(iSysRecvCount >4000)
        {
            iSysRecvCount = 0;
            if (g_iBroadcastSysSocket > 0)
            {
                DropMulticastAddr(g_iBroadcastSysSocket, "225.0.0.10");
                close(g_iBroadcastSysSocket);	
            }
            g_iBroadcastSysSocket = CreateMultcastSocket(8888);
            AddMulticastAddr(g_iBroadcastSysSocket, "225.0.0.10");
            FD_ZERO(&tAllSet);
            FD_SET(g_iBroadcastSysSocket, &tAllSet);
            FD_SET(g_iVideoSwitchSocket, &tAllSet);
            iMaxFd = g_iBroadcastSysSocket > g_iVideoSwitchSocket?g_iBroadcastSysSocket:g_iVideoSwitchSocket;
        }

        if(iVideoRecvCount >6000)
        {
            iVideoRecvCount = 0;
            if (g_iVideoSwitchSocket > 0)
            {
                DropMulticastAddr(g_iVideoSwitchSocket, "224.0.0.88");
                close(g_iVideoSwitchSocket);	
            }
            g_iVideoSwitchSocket = CreateMultcastSocket(12080);
            AddMulticastAddr(g_iVideoSwitchSocket, "224.0.0.88");
            FD_ZERO(&tAllSet);
            FD_SET(g_iBroadcastSysSocket, &tAllSet);
            FD_SET(g_iVideoSwitchSocket, &tAllSet);
            iMaxFd = g_iBroadcastSysSocket > g_iVideoSwitchSocket?g_iBroadcastSysSocket:g_iVideoSwitchSocket;
        }
        
        iNewLcdVolumeValue = GetLcdVolumeValue();
        if (iNewLcdVolumeValue != iOldLcdVolumeValue)
        {
            iRet = AdjustLcdVolume(iNewLcdVolumeValue);
			if((iNewLcdVolumeValue != 1) && iRet >0 )
			{
				T_LOG_INFO tLog;
				
				memset(&tLog,0,sizeof(T_LOG_INFO));
				tLog.iLogType = LOG_TYPE_EVENT;	
				snprintf(tLog.acLogDesc,sizeof(tLog.acLogDesc)-1
					,"lcd volume changed from %d to %d"
					,iOldLcdVolumeValue,iNewLcdVolumeValue);
				LOG_WriteLog(&tLog);
			}	
            iOldLcdVolumeValue = iNewLcdVolumeValue;
            iLcdVolumeCount = 1;
        }
              
        if (iLcdVolumeCount % 4000 == 0)
        {
            AdjustLcdVolume(iNewLcdVolumeValue);
		    iLcdVolumeCount = 0;
        }
		if(0 == iVideoSourceCount %3000)
		{
			iVideoSourceCount = 0;
			GetVideoSrcMode();
		}

        if( 0 == iHeartCount%1200 )
        {
            SendHeartInfo();
            iHeartCount = 0;
        }
        
        iLcdVolumeCount++;
		iVideoSourceCount++;
        iHeartCount ++;
        //usleep(2000);
    }	
    
    return NULL;
}

int CreateLcdSocket()
{
    int iSockFd = 0;
    int iOptval = 1;
    

    iSockFd = socket(PF_INET, SOCK_DGRAM, 0);
    if (iSockFd < 0)
    {
        printf("Opening udpsocket error\n");
        return -1;
    }
    
   	if (setsockopt(iSockFd, SOL_SOCKET, SO_BROADCAST, &iOptval, sizeof(iOptval)) == -1) 
    {
        printf("Could not setsocketopt on raw socket\n");
        close(iSockFd);
        return -1;
    }
    return iSockFd;
}

int InitPmsgproc(void)
{  
    int iRet = 0;

    g_iBroadcastSysSocket = CreateMultcastSocket(8888);
    g_iVideoSwitchSocket = CreateMultcastSocket(12080);  
    g_iDynamicMapSocket = CreateMultcastSocket(12306);
    g_iHeartSockt = CreateMultcastSocket(12090);
    g_iLcdSocket = CreateLcdSocket();
    iRet = AddMulticastAddr(g_iBroadcastSysSocket, "225.0.0.10");
    iRet = AddMulticastAddr(g_iVideoSwitchSocket, "224.0.0.88");
    g_iMultThreadRun = 1;
    pthread_create(&g_tid, NULL, MultcastRecvThread, NULL);
    
    return 0;
}


int UninitPmsgproc(void)
{
	g_iMultThreadRun = 0;
	pthread_join(g_tid, NULL);
	
    if (g_iBroadcastSysSocket > 0)
    {
        close(g_iBroadcastSysSocket);	
    }	
    
    if (g_iVideoSwitchSocket > 0)
    {
        close(g_iVideoSwitchSocket);	
    }
    
    if (g_iDynamicMapSocket > 0)
    {
        close(g_iDynamicMapSocket);	
    }

    if (g_iLcdSocket > 0)
    {
        close(g_iLcdSocket);	
    }
    
    return 0;
}

//广播系统 IP(225.0.0.10)及发送端口（8888）

int StartNoiseMonitor(char cNoiseMonitorFlag)
{
	int iRet = 0;

	if(GetASCUMasterSlaveFlag())
	{
    	char acMultAddr[] = "225.0.0.10";
    	unsigned short usMultPort = 8888;
    	char acBuf[24];
    	short sLen = 0;   
        T_LOG_INFO tLog;
				
		memset(&tLog,0,sizeof(T_LOG_INFO));
	    tLog.iLogType = LOG_TYPE_EVENT;	
		snprintf(tLog.acLogDesc,sizeof(tLog.acLogDesc)-1
				,"set noise flag  %d",cNoiseMonitorFlag );
		LOG_WriteLog(&tLog);
        
    	acBuf[0] = 0xFA;  // STX
    	acBuf[1] = 0x00;  // VER
    	acBuf[2] = 0x10;  // SrcAddr
    	acBuf[3] = 0xFF;  // DstAddr
    	acBuf[4] = 0x01;   // 功能码0x0701(小端模式)
    	acBuf[5] = 0x07;   // 功能码
    	sLen = 1;
    	acBuf[6] = (sLen & 0xFF);
    	acBuf[7] = (sLen >> 8) & 0xFF;
   
    	acBuf[8] = GetCRC8(&cNoiseMonitorFlag, 1);
    	acBuf[9] = cNoiseMonitorFlag;
    	acBuf[10] = 0xFC;
		iRet = SendMultcastPacket(g_iBroadcastSysSocket, acMultAddr, usMultPort, acBuf, 11);
	}
    return iRet;
}


//调节客室和司机室音量
int adjustCarriageSpeakerVolume(T_CARRIAGE_SPEAKER_VOL *ptCarriageSpeakerVol)
{
	int iRet = 0;
	
	if(GetASCUMasterSlaveFlag())
	{
    	char acMultAddr[] = "225.0.0.10";
    	unsigned short usMultPort = 8888;
    	char acBuf[128];
    	short sLen = 0;
    	char cSum;
        T_LOG_INFO tLog;
				
		memset(&tLog,0,sizeof(T_LOG_INFO));
	    tLog.iLogType = LOG_TYPE_EVENT;	
		snprintf(tLog.acLogDesc,sizeof(tLog.acLogDesc)-1
				,"monitor vol:%d train vol:%d",ptCarriageSpeakerVol->value_monitor[0],
				ptCarriageSpeakerVol->value_train);
		LOG_WriteLog(&tLog);
    
    	acBuf[0] = 0xFA;  // STX
    	acBuf[1] = 0x00;  // VER
    	acBuf[2] = 0x10;  // SrcAddr
    	acBuf[3] = 0xFF;  // DstAddr
    	acBuf[4] = 0x02;   // 功能码0x0702(小端模式)
    	acBuf[5] = 0x07;   // 功能码
    	sLen = sizeof(T_CARRIAGE_SPEAKER_VOL);
    	acBuf[6] = (sLen & 0xFF);
    	acBuf[7] = (sLen >> 8) & 0xFF;
    
    	cSum = GetCRC8((char *)ptCarriageSpeakerVol, sizeof(T_CARRIAGE_SPEAKER_VOL));
    	acBuf[8] = cSum;
    
    	memcpy(&acBuf[9], (char *)ptCarriageSpeakerVol, sizeof(T_CARRIAGE_SPEAKER_VOL));
    	acBuf[9 + sizeof(T_CARRIAGE_SPEAKER_VOL)] = 0xFC;
	  	iRet = SendMultcastPacket(g_iBroadcastSysSocket, acMultAddr, usMultPort, acBuf, sizeof(T_CARRIAGE_SPEAKER_VOL) + 10);
	}
	return iRet;
}

// 影视系统 IP(224.0.0.88)及发送端口（12080）
//视频源切换
int SwitchVideoSrc(char cSrc, char cType)
{
	int iRet = 0;
	
	if(GetASCUMasterSlaveFlag())
	{
    	char acMultAddr[] = "224.0.0.88";
    	unsigned short usMultPort = 12080;
    	char acBuf[8];
        T_LOG_INFO tLog;
				
		memset(&tLog,0,sizeof(T_LOG_INFO));
	    tLog.iLogType = LOG_TYPE_EVENT;	
		snprintf(tLog.acLogDesc,sizeof(tLog.acLogDesc)-1
				,"SwitchVideoSrc to %d",cType);
    	
    	acBuf[0] = 0xB0;
    	acBuf[1] = 0xFB;
    	acBuf[2] = 0x1;
    	acBuf[3] = 0x2;
    	acBuf[4] = cSrc^cType; // 数据异或
    	acBuf[5] = cSrc;
    	acBuf[6] = cType;
    	acBuf[7] = 0xE0;
	  	iRet = SendMultcastPacket(g_iVideoSwitchSocket, acMultAddr, usMultPort, acBuf, 8);
	}
	  // 是否接收响应，暂不接收  
    return iRet;
}

int GetVideoSrcMode()
{
	char acMultAddr[] = "224.0.0.88";
    unsigned short usMultPort = 12080;
    char acBuf[8];
    int iRet = 0;
    
    acBuf[0] = 0xB0;
    acBuf[1] = 0xFB;
    acBuf[2] = 0x2;
    acBuf[3] = 0;
    acBuf[4] = 0;
    acBuf[5] = 0xE0;
	iRet = SendMultcastPacket(g_iVideoSwitchSocket, acMultAddr, usMultPort, acBuf, 6);
    return iRet;
}

//IP(10.255.255.255)及发送端口（7000）
//0~100
int AdjustLcdVolume(char cVolume)
{
	int iRet = 0;
	
	if(GetASCUMasterSlaveFlag())
	{	
    	char acMultAddr[] = "10.255.255.255";
    	unsigned short usMultPort = 7000;
    	char acBuf[3];
    
    	acBuf[0] = 0x0;
    	acBuf[1] = cVolume;
    	//acBuf[2] = 0;
    	if(g_iLcdSocket<=0)
        {
            g_iLcdSocket = CreateLcdSocket();
        }   

        if(g_iLcdSocket >0)
        {
            iRet = SendMultcastPacket(g_iLcdSocket, acMultAddr, usMultPort, acBuf, 2);
            if(iRet <=0)
            {
                close(g_iLcdSocket);
                g_iLcdSocket = 0;
            }
        }
	}
    return iRet;
}

//IP(225.0.0.100)及发送端口（12306）
//0~100 
int AdjustDynamicMapBrightness(char cValue)
{
	int iRet = 0;
	if(GetASCUMasterSlaveFlag())
	{
    	char acMultAddr[] = "225.0.0.100";
    	unsigned short usMultPort = 12306;
    	char acBuf[10];
        T_LOG_INFO tLog;
				
		memset(&tLog,0,sizeof(T_LOG_INFO));
	    tLog.iLogType = LOG_TYPE_EVENT;	
		snprintf(tLog.acLogDesc,sizeof(tLog.acLogDesc)-1
				,"set dynamic map brightness  %d",cValue );
		LOG_WriteLog(&tLog);
        
    	acBuf[0] = 0x9F;
    	acBuf[1] = 0xA0;
    	acBuf[2] = 0x10;
    	acBuf[3] = 0x00;
    	acBuf[4] = 0x1;
    	acBuf[5] = 0x0;
    	acBuf[6] = 0x0;
    	acBuf[7] = cValue;
    	acBuf[8] = 0xEF;
    	acBuf[9] = 0xEF;
		iRet = SendMultcastPacket(g_iDynamicMapSocket, acMultAddr, usMultPort, acBuf, 10);   
    }
	return iRet;
}

int SendHeartInfo()
{
    char acMultAddr[] = "224.0.0.88";
    unsigned short usMultPort = 12090;
    T_tagBeaconMsg tMsg;
    int iRet = 0;
    
    memset(&tMsg,0,sizeof(T_tagBeaconMsg));
    tMsg.head = 0xc0;
    tMsg.heads = 0xf0;
    
	GetDeviceIp(tMsg.ip,16);
	
	iRet = SendMultcastPacket(g_iHeartSockt, acMultAddr, usMultPort, &tMsg, sizeof(tMsg));
    return iRet;
}
