
#ifdef WIN
  #include "stdafx.h"
  #include <io.h>
  #include <sys/types.h>
  #include <sys/stat.h>
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <sys/socket.h>
#endif

#include "mutex.h"
#include "types.h"
#include "rtspComm.h"
#include "rtsp.h"
#include "rtp.h"
#include "rtcp.h"
#include "rtspApi.h"
#include "md5.h"
#include "Base64EncDec.h"
#include "ourMD5.h"

static int s_iSrandSeed = 1;

INT32 CreateUdpSocket(INT32 iUdpPort)
{
    INT32 iSocket = -1;
    INT32 iReuseFlag = 1;
    UINT16 u16UdpPort = (UINT16)iUdpPort;
   // SOCKADDR_IN   dataSocketAddr;             /* Socket Info structure              */
    struct sockaddr_in dataSocketAddr;
    INT32 iLength = sizeof(dataSocketAddr);
    INT32 windowSize;
    INT32 windowSizeLen;
    
    iSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (iSocket <= 0)
    {
        return -1;	
    }

    /* Set Reusable flag                                                  */
    if ( setsockopt(iSocket, SOL_SOCKET, SO_REUSEADDR, \
                    (const char *)&iReuseFlag, sizeof(INT32)) < 0 )
    {
            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Failed to set Reuseaddr\n");
    }
    dataSocketAddr.sin_family      = AF_INET;
    dataSocketAddr.sin_addr.s_addr = htonl (INADDR_ANY);
    dataSocketAddr.sin_port = htons(u16UdpPort);

    if (bind(iSocket, (struct sockaddr *)&dataSocketAddr, iLength) < 0)
    {

        return -1;
    }
    windowSize = 1024*1024;
    windowSizeLen = sizeof(INT32);
    if ( setsockopt(iSocket, SOL_SOCKET, SO_RCVBUF, (char *)&windowSize, \
                                windowSizeLen) < 0 )
    {
            
    }
        
    return iSocket;
}

#if 0
int GetAuthorizationResponce(PT_RTSP_CONN ptRtspConn, unsigned char *pcMethod, unsigned char *pcAuthResponce)
{
    struct MD5Context ctx;
    char acTmp[512];
    unsigned char acMd5Tmp[16];
    char acMd5Passwd[36];
    char acMd5Method[36];
    char acMd5Response[36];
    int i = 0;

    if (NULL == pcAuthResponce)
    {
        return -1;
    }

    memset(&ctx, 0, sizeof(ctx));
    memset(acTmp, 0, sizeof(acTmp));
    memset(acMd5Tmp, 0, sizeof(acMd5Tmp));
    memset(acMd5Passwd, 0, sizeof(acMd5Passwd));
    memset(acMd5Method, 0, sizeof(acMd5Method));
    memset(acMd5Response, 0, sizeof(acMd5Response));

    // md5(username:realm:password)
    sprintf((char *)acTmp, "%s:%s:%s", ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acPasswd);
    MD5Init(&ctx);
    MD5Update(&ctx, acTmp, strlen((char *)acTmp));
    MD5Final(acMd5Tmp, &ctx);

    for(i = 0; i < 16; i++)
    {
        sprintf(&acMd5Passwd[i * 2], "%02x", acMd5Tmp[i]);
    }

    // md5(public_method:url)
    if (strstr(ptRtspConn->acNewUri, "rtsp"))
    {
        sprintf((char *)acTmp, "%s:%s", pcMethod, ptRtspConn->acNewUri);
    }
    else
    {
        sprintf((char *)acTmp, "%s:%s", pcMethod, ptRtspConn->acUri);
    }
    MD5Init(&ctx);
    MD5Update(&ctx, acTmp, strlen((char *)acTmp));
    MD5Final(acMd5Tmp, &ctx);

    for(i = 0; i < 16; i++)
    {
        sprintf(&acMd5Method[i * 2], "%02x", acMd5Tmp[i]);
    }

    // md5 total
    sprintf((char *)acTmp, "%s:%s:%s", acMd5Passwd, ptRtspConn->acAuthNonce, acMd5Method);
    MD5Init(&ctx);
    MD5Update(&ctx, acTmp, strlen((char *)acTmp));
    MD5Final(acMd5Tmp, &ctx);

    for(i = 0; i < 16; i++)
    {
        sprintf(&acMd5Response[i * 2], "%02x", acMd5Tmp[i]);
    }

    memcpy(pcAuthResponce, acMd5Response, 32);

    return 0;
}
#endif

#if 1
int GetAuthorizationResponce(PT_RTSP_CONN ptRtspConn, unsigned char *pcMethod, unsigned char *pcAuthResponce)
{
	struct MD5Context ctx;
    char acTmp[512];
    unsigned char acMd5Tmp[16];
	char acMd5Passwd[36];
	char acMd5Method[36];
	char acMd5Response[36];
	int i = 0;

	if (NULL == pcAuthResponce)
	{
	    return -1;
	}

	memset(&ctx, 0, sizeof(ctx));
    memset(acTmp, 0, sizeof(acTmp));
	memset(acMd5Tmp, 0, sizeof(acMd5Tmp));
	memset(acMd5Passwd, 0, sizeof(acMd5Passwd));
	memset(acMd5Method, 0, sizeof(acMd5Method));
    memset(acMd5Response, 0, sizeof(acMd5Response));

	// md5(username:realm:password)
	sprintf((char *)acTmp, "%s:%s:%s", ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acPasswd);

    our_MD5Data((const unsigned char *)acTmp, strlen(acTmp), acMd5Passwd);
/*
	MD5Init(&ctx);
	MD5Update(&ctx, acTmp, strlen((char *)acTmp));
	MD5Final(acMd5Tmp, &ctx);

	for(i = 0; i < 16; i++)
	{
		sprintf(&acMd5Passwd[i * 2], "%02x", acMd5Tmp[i]);
	}
*/


	// md5(public_method:url)
	if (strstr(ptRtspConn->acNewUri, "rtsp"))
	{
	    sprintf((char *)acTmp, "%s:%s", pcMethod, ptRtspConn->acNewUri);
	}
	else
	{
		sprintf((char *)acTmp, "%s:%s", pcMethod, ptRtspConn->acUri);
	}
    /*
	MD5Init(&ctx);
	MD5Update(&ctx, acTmp, strlen((char *)acTmp));
	MD5Final(acMd5Tmp, &ctx);

	for(i = 0; i < 16; i++)
	{
		sprintf(&acMd5Method[i * 2], "%02x", acMd5Tmp[i]);
	}
    */
    our_MD5Data((const unsigned char *)acTmp, strlen(acTmp), acMd5Method);

	// md5 total
	sprintf((char *)acTmp, "%s:%s:%s", acMd5Passwd, ptRtspConn->acAuthNonce, acMd5Method);

    /*
	MD5Init(&ctx);
	MD5Update(&ctx, acTmp, strlen((char *)acTmp));
	MD5Final(acMd5Tmp, &ctx);

	for(i = 0; i < 16; i++)
	{
		sprintf(&acMd5Response[i * 2], "%02x", acMd5Tmp[i]);
	}
    */
    our_MD5Data((const unsigned char *)acTmp, strlen(acTmp), acMd5Response);

	memcpy(pcAuthResponce, acMd5Response, 32);

	return 0;
}

#endif

RTSP_HANDLE RTSP_Login(const char *pUrl, char *pcUser, char *pcPasswd)
{
    PT_RTSP_CONN ptRtspConn = NULL;
   // T_RTSP_REQUEST_MSG tRequestMsg = {0};
    int iRet = 0;
	char acBuf[512];
    
    ptRtspConn = (PT_RTSP_CONN)malloc(sizeof(T_RTSP_CONN));
    if (NULL == ptRtspConn)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "can't malloc memery\n");
        return 0;	
    }
    memset(ptRtspConn, 0, sizeof(T_RTSP_CONN));

    iRet = RtspConnect(ptRtspConn, pUrl);
    if (iRet < 0)
    {
        free(ptRtspConn);
        ptRtspConn = NULL;
        return 0;	
    }

	if (pcUser)
	{
		strncpy(ptRtspConn->acUser, pcUser, sizeof(ptRtspConn->acUser));
	}
	if (pcPasswd)
	{
		strncpy(ptRtspConn->acPasswd, pcPasswd, sizeof(ptRtspConn->acPasswd));
	}
#if 1
    /* now lets send an options request. */
    AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", "0");
    AddRequestHeader(&ptRtspConn->tRequestMsg, "Accept", "application/sdp");
    AddRequestHeader(&ptRtspConn->tRequestMsg, "User-Agent", "ONVIF Filter");
    iRet = RtspRequestOptions(ptRtspConn);
    if (iRet < 0)
    {
	    RTSP_Close(ptRtspConn->iRtspSocket);
	    ptRtspConn->iRtspSocket = -1;
	    free(ptRtspConn);
        ptRtspConn = NULL;
        
        return 0;
    }
#endif
	/* send an describe request. */
    ptRtspConn->iRtspCseq++;
    sprintf(acBuf, "%d", ptRtspConn->iRtspCseq);
    AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", acBuf);
    AddRequestHeader(&ptRtspConn->tRequestMsg, "Accept", "application/sdp");
    AddRequestHeader(&ptRtspConn->tRequestMsg, "User-Agent", "ONVIF Filter");
    iRet = RtspRequestDescribe(ptRtspConn);


	if (iRet < 0)
	{
		if (RET_UNAUTHORIZATION == iRet)
		{
			/* send an describe request. */
			ptRtspConn->iRtspCseq++;
			sprintf(acBuf, "%d", ptRtspConn->iRtspCseq);
			AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", acBuf);

			// ��?��?��a��??��
			if (AUTHORIZATE_TYPE_DIGEST == ptRtspConn->iAuthorizeFlag)
			{
				unsigned char acAuthResponce[36];
				unsigned char acMethod[32];

				memset(acAuthResponce, 0, sizeof(acAuthResponce));
				memset(acMethod, 0, sizeof(acMethod));

				strcpy((char *)acMethod, "DESCRIBE");
                GetAuthorizationResponce(ptRtspConn, acMethod, acAuthResponce);

				if (strstr(ptRtspConn->acNewUri, "rtsp"))
				{
					sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
						ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acNewUri, 
						acAuthResponce);
				}
				else
				{
					sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
						ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acUri, 
						acAuthResponce);
				}
                AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
			}
			else if (AUTHORIZATE_TYPE_BASIC == ptRtspConn->iAuthorizeFlag)
			{
				unsigned char acAuthResponce[128];
				char acInputStr[128];
				int iAuthRespLen = 0;

				memset(acAuthResponce, 0, sizeof(acAuthResponce));
				sprintf(acInputStr, "%s:%s", ptRtspConn->acUser, ptRtspConn->acPasswd);
			    Base64Encode((unsigned char *)acInputStr, strlen(acInputStr), acAuthResponce, &iAuthRespLen);
				sprintf(acBuf, "Basic %s=", acAuthResponce);
				AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
			}

			AddRequestHeader(&ptRtspConn->tRequestMsg, "Accept", "application/sdp");
            AddRequestHeader(&ptRtspConn->tRequestMsg, "User-Agent", "ONVIF Filter");
            iRet = RtspRequestDescribe(ptRtspConn);
		}

		if (iRet < 0)
		{
			RTSP_Close(ptRtspConn->iRtspSocket);
			ptRtspConn->iRtspSocket = -1;	
			free(ptRtspConn);
			ptRtspConn = NULL;

			return 0;
		}
	}
	MUTEX_INIT(&ptRtspConn->tMutex);

    return (RTSP_HANDLE)ptRtspConn;
}

int RTSP_Logout(RTSP_HANDLE RHandle)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;

    if (NULL == ptRtspConn)
    {
        return -1;	
    }

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "rtsp logout uri %s\n", ptRtspConn->acUri);
   
    /* delete rtp conn list */
    while (ptRtspConn->ptRtpList)
    {
        PT_RTP_CONN  ptRtpConn = NULL;
        
        ptRtspConn->ptRtpList->iRtpThreadRunFlag = 0;
        
        RTSP_Close(ptRtspConn->ptRtpList->iRtpSocket);
        ptRtspConn->ptRtpList->iRtpSocket = -1;
        if (ptRtspConn->ptRtpList->RtpRecvThread)
        {
            RTSP_ThreadJoin(ptRtspConn->ptRtpList->RtpRecvThread);
        }
        ptRtpConn = ptRtspConn->ptRtpList->next;
        free(ptRtspConn->ptRtpList);
        ptRtspConn->ptRtpList = ptRtpConn;
    }

    /* delete rtcp conn list */
    while (ptRtspConn->ptRtcpList)
    {
        PT_RTCP_CONN  ptRtcpConn = NULL;
        
        ptRtspConn->ptRtcpList->iRtcpThreadRunFlag = 0;

        RTSP_Close(ptRtspConn->ptRtcpList->iRtcpSocket);
        ptRtspConn->ptRtcpList->iRtcpSocket = -1;
        if (ptRtspConn->ptRtcpList->RtcpRecvThread)
        {
            RTSP_ThreadJoin(ptRtspConn->ptRtcpList->RtcpRecvThread);
        }
        ptRtcpConn = ptRtspConn->ptRtcpList->next;
        free(ptRtspConn->ptRtcpList);
        ptRtspConn->ptRtcpList = ptRtcpConn;
    }

    /* delete rtsp conn */
    ptRtspConn->iRtspThreadRunFlag = 0;
    if (ptRtspConn->iRtspSocket > 0)
    {
        RTSP_Close(ptRtspConn->iRtspSocket);
        ptRtspConn->iRtspSocket = -1;
    }
    if (ptRtspConn->RtspRecvThread)
    {
        RTSP_ThreadJoin(ptRtspConn->RtspRecvThread);
    }

    MUTEX_DESTROY(&ptRtspConn->tMutex);
	
    free(ptRtspConn);
    ptRtspConn = NULL;

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "rtsp logout exit\n");
    
    return 0;
}


int RTSP_OpenStream(RTSP_HANDLE RHandle, int iCh, int iRtpTransportProtocol, void *pfRtpSetDataCB, void *pUserData)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;
    int iRet = 0;
    int iUdpPort = 0;
    char acBuf[512];
    
    if (NULL == ptRtspConn)
    {
        return -1;	
    }
    
	memset(acBuf, 0, sizeof(acBuf));

    ptRtspConn->iEnableCBFunFlag = 1;
    ptRtspConn->iRtpTransportProtocol = iRtpTransportProtocol;
    ptRtspConn->pfRtpSetDataCB = (PF_RTP_SET_DATA_CALLBACK)pfRtpSetDataCB;
    ptRtspConn->pRtpCbArg = pUserData;
    
    
    if (TCP == iRtpTransportProtocol)
    {
    #ifdef WIN
        int timeout = 60000;

    #else
        struct timeval timeout;
        timeout.tv_sec = 60;
        timeout.tv_usec = 0;  

    #endif
        if(setsockopt(ptRtspConn->iRtspSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout))==-1)
        {
            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Set SO_RCVTIMEO error\n");
            RTSP_Close(ptRtspConn->iRtspSocket);
            ptRtspConn->iRtspSocket = -1;
            return -4;
        }	
    }
   	
	//srand( (unsigned)time( NULL ) );
	srand(s_iSrandSeed++);

	//iUdpPort = (int)((double)rand() / (RAND_MAX + 1) * (50000 - 20000) + 20000);
	iUdpPort = (int)(30000.0*rand()/(RAND_MAX+1.0)) + 20000;

  //  for (int i = 0; i < iCh; i++)
    {
        PT_RTP_CONN  ptRtpConn = NULL;
        PT_RTCP_CONN ptRtcpConn = NULL;
            
   	    /* send an setup request. */
        ptRtspConn->iRtspCseq++;
        sprintf(acBuf, "%d", ptRtspConn->iRtspCseq);
        AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", acBuf);

		// ��?��?��a��??��
		if (AUTHORIZATE_TYPE_DIGEST == ptRtspConn->iAuthorizeFlag)
		{
			unsigned char acAuthResponce[36];
			unsigned char acMethod[32];

			memset(acAuthResponce, 0, sizeof(acAuthResponce));
			memset(acMethod, 0, sizeof(acMethod));

			strcpy((char *)acMethod, "SETUP");
			GetAuthorizationResponce(ptRtspConn, acMethod, acAuthResponce);
			if (strstr(ptRtspConn->acNewUri, "rtsp"))
			{
				sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
					ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acNewUri, 
					acAuthResponce);
			}
			else
			{
				sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
					ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acUri, 
					acAuthResponce);
			}
			AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
		}
		else if (AUTHORIZATE_TYPE_BASIC == ptRtspConn->iAuthorizeFlag)
		{
			unsigned char acAuthResponce[128];
			char acInputStr[128];
			int iAuthRespLen = 0;

			memset(acAuthResponce, 0, sizeof(acAuthResponce));
			sprintf(acInputStr, "%s:%s", ptRtspConn->acUser, ptRtspConn->acPasswd);
			Base64Encode((unsigned char *)acInputStr, strlen(acInputStr), acAuthResponce, &iAuthRespLen);
			sprintf(acBuf, "Basic %s=", acAuthResponce);
			AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
		}

		AddRequestHeader(&ptRtspConn->tRequestMsg, "User-Agent", "ONVIF Filter");
        if (TCP == ptRtspConn->iRtpTransportProtocol)
        {
            sprintf(acBuf, "%s", "RTP/AVP/TCP;unicast;interleaved=0-1");
        }
        else
        {
            ptRtpConn = (PT_RTP_CONN)malloc(sizeof(T_RTP_CONN));
            if (NULL == ptRtpConn)
            {
                return -3;
            }
            memset(ptRtpConn, 0, sizeof(T_RTP_CONN));
            
            ptRtcpConn = (PT_RTCP_CONN)malloc(sizeof(T_RTCP_CONN));
            if (NULL == ptRtcpConn)
            {
                free(ptRtpConn);
                ptRtpConn = NULL;
            }
            memset(ptRtcpConn, 0, sizeof(T_RTCP_CONN));
            

            ptRtpConn->iRtpPort = iUdpPort ++;
            ptRtcpConn->iRtcpPort = iUdpPort ++;

            sprintf(acBuf, "RTP/AVP;unicast;client_port=%d-%d",
                     iUdpPort, iUdpPort + 1); 
		
            ptRtpConn->iRtpPort = iUdpPort;
            ptRtpConn->iRtpSocket = CreateUdpSocket(ptRtpConn->iRtpPort);
            ptRtpConn->ptRtspConn = ptRtspConn;
            ptRtpConn->iRtpThreadRunFlag = 1;

            ptRtcpConn->iRtcpPort = iUdpPort + 1;
            ptRtcpConn->iRtcpSocket= CreateUdpSocket(ptRtcpConn->iRtcpPort);
            ptRtcpConn->ptRtspConn = ptRtspConn;
            ptRtcpConn->iRtcpThreadRunFlag = 1;
        }
        AddRequestHeader(&ptRtspConn->tRequestMsg, "Transport", acBuf);

        iRet = RtspRequestSetup(ptRtspConn, E_VIDEO_TRACK);
        if (iRet < 0)
        {
             if (ptRtpConn)
             {
    	          free(ptRtpConn);
    	          ptRtpConn = NULL;
             }
             if (ptRtcpConn)
             {
    	         free(ptRtcpConn);
    	          ptRtcpConn = NULL;
             }
    	      return iRet;
        }
        
    #ifdef WIN
        if (UDP == iRtpTransportProtocol)
        {
            ptRtpConn->RtpRecvThread = CreateThread(NULL, 0, RtpRecvThread, ptRtpConn, 0, NULL);
            ptRtcpConn->RtcpRecvThread = CreateThread(NULL, 0, RtcpRecvThread, ptRtcpConn, 0, NULL);
        }
    #else
        if (UDP == iRtpTransportProtocol)
        {
            pthread_create(&(ptRtpConn->RtpRecvThread), NULL, RtpRecvThread, (void *)ptRtpConn);
            pthread_create(&(ptRtcpConn->RtcpRecvThread), NULL, RtcpRecvThread, (void *)ptRtcpConn);
        }
    #endif
    
        if (UDP == iRtpTransportProtocol)
        {
            ptRtcpConn->ptRtpConn = ptRtpConn;

            /* add rtp conn to list */
            if (ptRtspConn->ptRtpList == NULL)
            {
                ptRtspConn->ptRtpList = ptRtpConn;
            }
            else
            {
                PT_RTP_CONN  ptTmpList = NULL;
        
                ptTmpList = ptRtspConn->ptRtpList;
                while (ptTmpList->next != NULL)
                {
                    ptTmpList = ptTmpList->next;
                }

                ptTmpList->next = ptRtpConn;
            }
            
            /* add rtcp conn to list */
            if (ptRtspConn->ptRtcpList == NULL)
            {
                ptRtspConn->ptRtcpList = ptRtcpConn;
            }
            else
            {
                PT_RTCP_CONN  ptTmpList = NULL;
        
                ptTmpList = ptRtspConn->ptRtcpList;
                while (ptTmpList->next != NULL)
                {
                    ptTmpList = ptTmpList->next;
                }

                ptTmpList->next = ptRtcpConn;
            }

        }
    }

	if (TCP == ptRtspConn->iRtpTransportProtocol && strlen(ptRtspConn->acAudioTrackId))
	{
		/* send an setup request. */
		ptRtspConn->iRtspCseq++;
		sprintf(acBuf, "%d", ptRtspConn->iRtspCseq);
		AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", acBuf);

		// �Ƿ�Ҫ��֤
		if (AUTHORIZATE_TYPE_DIGEST == ptRtspConn->iAuthorizeFlag)
		{
			unsigned char acAuthResponce[36];
			unsigned char acMethod[32];

			memset(acAuthResponce, 0, sizeof(acAuthResponce));
			memset(acMethod, 0, sizeof(acMethod));

			strcpy((char *)acMethod, "SETUP");
			GetAuthorizationResponce(ptRtspConn, acMethod, acAuthResponce);
			if (strstr(ptRtspConn->acNewUri, "rtsp"))
			{
				sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
					ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acNewUri, 
					acAuthResponce);
			}
			else
			{
				sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
					ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acUri, 
					acAuthResponce);
			}
			AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
		}
		else if (AUTHORIZATE_TYPE_BASIC == ptRtspConn->iAuthorizeFlag)
		{
			unsigned char acAuthResponce[128];
			char acInputStr[128];
			int iAuthRespLen = 0;

			memset(acAuthResponce, 0, sizeof(acAuthResponce));
			sprintf(acInputStr, "%s:%s", ptRtspConn->acUser, ptRtspConn->acPasswd);
			Base64Encode((unsigned char *)acInputStr, strlen(acInputStr), acAuthResponce, &iAuthRespLen);
			sprintf(acBuf, "Basic %s=", acAuthResponce);
			AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
		}

		AddRequestHeader(&ptRtspConn->tRequestMsg, "User-Agent", "ONVIF Filter");

		sprintf(acBuf, "%s", "RTP/AVP/TCP;unicast;interleaved=2-3");
		AddRequestHeader(&ptRtspConn->tRequestMsg, "Transport", acBuf);
		AddRequestHeader(&ptRtspConn->tRequestMsg, "Session", ptRtspConn->acSession);
		iRet = RtspRequestSetup(ptRtspConn, E_AUDIO_TRACK);
		if (iRet < 0)
		{
			
		//	return iRet;
		}

	}

    ptRtspConn->iRtspThreadRunFlag = 1;
#ifdef WIN
    ptRtspConn->RtspRecvThread = CreateThread(NULL, 0, RtspRecvThread, ptRtspConn, 0, NULL);
#else
    pthread_create(&(ptRtspConn->RtspRecvThread), NULL, RtspRecvThread, (void *)ptRtspConn);
#endif


    return 0;
}

int RTSP_CloseStream(RTSP_HANDLE RHandle, int iCh)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;
    char acCseq[12];
    int iRet = 0;

    if (NULL == ptRtspConn)
    {
        return -1;	
    }
    
    ptRtspConn->iRtspCseq++;
    sprintf(acCseq, "%d", ptRtspConn->iRtspCseq);
    AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", acCseq);
    AddRequestHeader(&ptRtspConn->tRequestMsg, "User-Agent", "ONVIF Filter");
    AddRequestHeader(&ptRtspConn->tRequestMsg, "Session", ptRtspConn->acSession);

    iRet = RtspRequestTeardown(ptRtspConn);

    return 0;
}

int RTSP_PlayControl(RTSP_HANDLE RHandle, int iCmd, double dValue)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;
    int iRet = 0;
    char acBuf[512];
	  
    if (NULL == ptRtspConn)
    {
        return -1;	
    }
    
    ptRtspConn->iRtspCseq++;
    sprintf(acBuf, "%d", ptRtspConn->iRtspCseq);
    AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", acBuf);

	// �Ƿ�Ҫ��֤
	if (AUTHORIZATE_TYPE_DIGEST == ptRtspConn->iAuthorizeFlag)
	{
		unsigned char acAuthResponce[36];
		unsigned char acMethod[32];

		memset(acAuthResponce, 0, sizeof(acAuthResponce));
		memset(acMethod, 0, sizeof(acMethod));

		if (E_PLAY_STATE_PAUSE == iCmd)
		{
		    strcpy((char *)acMethod, "PAUSE");
		}
		else
		{
			strcpy((char *)acMethod, "PLAY");
		}

		GetAuthorizationResponce(ptRtspConn, acMethod, acAuthResponce);
		if (strstr(ptRtspConn->acNewUri, "rtsp"))
		{
			sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
				ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acNewUri, 
				acAuthResponce);
		}
		else
		{
			sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
				ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acUri, 
				acAuthResponce);
		}
		AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
	}
	else if (AUTHORIZATE_TYPE_BASIC == ptRtspConn->iAuthorizeFlag)
	{
		unsigned char acAuthResponce[128];
		char acInputStr[128];
		int iAuthRespLen = 0;

		memset(acAuthResponce, 0, sizeof(acAuthResponce));
		sprintf(acInputStr, "%s:%s", ptRtspConn->acUser, ptRtspConn->acPasswd);
		Base64Encode((unsigned char *)acInputStr, strlen(acInputStr), acAuthResponce, &iAuthRespLen);
		sprintf(acBuf, "Basic %s=", acAuthResponce);
		AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
	}

    AddRequestHeader(&ptRtspConn->tRequestMsg, "User-Agent", "ONVIF Filter");
    AddRequestHeader(&ptRtspConn->tRequestMsg, "Session", ptRtspConn->acSession);
    
    if (E_PLAY_STATE_PLAY == iCmd)
    #if 0
    {
        //sprintf(acBuf, "%d", iValue);
		strcpy(acBuf, "npt=0.000-");
		DebugPrint(DEBUG_LEVEL_14, "RTSP_PlayControl play npt %lf\n", dValue);
        AddRequestHeader(&ptRtspConn->tRequestMsg, "Range", acBuf);
        //AddRequestHeader(&ptRtspConn->tRequestMsg, "Scale", "1.0");
        iRet = RtspRequestPlay(ptRtspConn);
        if (iRet < 0)
        {
			RTSP_DEBUG(DEBUG_LEVEL_14, "[%s:%d]RtspRequestPlay error\n", __FUNCTION__, __LINE__);
        }
    }  
    #endif 
	{
		//sprintf(acBuf, "%d", iValue);
		if (E_RTSP_STATE_PAUSE != ptRtspConn->iRtspState)
		{
			strcpy(acBuf, "npt=0.000-");
			AddRequestHeader(&ptRtspConn->tRequestMsg, "Range", acBuf);
		}
		AddRequestHeader(&ptRtspConn->tRequestMsg, "Scale", "1.0");
		iRet = RtspRequestPlay(ptRtspConn);
	}
    else if ((E_PLAY_STATE_FAST_FORWARD == iCmd) || (E_PLAY_STATE_FAST_REVERSE == iCmd))
    {
	    sprintf(acBuf, "%f", dValue);
		DebugPrint(DEBUG_LEVEL_14, "RTSP_PlayControl forward speed %lf\n", dValue);
        printf("RTSP_PlayControl forward speed %lf\n", dValue);
        AddRequestHeader(&ptRtspConn->tRequestMsg, "Scale", acBuf);
        iRet = RtspRequestPlay(ptRtspConn);
        if (iRet < 0)
        {
			RTSP_DEBUG(DEBUG_LEVEL_14, "[%s:%d]RtspRequestPlay error\n", __FUNCTION__, __LINE__);
        }
    }
    else if (E_PLAY_STATE_DRAG_POS == iCmd)
    {
        sprintf(acBuf, "npt=%f-", dValue);
		DebugPrint(DEBUG_LEVEL_14, "RTSP_PlayControl Set position to %lf\n", dValue);
        AddRequestHeader(&ptRtspConn->tRequestMsg, "Range", acBuf);
        iRet = RtspRequestPlay(ptRtspConn);
        if (iRet < 0)
        {
			RTSP_DEBUG(DEBUG_LEVEL_14, "[%s:%d]RtspRequestPlay error\n", __FUNCTION__, __LINE__);
        }
    }
    else if (E_PLAY_STATE_PAUSE == iCmd)
    {
		DebugPrint(DEBUG_LEVEL_14, "RTSP_PlayControl pause\n");
        iRet = RtspRequestPause(ptRtspConn);
        if (iRet < 0)
        {
			RTSP_DEBUG(DEBUG_LEVEL_14, "[%s:%d]RtspRequestPause error\n", __FUNCTION__, __LINE__);
        }
    }
    else
    {
        return 0;	
    }
    
    return iRet;
}

int RTSP_SendHeart(RTSP_HANDLE RHandle)
{
	  PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;
	  int iRet = 0;
	  char acBuf[512];
	  
    if (NULL == ptRtspConn)
    {	  	
        return -1;	
    }
	  
    ptRtspConn->iRtspCseq++;
    sprintf(acBuf, "%d", ptRtspConn->iRtspCseq);
    AddRequestHeader(&ptRtspConn->tRequestMsg, "CSeq", acBuf);

	// �Ƿ�Ҫ��֤
	if (AUTHORIZATE_TYPE_DIGEST == ptRtspConn->iAuthorizeFlag)
	{
		unsigned char acAuthResponce[36];
		unsigned char acMethod[32];

		memset(acAuthResponce, 0, sizeof(acAuthResponce));
		memset(acMethod, 0, sizeof(acMethod));

		strcpy((char *)acMethod, "GET_PARAMETER");
		GetAuthorizationResponce(ptRtspConn, acMethod, acAuthResponce);
		if (strstr(ptRtspConn->acNewUri, "rtsp"))
		{
			sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
				ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acNewUri, 
				acAuthResponce);
		}
		else
		{
			sprintf(acBuf, "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", response=\"%s\"", 
				ptRtspConn->acUser, ptRtspConn->acAuthRealm, ptRtspConn->acAuthNonce, ptRtspConn->acUri, 
				acAuthResponce);
		}
		AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
	}
	else if (AUTHORIZATE_TYPE_BASIC == ptRtspConn->iAuthorizeFlag)
	{
		unsigned char acAuthResponce[128];
		char acInputStr[128];
		int iAuthRespLen = 0;

		memset(acAuthResponce, 0, sizeof(acAuthResponce));
		sprintf(acInputStr, "%s:%s", ptRtspConn->acUser, ptRtspConn->acPasswd);
		Base64Encode((unsigned char *)acInputStr, strlen(acInputStr), acAuthResponce, &iAuthRespLen);
		sprintf(acBuf, "Basic %s=", acAuthResponce);
		AddRequestHeader(&ptRtspConn->tRequestMsg, "Authorization", acBuf);
	}

    AddRequestHeader(&ptRtspConn->tRequestMsg, "Session", ptRtspConn->acSession);
    iRet = RtspRequestGetParameter(ptRtspConn);
    
    return iRet;
}

int RTSP_GetRtspTimeout(RTSP_HANDLE RHandle)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;
	  
    if (NULL == ptRtspConn)
    {	  	
        return 0;	
    }
	  
    return ptRtspConn->iRtspTimeout;
}

int RTSP_GetParam(RTSP_HANDLE RHandle, int iType, void *pValue)
{
	PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;

	if (NULL == ptRtspConn)
	{
		return 0;	
	}

	if (E_TYPE_PLAY_RANGE == iType)
	{
		int *piRangeValue = (int *)pValue;

		*piRangeValue = ptRtspConn->iPlayRange;
//        printf("********RTSP_GetParam***line----934=%d\n",*piRangeValue);
    }

    return 0;
}

int RTSP_GetServIpAddr(RTSP_HANDLE RHandle, char *pcIpAddr, int iLen)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;
	  
    if (NULL == ptRtspConn)
    {
	  	
        return 0;	
    }
    strncpy(pcIpAddr, ptRtspConn->acServHost, iLen);
    return 0;
}

int RTSP_GetConnect(RTSP_HANDLE RHandle)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;
	  
    if (NULL == ptRtspConn)
    {	  	
        return -1;	
    }
	  
    return ptRtspConn->iRtspSocket;
}

int RTSP_CBFunEnable(RTSP_HANDLE RHandle,int iEnable)
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)RHandle;

    if (NULL == ptRtspConn)
    {	  	
        return -1;	
    }
    ptRtspConn->iEnableCBFunFlag = iEnable;
    return 0;
}


