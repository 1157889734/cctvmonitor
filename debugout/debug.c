#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>



#include "debug.h"

enum _E_DEBUG_MSG
{
    DEBUG_START     = 0x01, 
    DEBUG_END       = 0x02, 
    DEBUG_HEART     = 0x03,
    DEBUG_SET_LEVEL = 0x04
};

static unsigned int g_uiPrintLevel = 0;
static int g_iDegugConnectFlag = 0;
static int g_iListenSocket = -1;
static int g_iConnSocket = -1;
static int g_iHeartTimeout = 0;
static     pthread_mutex_t	g_tDebugMutex;
static  pthread_t g_DebugThreadHandle;
static int g_iThreadRunFlag = 0;



void DebugPrint(unsigned int uiDebugLevel, const char *format, ...)
{
    va_list args;
    int iLen = 0;
    int iRet = 0;
    struct timeval tv;
    
    if (0 == g_iDegugConnectFlag)
    {
        if((uiDebugLevel & DEBUG_NORMAL_PRINT) || (uiDebugLevel & DEBUG_ERROR_PRINT))
        {
			char acBuf[768] = {0};
			char acTimeStr[40] = {0};
			struct tm* ptm;
            int iTimeLen = 0;
            
            time(&tv.tv_sec);
            ptm = localtime (&tv.tv_sec);
            strftime (acTimeStr, sizeof(acTimeStr), "\r\n%Y-%m-%d %H:%M:%S ", ptm);
            iTimeLen = strlen(acTimeStr); 
            strcpy(acBuf,acTimeStr);
			  
            va_start(args,format);
            iLen = vsnprintf(acBuf + iTimeLen, 728, format, args);
			printf(acBuf);
			va_end(args);
			
           /* va_start(args,format);
            iLen = vprintf(format, args);
            va_end(args);*/
        }
    }
    else
    {
        if(uiDebugLevel & g_uiPrintLevel)
        {
            char acBuf[768] = {0};
            char acTimeStr[40] = {0};
            struct tm* ptm;
            int iTimeLen = 0;
            
            time(&tv.tv_sec);
            ptm = localtime (&tv.tv_sec);
            strftime (acTimeStr, sizeof(acTimeStr), "\r\n%Y-%m-%d %H:%M:%S ", ptm);
            iTimeLen = strlen(acTimeStr); 
            strcpy(acBuf,acTimeStr);
			  
            va_start(args,format);
            iLen = vsnprintf(acBuf + iTimeLen, 728, format, args);
            if(iLen < 0)
            {
                printf("**********ERROR:Too few parameters at DebugPrint**********\n");
            }
            va_end(args);
            if (iLen >= 0)
            {
                pthread_mutex_lock(&g_tDebugMutex);
                iRet = send(g_iConnSocket, acBuf, strlen(acBuf), 0);
                if (iRet <= 0)
                {
                    printf("debuger send error");
                    perror(":");
                    close(g_iConnSocket);
                    g_iConnSocket = -1;
                    g_iDegugConnectFlag = 0;
                    g_uiPrintLevel = 0;
                }
                pthread_mutex_unlock(&g_tDebugMutex);
            }
        }
    }
}

int DebugCmdProcess(void)
{
    int iMaxFd = 0;
    int iResult = 0;
    int iConnFd = 0;
    fd_set	tReadSet;
    struct timeval tv;
    char acBuf[128];
    struct sockaddr_in cliaddr;
    int clilen = sizeof(cliaddr);

    iMaxFd = (g_iConnSocket > g_iListenSocket) ? g_iConnSocket : g_iListenSocket;
	FD_ZERO(&tReadSet);
	if (g_iListenSocket > 0)
	{
		FD_SET(g_iListenSocket, &tReadSet);
	}
	if (g_iConnSocket > 0)
	{
		FD_SET(g_iConnSocket, &tReadSet);	
	}
		
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
	iResult = select(iMaxFd + 1, &tReadSet, NULL, NULL, &tv);
	if(iResult <= 0)
	{
		g_iHeartTimeout++;
			  
		if (g_iHeartTimeout > 1000 && g_iConnSocket > 0)
		{
			pthread_mutex_lock(&g_tDebugMutex);
            strcpy(acBuf, "Debuger Timeout!\n");
			close(g_iConnSocket);
            g_iConnSocket = -1;
            g_iDegugConnectFlag = 0;
            g_uiPrintLevel = 0;
            g_iHeartTimeout = 0;
            pthread_mutex_unlock(&g_tDebugMutex);
		}
	}
	else
	{
        if(FD_ISSET(g_iListenSocket, &tReadSet)) 
        {
            iConnFd = accept(g_iListenSocket, (struct sockaddr *)&cliaddr, &clilen);
            if (iConnFd > 0)
            {
                if (g_iConnSocket > 0)
                {
                    pthread_mutex_lock(&g_tDebugMutex);
                    strcpy(acBuf, "Debuger have a new client connect, and disconnect old client\n");
                    send(g_iConnSocket, acBuf, strlen(acBuf), 0);
                    close(g_iConnSocket);
                    g_iConnSocket = -1;
                    g_iDegugConnectFlag = 0;
                    g_uiPrintLevel = 0;
                    pthread_mutex_unlock(&g_tDebugMutex);
                }
                g_iConnSocket = iConnFd;
                g_iHeartTimeout = 0;
                
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                if (setsockopt(g_iConnSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)))
                {
                    perror("setsockopt");
                }
                
                return 0;
            }
        }
        if (g_iConnSocket > 0)
        {
            if(FD_ISSET(g_iConnSocket, &tReadSet)) 
            {
                unsigned char acRecvBuf[128];
                unsigned char ucType = 0;
                unsigned int uiLevel = 0;
                int iRecvLen = 0;

                memset(acRecvBuf, 0, sizeof(acRecvBuf));
                iRecvLen = recv(g_iConnSocket, acRecvBuf, sizeof(acRecvBuf), 0);
                if (iRecvLen <= 0)
                {
                    close(g_iConnSocket);
                    g_iConnSocket = -1;
                    g_iDegugConnectFlag = 0;
                    g_uiPrintLevel = 0;
                    perror("Debug recv error0:");
                    return 0;
                }
                if (0xFF != acRecvBuf[0])
                {
                    return 0;	
                }
                ucType = acRecvBuf[1];
                g_iHeartTimeout = 0;
                switch(ucType)
                {
                    case DEBUG_START:
                    {
                        uiLevel = *(unsigned int *)(acRecvBuf+4);
                        pthread_mutex_lock(&g_tDebugMutex);
                        g_uiPrintLevel = ntohl(uiLevel);
                        sprintf(acBuf, "Debuger print start, level %x\r\n", g_uiPrintLevel);
                        send(g_iConnSocket, acBuf, strlen(acBuf), 0);
                        g_iDegugConnectFlag = 1;
                        pthread_mutex_unlock(&g_tDebugMutex);
                        printf("%s", acBuf);
                        
                        break;
                    }
                    case DEBUG_END:
                    {	
                        pthread_mutex_lock(&g_tDebugMutex);
                        strcpy(acBuf, "Debuger print stop\n");
                        send(g_iConnSocket, acBuf, strlen(acBuf), 0);
                        close(g_iConnSocket);
                        g_iConnSocket = -1;
                        g_iDegugConnectFlag = 0;
                        g_uiPrintLevel = 0;
                        pthread_mutex_unlock(&g_tDebugMutex);
                        printf("%s", acBuf);
                        
                        break;
                    }
                    case DEBUG_HEART:
                        break;
                    case DEBUG_SET_LEVEL:
                    {
                        uiLevel = *(unsigned int *)(acRecvBuf+4);
                        g_uiPrintLevel = ntohl(uiLevel);
                        printf("Debuger print level %x\n", g_uiPrintLevel);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
	}
	return 0;
}

#ifdef WIN
	DWORD WINAPI ProcessThread(void *arg)
#else
  void *ProcessThread(void *arg)
#endif
{
	while(g_iThreadRunFlag)
	{
		DebugCmdProcess();
		usleep(3000);
	}
}

int DebugInit(short sDebugPort)
{
    int flag = 1;
    int iSockFd = -1;
    int iRet = 0;
    struct sockaddr_in servaddr;
    pthread_mutexattr_t mutexattr;
    
    iSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (iSockFd < 0)
    {
        perror("socket error:");
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(sDebugPort);

    iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(int));
    if(iRet != 0)
    {
        printf("ERR: setsockopt debug socket error. err = %d,errno = %d[%s]\n",iRet, errno, strerror(errno));
        close(iSockFd);
        return -1;
    }
	
    iRet = bind(iSockFd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (iRet < 0)
    {
        printf("bind debug port %d  error\n", sDebugPort);
        perror(":");
        close(iSockFd);
        return -1;
    }

    listen(iSockFd, 1);
    
    g_iListenSocket = iSockFd;
   
    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr,PTHREAD_MUTEX_TIMED_NP);
    pthread_mutex_init(&g_tDebugMutex,&mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
	g_iThreadRunFlag = 1;
	pthread_create(&g_DebugThreadHandle, NULL, ProcessThread, NULL);
    
    return 0;
}

void DebugUninit(void)
{
    g_uiPrintLevel = 0;
    g_iDegugConnectFlag = 0;
	g_iThreadRunFlag = 0;
	pthread_join(g_DebugThreadHandle, NULL);
    if (g_iListenSocket > 0)
    {
        close(g_iListenSocket);
        g_iListenSocket = -1;
    }
    
    if (g_iConnSocket > 0)
    {
        close(g_iConnSocket);
        g_iConnSocket = -1;
    }
    
    pthread_mutex_destroy(&g_tDebugMutex);
}


