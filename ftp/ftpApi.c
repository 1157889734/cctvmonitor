#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<pthread.h>
#include<signal.h>
#include<netinet/in.h>
#include<sys/wait.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include <sys/ioctl.h>

#include "ftpApi.h"
#include "gb2312_utf8.h" 
#include "./debugout/debug.h"
#include "./log/log.h"

typedef struct _T_FTP_PACKET
{
	int iCmdType;  //文件上传还是文件下载，1-文件下载，2-文件上传
    char acSrcFileName[256];
    char acDstFileName[256];
}__attribute__((packed)) T_FTP_PACKET, *PT_FTP_PACKET;

typedef struct _T_FTP_PACKET_LIST
{
    T_FTP_PACKET tPkt;
    struct _T_FTP_PACKET_LIST *next;
}__attribute__((packed))  T_FTP_PACKET_LIST;

typedef struct _T_FTP_QUEUE
{
    T_FTP_PACKET_LIST *ptFirst, *ptLast;
    int iQueueType;  // 0:先进先出(FIFO)，1:后进先出(LIFO)
    int iPktCount; 
    pthread_mutex_t *pMutex;			
}__attribute__((packed))  T_FTP_QUEUE, *PT_FTP_QUEUE;

typedef struct _T_FTP_CONNECTION_INFO
{
	int client_socket; //控制socket  
	int data_socket; //数据socket
	unsigned long long DownloadFileSize;
	int iMode;     //FTP模式，PORT/PASV
	int iUploadType;    //文件上传传输类型
	unsigned long long uploadFileOffset;    //文件上传偏移字节数，可做断点续传用
	unsigned long long uploadLocalFileSize;
	int threadRunFlag;
	pthread_t ThreadId;
	fd_set readSet;
    struct timeval selectTime;
    PF_FTP_PROC_CALLBACK pFtpProcFunc;
    pthread_mutex_t tMutex;
    PT_FTP_QUEUE ptQueue;
    pthread_mutex_t tQueueMutex;
}__attribute__((packed)) T_FTP_CONNECTION_INFO,*PT_FTP_CONNECTION_INFO;


PT_FTP_QUEUE FTP_CreateWorkQueue(pthread_mutex_t *pMutex, int iQueueType)
{
    PT_FTP_QUEUE ptWorkQueue = NULL;
    
    ptWorkQueue = (PT_FTP_QUEUE)malloc(sizeof(T_FTP_QUEUE));
    if (NULL == ptWorkQueue)
    {
        return NULL;
    }
    memset(ptWorkQueue, 0, sizeof(T_FTP_QUEUE));
    ptWorkQueue->pMutex = pMutex;
    ptWorkQueue->iQueueType = iQueueType;

    return ptWorkQueue;
}

int FTP_DestroyWorkQueue(PT_FTP_QUEUE ptWorkQueue)
{
    T_FTP_PACKET_LIST *ptPktList = NULL, *ptTmp;

    if (NULL == ptWorkQueue)
    {
        return -1;
    }

    if (ptWorkQueue->pMutex)
    {
        pthread_mutex_lock(ptWorkQueue->pMutex);
    }

    ptPktList = ptWorkQueue->ptFirst;
    while (ptPktList)
    {
        ptTmp = ptPktList;
        ptPktList = ptPktList->next;
        free(ptTmp);
    }
	
    ptWorkQueue->ptLast = NULL;
    ptWorkQueue->ptFirst = NULL;
    ptWorkQueue->iPktCount= 0;

    if (ptWorkQueue->pMutex)
    {
        pthread_mutex_unlock(ptWorkQueue->pMutex);
    }

    free(ptWorkQueue);

    return 0;
}

int FTP_PutNodeToWorkQueue(PT_FTP_QUEUE ptWorkQueue, PT_FTP_PACKET ptPkt)
{
    T_FTP_PACKET_LIST *ptPktList = NULL;

    if ((NULL == ptWorkQueue) || (NULL == ptPkt))
    {
        return -1;
    }
    ptPktList = (T_FTP_PACKET_LIST *)malloc(sizeof(T_FTP_PACKET_LIST));
    if (NULL == ptPktList)
    {
        return -1;
    }

    memset(ptPktList, 0, sizeof(T_FTP_PACKET_LIST));
    ptPktList->tPkt = *ptPkt;

    if (ptWorkQueue->pMutex)
    {
        pthread_mutex_lock(ptWorkQueue->pMutex);
    }

    if (NULL == ptWorkQueue->ptLast)
    {
	    ptWorkQueue->ptFirst = ptPktList;
    }
    else
    {
	    ptWorkQueue->ptLast->next = ptPktList;
    }
    ptWorkQueue->ptLast = ptPktList;
    ptWorkQueue->iPktCount++;

    if (ptWorkQueue->pMutex)
    {
        pthread_mutex_unlock(ptWorkQueue->pMutex);
    }

    return 0;
}

int FTP_GetNodeFromWorkQueue(PT_FTP_QUEUE ptWorkQueue, PT_FTP_PACKET ptPkt)
{
    T_FTP_PACKET_LIST *ptTmp = NULL;

    if ((NULL == ptWorkQueue) || (NULL == ptPkt))
    {
        return 0;
    }

    if (ptWorkQueue->pMutex)
    {
        pthread_mutex_lock(ptWorkQueue->pMutex);
    }

    if (NULL == ptWorkQueue->ptFirst)
    {
        if (ptWorkQueue->pMutex)
        {
            pthread_mutex_unlock(ptWorkQueue->pMutex);
        }

        return 0;
    }
	
    ptTmp = ptWorkQueue->ptFirst;
    ptWorkQueue->ptFirst = ptWorkQueue->ptFirst->next;
    if (NULL == ptWorkQueue->ptFirst)
    {
        ptWorkQueue->ptLast= NULL;
    }
    ptWorkQueue->iPktCount--;

    *ptPkt = ptTmp->tPkt;
    free(ptTmp);

    if (ptWorkQueue->pMutex)
    {
        pthread_mutex_unlock(ptWorkQueue->pMutex);
    }
    
    return 1;
}

static void get_file_name(char *file_name, char *name)
{
    char *pev1, *pev2;
    int len;    
    if((pev1=strstr(file_name,"/"))!=0)
    {
        pev2=strrchr(file_name,'/');
        len = strlen(file_name)-(int)(pev2-file_name)-1;
        strncpy(name, pev2+1, len);
        name[len]='\0';
    }
    else
    {
        strcpy(name,file_name);
    }

}

static int  displace_char(char *dir)
{   
    char *pre;
    char str_result[60];
    while((pre=strchr(dir,','))!=0)
    {
        strncpy(str_result,dir,(pre-dir));
        str_result[pre-dir]='\0';
        strcat(str_result,".");
        strcat(str_result,pre+1);
        strcpy(dir,str_result);
    }
    return 1;
}

static void  make_cwd_comm( char *dir_filename,char *buf_dir)
{
    char *pev1; 
    char filename[128];
    char buf[128]={0};
	char tmpbuf[128]={0};
    int filelen = 0;
	int outlen = 0;
    get_file_name(dir_filename, filename);
    filelen = strlen(filename);
    if (filelen > 0)
    {
        pev1=strstr(dir_filename,filename);
        memset(tmpbuf,0,sizeof(tmpbuf));  
        strncpy(tmpbuf,dir_filename,(int)(pev1-dir_filename)); 
    }
    else
    {
        strcpy(tmpbuf, dir_filename);
    }
    strcpy(buf_dir,"CWD ");
	UTF_8ToGB2312(&outlen, buf, tmpbuf, strlen(tmpbuf), 128);
    strcat(buf_dir,buf);
    strcat(buf_dir,"\r\n"); 
}

static int check_bracket_count(char *str ,char str_2)  
{ 
    int count=0;
    int length;
    char check_char=str_2; 
    char *char_search_1;
    while ((char_search_1=strchr(str,check_char))!=NULL) 
    {
        length=(int)(char_search_1 - str);
        str=str+length+1;
        count=count+1;  }

    return count;

}

static int get_position_str(char *str ,char str_2,int num,char *str3)
{

    int num_1=0;
    char *str_prosition;
    char str_check[100];
    if(num==0)
    {
    }
    strcpy(str_check,str);
  
    while(num_1<num)
    {
        str_prosition=strchr(str_check, str_2);
        strcpy(str_check,str_prosition+1);
        num_1++;
    }
    strncpy(str3, str, (strlen(str)-strlen(str_check)) );
    str3[(strlen(str)-strlen(str_check))]='\0';  
    return 2;
}

int find_usbDev()
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

int FTP_SendCmd(T_FTP_CONNECTION_INFO *ftp, char *cmd)
{
    int write_pos = 0;
    int nLeft = 0;
    int clen = 0;
    clen = strlen(cmd);
    if (clen == 0)
    {
        return FTP_SEND_CMD_ERR;
    }

    nLeft = clen;
    int maxtime = 10*1000000;//10秒钟写不完就算失败了
    while (nLeft > 0)
    {
        int nWrite = 0;
        if ((nWrite = send (ftp->client_socket, cmd+write_pos, nLeft, MSG_NOSIGNAL)) <= 0)
        {
            if (errno == EWOULDBLOCK)
            {
                nWrite = 0;
                if (maxtime<= 0)
                    return FTP_ERR_ELSE;
                usleep(1000);
                maxtime -= 1000;
            }
            else 
            {
            	return FTP_ERR_ELSE; //表示写失败
            }
        }
        nLeft -= nWrite;
        write_pos += nWrite;
    }
    
    return 0;
}

int FTP_RecvResponse(T_FTP_CONNECTION_INFO *ftp, char *recvbuf, int buflen, char *c)
{
	pthread_mutex_lock(&ftp->tMutex);
    fd_set read_set;
    int err = 0;
    struct timeval time_current;
    time_current.tv_sec = 10;
    time_current.tv_usec = 0;
	
    if ((NULL == ftp) || (NULL == recvbuf) || (ftp->client_socket <= 0))
    {
		pthread_mutex_unlock(&ftp->tMutex);
    	return FTP_RECV_ERR;
    }
    
    FD_ZERO(&read_set);
    FD_SET(ftp->client_socket, &read_set);
    err = select(FD_SETSIZE, &read_set, 0, 0, &time_current);
    if (err > 0 && FD_ISSET(ftp->client_socket, &read_set))
    {
        memset(recvbuf,0,buflen);
        err=recv(ftp->client_socket,recvbuf,buflen,0);
        if (err >0)
        {
            recvbuf[err] = '\0';
            if(strstr(recvbuf,c)!=NULL) 
            {
				pthread_mutex_unlock(&ftp->tMutex);
                return 0;
            }   
            else if (strstr(recvbuf,"45")!=NULL) 
            {
				pthread_mutex_unlock(&ftp->tMutex);
                return FTP_RECV_45_ERR;
            }
            else
            {
				pthread_mutex_unlock(&ftp->tMutex);
                return FTP_RECV_ERR;
            }
        }
        else
        {
			pthread_mutex_unlock(&ftp->tMutex);
            return FTP_RECV_ERR;
        }
    }
    else
    {
		pthread_mutex_unlock(&ftp->tMutex);
        return FTP_RECV_ERR;
    }
}


int FTP_CmdCtrl(T_FTP_CONNECTION_INFO *ftp, char *cmdbuf,char *c)
{
    char recv_buf[1024] = {0};
    int err = 0;
    
    err = FTP_SendCmd(ftp, cmdbuf);
    if (err < 0)
    {
        return err;
    }

    err = FTP_RecvResponse(ftp,recv_buf, 1024, c);
    if (err < 0)
    {
        return err;
    }

    return 0;
}

int FTP_Explain227Cmd(char *buf_ip,char *pcStr_227)
{
    char *begin;
    char *end;
    char *pwd;
    char str[60],str2[60];
    char str_ip2[60];
    char str_point[60],str_point_l[10],str_point_h[10];
    int  i=1;
    int buf_point;  
    begin=strstr(pcStr_227,"(");
    end=strstr(pcStr_227,")"); 
    strncpy(str,begin+1,(int)(end-begin));
    str[end-begin-1]='\0';  
    strcpy(str2,str);
    while((pwd=strstr(str2,","))!=0)
    {
        if(i==4)
        {   
            strcpy(str_point,pwd+1);  //解析得到的端口号
            break;
        }
        strcpy(str2,pwd+1);
        ++i;
    }
    pwd=strstr(str,str_point);    
    strncpy(str_ip2,str,(int)(pwd-str-1));
    str_ip2[pwd-str-1]='\0';                //得到的ip
    displace_char(str_ip2);
    strcpy(buf_ip,str_ip2);
    pwd=strstr(str_point,",");  
    strncpy(str_point_l,str_point,(int)(pwd-str_point));
    str_point_l[pwd-str_point]='\0';  //高位
    strcpy(str_point_h,pwd+1);        //低位
    buf_point=atoi(str_point_l)*256+atoi(str_point_h);
    return buf_point;
}

int FTP_CloseClientDataSocket(T_FTP_CONNECTION_INFO *ftp)
{
    if ((ftp != NULL)&&(ftp->data_socket > 0))
    {
        shutdown(ftp->data_socket,0x00);
        close(ftp->data_socket);
        ftp->data_socket= 0;
    }
    return 0;   
}

int FTP_CloseClientCmdSocket(T_FTP_CONNECTION_INFO *ftp)
{
    if ((ftp != NULL)&&(ftp->client_socket > 0))
    {
        shutdown(ftp->client_socket,0x00);
        close(ftp->client_socket);
        ftp->client_socket = 0;
    }
    return 0;   
}

int FTP_CreateClientDataSocket(void)
{
    int iSockFd;
    struct timeval sendTimeOut={5,0}, recvTimeOut = {5, 0};
    int on = 1;
    
    iSockFd =socket(AF_INET, SOCK_STREAM, 0);    
    if(iSockFd == INVALID_SOCKET)
    {
        return FTP_ERR_ELSE;  
    }   
    else 
    {   
        setsockopt(iSockFd, SOL_SOCKET, SO_SNDTIMEO, &sendTimeOut, sizeof(sendTimeOut));
        setsockopt(iSockFd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeOut, sizeof(recvTimeOut));
        setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        return iSockFd;
    }
}

int FTP_CreateClientCmdSocket(void)
{
    int iSockFd;
    struct timeval sendTimeOut={1,0}, recvTimeOut = {1, 0};
    int on = 1;
    
    iSockFd =socket(AF_INET, SOCK_STREAM, 0);  
    if(iSockFd == INVALID_SOCKET)
    {
        return FTP_ERR_ELSE;  
    }   
    else 
    {   
        setsockopt(iSockFd, SOL_SOCKET, SO_SNDTIMEO, &sendTimeOut, sizeof(sendTimeOut));
        setsockopt(iSockFd, SOL_SOCKET, SO_RCVTIMEO, &recvTimeOut, sizeof(recvTimeOut));
        setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        return iSockFd;
    }
}

int FTP_SetStartOffset(T_FTP_CONNECTION_INFO *ftp)
{
    char acBuf[128] = {0};
    char acRecvBuf[1024] = {0};
    int iRet = 0;

    sprintf(acBuf, "REST %llu\r\n", ftp->uploadFileOffset);
    iRet = FTP_SendCmd(ftp, acBuf);
    if (iRet < 0)
    {
        return iRet;
    }

    iRet = FTP_RecvResponse(ftp,acRecvBuf, 1024, "350");
    if (iRet < 0)
    {
        if(strstr(acRecvBuf,"150")==NULL) 
        {
            return iRet;
        }   
    }
    
    return 0;
}

int FTP_SetTransferType(T_FTP_CONNECTION_INFO *ftp)
{
    char acBuf[128] = {0};
    char cType;
    int iRet = 0;
    
    switch (ftp->iUploadType) 
    {
        case KTYPE_BINARY:
            cType = 'I';
            break;
        case KTYPE_ASCII:
            cType = 'A';
            break;
        case KTYPE_EBCDIC:
            cType = 'E';
            break;
        default:
            return FTP_UNSUPORT_TYPE;
    }

    sprintf(acBuf, "TYPE %c\r\n", cType);      
    iRet = FTP_CmdCtrl(ftp, acBuf, "200");
    if (iRet < 0)
    {
        return iRet;
    }

    return 0;
}

int FTP_OpenDataConntion(T_FTP_CONNECTION_INFO *ptFtpConnectionInfo)
{
    char acBuf[128] = {0};
    char acRecvBuf[1024] = {0};
    char acTpStr[60];
    unsigned short port;
    struct sockaddr_in data_client_ip;  
    
    if (PASV_MODE == ptFtpConnectionInfo->iMode)//被动式
    {
        sprintf(acBuf,"PASV\r\n");        
        if (FTP_SendCmd(ptFtpConnectionInfo, acBuf) < 0)
        {
            return -1;
        }
    
        if (FTP_RecvResponse(ptFtpConnectionInfo,acRecvBuf, 1024, "227") < 0)
        {
            return -1;
        }

        memset(acTpStr,0,sizeof(acTpStr));
        port=FTP_Explain227Cmd(acTpStr,acRecvBuf);                           //得到端口和IP     
        memset(&data_client_ip, 0, sizeof(struct sockaddr_in)); 
    	
        data_client_ip.sin_family = AF_INET;                          
        data_client_ip.sin_port =htons(port);                      
        data_client_ip.sin_addr.s_addr =inet_addr(acTpStr); 

        if(ptFtpConnectionInfo->data_socket > 0)
	    {
	    	FTP_CloseClientDataSocket(ptFtpConnectionInfo);
	    }
        ptFtpConnectionInfo->data_socket=FTP_CreateClientDataSocket();
        
        if (connect(ptFtpConnectionInfo->data_socket, (struct sockaddr*)&data_client_ip, sizeof(struct sockaddr_in)) < 0) 
        {
            DebugPrint(DEBUG_FTP_PRINT, "\n ftp_chdir 227 data_link not link to ftp \n");
            return FTP_OPEN_DATA_ERR;
        }
    }
    else
    {
    }

    return 0;
}

int FTP_Quit(T_FTP_CONNECTION_INFO *ftp)
{
	int err = 0;
    char buf[256] = {0};
    char recv_buf[1024] = {0};
    
    sprintf(buf, "QUIT\r\n");   
    if ((err = FTP_SendCmd(ftp, buf)) < 0)
    {
        return err;
    }

    if ((err = FTP_RecvResponse(ftp,recv_buf, 1024, "221")) < 0)
    {
        return err;
    }

    return 0;
}

int FTP_ServerLogin(T_FTP_CONNECTION_INFO *ftp, int addr, int port, char *uname, char *pword)
{
    int err = 0; 
    int len = 0;
    char buf[128];
    char recv_buf[1024];
    char username[128];
	char tmpusername[128];
    char userpwd[128];
	int outlen = 0; 
    struct sockaddr_in server;
    
	
    memset(&server, 0, sizeof(struct sockaddr_in));              //先将保存地址的server置为全0
    server.sin_family = AF_INET;                          //声明地址格式是TCP/IP地址格式
    server.sin_port = htons(port);                          //指明连接服务器的端口号，htons()用于 converts values  //  between the host and network byte order
    server.sin_addr.s_addr = htonl(addr);//inet_addr(addr);
    
    err = connect(ftp->client_socket, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
    if (err < 0)
    {
        if (EINPROGRESS != errno)
        {
            DebugPrint(DEBUG_FTP_PRINT, "ftp_open not link to ftp server(0x%x),port=0x%x(%d), errno=%d(%s)\n", 
				server.sin_addr.s_addr, server.sin_port, port, errno, strerror(errno));
            return FTP_CONNECT_SEVER_ERR;
        }
    }
	memset(recv_buf, 0, sizeof(recv_buf));
    if (FTP_RecvResponse(ftp,recv_buf, 1024, "220") < 0)
    {
        DebugPrint(DEBUG_FTP_PRINT, "ftp_open FTP_RecvResponse not link to ftp server(0x%x),port=0x%x(%d), errno=%d(%s)\n", server.sin_addr.s_addr, server.sin_port, port, errno, strerror(errno));
        return FTP_CONNECT_SEVER_ERR;
    }

    memset(buf, 0, 128);
    len = strlen(uname);
    if (len > 0)
    {
        //strcpy(username, uname);
        strcpy(tmpusername, uname);
        strcpy(userpwd, pword);
    }
    else
    {
        //strcpy(username, "anonymous");
        strcpy(tmpusername, uname);
        strcpy(userpwd, "anon");
    }

	UTF_8ToGB2312(&outlen, username, tmpusername, strlen(tmpusername), 128);
    //if ((strlen(username)==9)&&(strncasecmp(username, "anonymous", 9) == 0) && (strlen(userpwd) == 0))
    if ((outlen==9)&&(strncasecmp(username, "anonymous", 9) == 0) && (strlen(userpwd) == 0))
        strcpy(userpwd, "123456");
    sprintf(buf, "USER %s\r\n", username);
    err = FTP_SendCmd(ftp, buf);
    if (err < 0)
    {
        FTP_Quit(ftp);
        return FTP_SEND_USER_ERR;
    }
    
	memset(recv_buf, 0, sizeof(recv_buf));
    err = FTP_RecvResponse(ftp,recv_buf, 1024, "331");
    if (err < 0)
    {
        if(strstr(recv_buf,"230")!=0)
        {   
            return 0;     
        }
        else
        {   
            FTP_Quit(ftp);
            return FTP_SEND_USER_ERR;
        }
    }

    memset(buf, 0, 128);
    sprintf(buf, "PASS %s\r\n", userpwd);
    if (FTP_CmdCtrl(ftp, buf,"230") < 0)
    {
        DebugPrint(DEBUG_FTP_PRINT, "ftp_open send pwd err, usr=%s, errno=%d(%s)\n", userpwd, errno, strerror(errno));
        FTP_Quit(ftp);
        return FTP_USR_PWD_ERR;
    }
    
    return 0;
}

int FTP_GetFileSize(T_FTP_CONNECTION_INFO *ftp, char *filepath, unsigned long long *filesize)
{
    char buf[256] = {0};
	char sendfilepath[256] = {0};
    char recv_buf[1024] = {0};
    int err;
	int outlen = 0;
	outlen = 0;
    UTF_8ToGB2312(&outlen, sendfilepath, filepath, strlen(filepath), 256);
    sprintf(buf, "SIZE %s\r\n", sendfilepath);
    *filesize = 0;
    err = FTP_SendCmd(ftp, buf);
    if (err < 0)
    {
        return err;
    }

    err = FTP_RecvResponse(ftp,recv_buf, 1024, "213");
    if (err < 0)
    {
        return err;
    }

    char *pev1;
    char size[128] = {0};
    pev1=strstr(recv_buf,"213 ");
    if (pev1 != NULL)
    {
        strncpy(size,pev1+4,strlen(recv_buf)-6);
        size[strlen(recv_buf)-6]='\0';
        *filesize = strtoul(size, NULL, 10);

    }
    
    return 0;
}

int FTP_CheckdirAndMakedir(T_FTP_CONNECTION_INFO *ftp,  char *dname)
{
    int  err1;
    char recv_buf[100]; //接受缓冲
    char send_buf[100]; //发送缓冲
    char gb2312send_buf[100]; //发送缓冲
    char check_buf[256]; //从dname获取的路径
    char sendcheck_buf[256];
	int outlen = 0;
    char cwd_str[256];
    char str_mkd_1[100];
    char str_mkd_2[100];
    char str[100]="NOOP \r\n";
    char path[256] = {0};
    int num=1,count=0;
    int err = 0;

    if (dname == NULL)
    {
    	return FTP_ERR_ELSE;
    }
    
    char *tmp = dname;
    if (*tmp != '/')
    {
        strcpy(path, "/");
        strcat(path, dname);
    }
    else
    {
        strcpy(path, dname);
    }

    make_cwd_comm(path,cwd_str);//获取路径
    err = FTP_CmdCtrl(ftp, cwd_str,"250");
    if (err == 0)
    {
        DebugPrint(DEBUG_FTP_PRINT, "ftp_chdir_and_makedir_c 250 dir(%s) exist\n", cwd_str);
        return 0;//路径已存在
    }
    //路径不存在需要创建不存在的目录

    count=check_bracket_count(path,'/');
    for(num=0; num<=count ; num++)
    {   
        get_position_str(path , '/', num, check_buf);
        //组织cmd命令
        strcpy(str_mkd_1,check_buf);
        strcpy(send_buf,"CWD ");
		outlen = 0;
        UTF_8ToGB2312(&outlen, sendcheck_buf, check_buf, strlen(check_buf), 256);
        strcat(send_buf,sendcheck_buf);
        strcat(send_buf,"\r\n");
        if(send (ftp->client_socket, send_buf, strlen(send_buf), MSG_NOSIGNAL) <0)  //发送CWD命令目录
        {
            return FTP_SEND_CMD_ERR;
        }
        err1=recv(ftp->client_socket,recv_buf,sizeof(recv_buf),0);
        recv_buf[err1]='\0';

        if((strstr(recv_buf,"550")!=NULL)||((strstr(recv_buf,"450")!=NULL)))
        {
            //发送cwd命令
            get_position_str(path , '/', num-1, check_buf);
            strcpy(send_buf,"CWD ");
			outlen = 0;
        	UTF_8ToGB2312(&outlen, sendcheck_buf, check_buf, strlen(check_buf), 256);
            strcat(send_buf,sendcheck_buf);
            strcat(send_buf,"\r\n");
            if(send (ftp->client_socket, send_buf, strlen(send_buf), MSG_NOSIGNAL) <0)  //发送CWD命令目录
            {
                return FTP_SEND_CMD_ERR;
            }
            err1=recv(ftp->client_socket,recv_buf,sizeof(recv_buf),0);//接受cwd的命令回复
            recv_buf[err1]='\0';
            
 
              //创建路径　
            get_position_str(path , '/', num-1, str_mkd_2); 
            strcpy(send_buf,(str_mkd_1+strlen(str_mkd_2)));
            send_buf[strlen(send_buf)-1]='\0';
            strcpy(str_mkd_2,"MKD ");
			outlen = 0;
       		UTF_8ToGB2312(&outlen, gb2312send_buf, send_buf, strlen(send_buf), 100);
            strcat(str_mkd_2,gb2312send_buf);
            strcat(str_mkd_2,"\r\n");
            if(send (ftp->client_socket, str_mkd_2, strlen(str_mkd_2), MSG_NOSIGNAL) <0)  //发送CWD命令目录
            {
                return FTP_SEND_CMD_ERR;
            } 
            err1=recv(ftp->client_socket,recv_buf,sizeof(recv_buf),0);
            recv_buf[err1]='\0';
 
        }
        else
        if(strstr(recv_buf,"521")!=NULL)
        {
            send(ftp->client_socket, str, strlen(str), MSG_NOSIGNAL);   
        }
    }
    return 0;   
}

void *FTP_DownloadDataRecvThread(void *param)   
{
	int iRet = 0, trytimes = 0;
	int iPos = 0, iRecvLen = 0, iRecvSize = 0;
	unsigned long long iFileSize = 0;
	char acLocalDatabuf[4096] = {0};
	
    char buf[256] = {0};
    char recv_buf[1024] = {0};
	char sendfilename[256] = {0};
	int outlen = 0;
    char ip_str[60];
    int  point;
    struct sockaddr_in  data_client_ip;
    fd_set read_set;
    struct timeval time_current;
    FILE *fp = NULL;
    T_FTP_PACKET tFtpPkt;
    
    PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)param;
	if (NULL == ptFtpConnectionInfo)
	{
		return NULL;
	}

    while (1 == ptFtpConnectionInfo->threadRunFlag)
    {		
    	if (0 == find_usbDev())
		{
			if (0 == access(USB_PATH, F_OK))
			{
				system("rm -r" USB_PATH);
			}
			iPos = -1;  //暂定回调进度-1，表示告知U盘已拔出
			goto FAIL;
		}
		
    	iRet = FTP_GetNodeFromWorkQueue(ptFtpConnectionInfo->ptQueue, &tFtpPkt);
	    if (iRet > 0)
	    {
	    	if (tFtpPkt.iCmdType != FTP_DOWNLOAD_CMD)
	    	{
	    		iPos = -2;  
				goto FAIL;	
	    		continue;
	    	}
	    	
	    	if (strlen(tFtpPkt.acDstFileName) != 0)
			{
			    fp = fopen(tFtpPkt.acDstFileName, "wb");
				if (NULL == fp)
				{
					iPos = -2;  
					goto FAIL;	
				}
			}
			else
			{
				iPos = -2;  
				goto FAIL;	
			}

			//传输模式
		    memset(buf,0,sizeof(buf));
		    strcpy(buf,"TYPE I\r\n");  
		    iRet = FTP_CmdCtrl(ptFtpConnectionInfo, buf, "200");
		    if (iRet < 0)
		    {
		    	fclose(fp);
		    	fp = NULL;
		    	iPos = -3;  //暂定回调进度-3，表示告知数据接收失败
		        goto FAIL;
		    }
		    
		    //被动模式
		    memset(buf,0,sizeof(buf));
		    strcpy(buf,"PASV\r\n");
		    if ((iRet = FTP_SendCmd(ptFtpConnectionInfo, buf)) < 0)
		    {
		    	fclose(fp);
		    	fp = NULL;
		        iPos = -3;  //暂定回调进度-3，表示告知数据接收失败
		        goto FAIL;
		    }

			memset(recv_buf, 0, sizeof(recv_buf));
		    if ((iRet = FTP_RecvResponse(ptFtpConnectionInfo,recv_buf, 1024, "227")) < 0)
		    {
		    	fclose(fp);
		    	fp = NULL;
		        iPos = -3;  //暂定回调进度-3，表示告知数据接收失败
		        goto FAIL;
		    }
		    memset(ip_str,0,sizeof(ip_str));
		    point=FTP_Explain227Cmd(ip_str,recv_buf);    //得到端口和IP
		    memset(&data_client_ip, 0, sizeof(struct sockaddr_in));              //先将保存地址的server置为全0
		    //声明地址格式是TCP/IP地址格式
		    data_client_ip.sin_family = AF_INET;                          //声明地址格式是TCP/IP地址格式
		    data_client_ip.sin_port =htons(point);                        //指明连接服务器的端口号，htons()用于 converts values  //  between the host and network byte order
		    data_client_ip.sin_addr.s_addr =inet_addr(ip_str); 
		    
		    if(ptFtpConnectionInfo->data_socket > 0)
		    {
		    	FD_CLR(ptFtpConnectionInfo->data_socket, &ptFtpConnectionInfo->readSet);
		    	FTP_CloseClientDataSocket(ptFtpConnectionInfo);
		    }
	        ptFtpConnectionInfo->data_socket=FTP_CreateClientDataSocket();
	        
		    if (connect(ptFtpConnectionInfo->data_socket, (struct sockaddr*)&data_client_ip, sizeof(struct sockaddr_in)) <0) 
		    {
		        DebugPrint(DEBUG_FTP_PRINT, "\ndata_link not link to ftp \n");
		        fclose(fp);
		    	fp = NULL;
		        iPos = -3;  //暂定回调进度-3，表示告知数据接收失败
		        goto FAIL;
		    }

			FD_SET(ptFtpConnectionInfo->data_socket, &ptFtpConnectionInfo->readSet);
			
		    memset(buf,0,sizeof(buf));
			outlen = 0;
		    UTF_8ToGB2312(&outlen, sendfilename, tFtpPkt.acSrcFileName, strlen(tFtpPkt.acSrcFileName), 256);
		    sprintf(buf, "RETR %s\r\n", sendfilename);
		    if ((iRet = FTP_SendCmd(ptFtpConnectionInfo, buf)) < 0)
		    {
		        DebugPrint(DEBUG_FTP_PRINT, "FTP_SendCmd err\n");
		        fclose(fp);
		    	fp = NULL;
		        iPos = -3;  //暂定回调进度-3，表示告知数据接收失败
		        goto FAIL;
		    }
		    
			memset(recv_buf, 0, sizeof(recv_buf));
		    if ((iRet = FTP_RecvResponse(ptFtpConnectionInfo,recv_buf, 1024, "150")) < 0)
		    {
		        DebugPrint(DEBUG_FTP_PRINT, "FTP_RecvResponse(RETR) err, iRet=%d\n",iRet);
		        fclose(fp);
		    	fp = NULL;
		        iPos = -3;  //暂定回调进度-3，表示告知数据接收失败
		        goto FAIL;
		    }

			while (1 == ptFtpConnectionInfo->threadRunFlag)
			{	
				if (0 == find_usbDev())
				{
					if (0 == access(USB_PATH, F_OK))
					{
						system("rm -r" USB_PATH);
					}
					iPos = -1;  //暂定回调进度-1，表示告知U盘已拔出
					break;
				}
				
				read_set = ptFtpConnectionInfo->readSet;
				time_current = ptFtpConnectionInfo->selectTime;
				
				iRet = select(FD_SETSIZE, &read_set, 0, 0, &time_current);
			    if (iRet > 0 && FD_ISSET(ptFtpConnectionInfo->data_socket, &read_set))
			    {
			        trytimes = 0;
			        memset(acLocalDatabuf, 0, sizeof(acLocalDatabuf));
					iRecvSize = ((ptFtpConnectionInfo->DownloadFileSize - iFileSize)>=sizeof(acLocalDatabuf))?sizeof(acLocalDatabuf):(ptFtpConnectionInfo->DownloadFileSize - iFileSize);
			        iRecvLen = recv(ptFtpConnectionInfo->data_socket, acLocalDatabuf, iRecvSize, 0);

					if (iRecvLen > 0)
			        {
			        	if (fp != NULL)
			        	{
			                iRet = fwrite(acLocalDatabuf, 1, iRecvLen, fp);
			                if (iRet <= 0)
			                {
			                	if (1 == find_usbDev())  
								{
									iPos = -2;  //暂定回调进度-2，表示告知U盘写入失败
								}
								else
								{
									if (0 == access(USB_PATH, F_OK))
									{
										system("rm -r" USB_PATH);
									}
									iPos = -1;  //暂定回调进度-1，表示告知U盘已拔出
								}
			                	break;
			                }

			                iFileSize += iRet;
			                if (ptFtpConnectionInfo->DownloadFileSize > 0)
			                {
								iPos = iFileSize*100/ptFtpConnectionInfo->DownloadFileSize;
								if (iPos >= 0)
								{	     
								    //DebugPrint(DEBUG_FTP_PRINT, "LINE:%d iPos =%d\n", __LINE__,iPos);
									if (iPos == 100)
									{
						        		break;
						        	} 
						        	else
						        	{
										ptFtpConnectionInfo->pFtpProcFunc((PFTP_HANDLE)ptFtpConnectionInfo, iPos);
						        	}
				                }
			                } 
			            }
			        }
					else if (0 == iRecvLen)
					{		
						T_LOG_INFO tLog;
                        
									   
						memset(&tLog,0,sizeof(T_LOG_INFO));
						memset(recv_buf, 0, sizeof(recv_buf));
						if ((iRet = FTP_RecvResponse(ptFtpConnectionInfo, recv_buf, 1024, "226")) < 0)
					    {
					        DebugPrint(DEBUG_FTP_PRINT, "recv_buf err, buf=%s, ptFtpConnectionInfo->data_socket=%d\n", recv_buf,ptFtpConnectionInfo->data_socket);
					    }
						if(ptFtpConnectionInfo->data_socket > 0)
					    {
					    	FD_CLR(ptFtpConnectionInfo->data_socket, &ptFtpConnectionInfo->readSet);
					    	FTP_CloseClientDataSocket(ptFtpConnectionInfo);
					    }

						tLog.iLogType = LOG_TYPE_EVENT;	
						snprintf(tLog.acLogDesc,sizeof(tLog.acLogDesc)-1,"down succ:%s"
							,tFtpPkt.acSrcFileName);
						LOG_WriteLog(&tLog);
						break;
					}
			        else
			        {
			            DebugPrint(DEBUG_FTP_PRINT, "recv err,iRecvLen=%d\n",iRecvLen);
			            iPos = -3; //暂定回调进度-3，表示告知数据接收失败
			            break;
			        }
			    }
			    else
			    {
			        if ((trytimes ++) > 3)
			        {
			            DebugPrint(DEBUG_FTP_PRINT, "select err, trytimes=%d\n", trytimes);
			            iPos = -3;  //暂定回调进度-3，表示告知数据接收失败
			            break;
			        }
			        else
			        {
			            DebugPrint(DEBUG_FTP_PRINT, "select err, errno=0x%x, err=%s trytimes=%d, sleep(1)\n", errno, strerror(errno), trytimes);
			            continue;
			        }
			    }
			}
			fflush(fp);
			fsync(fileno(fp));
			fclose(fp);
		    fp = NULL;
	    }
		
	FAIL:
		if ((100 == iPos) || (-1 == iPos) || (-2 == iPos) || (-3 == iPos))
		{
			ptFtpConnectionInfo->pFtpProcFunc((PFTP_HANDLE)ptFtpConnectionInfo, iPos);
			break;
		}
	    usleep(50*1000);
    }
    
    return NULL;
}

void *FTP_UploadDataSendThread(void *param)           
{
	int iRet = 0, iValue = 0;
    char acBuf[256] = {0};
	int iOutlen = 0, iReadLen = 0;
	unsigned long long llFileSize = 0;
	char acSendFilePath[256] = {0};
	char acLocalDatabuf[4096] = {0};
	FILE *fp = NULL;
    T_FTP_PACKET tFtpPkt;
    
	PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)param;
	if (NULL == ptFtpConnectionInfo)
	{
		return NULL;
	}

	while (1 == ptFtpConnectionInfo->threadRunFlag && llFileSize < ptFtpConnectionInfo->uploadLocalFileSize)
    {
		iRet = FTP_GetNodeFromWorkQueue(ptFtpConnectionInfo->ptQueue, &tFtpPkt);
	    if (iRet > 0)
	    {
	    	if (tFtpPkt.iCmdType != FTP_UPLOAD_CMD)
	    	{
	    		continue;
	    	}

	    	if (strlen(tFtpPkt.acSrcFileName) != 0)
			{
			    fp = fopen(tFtpPkt.acSrcFileName, "rb");
				if (NULL == fp)
				{
					continue;
				}
			}
			else
			{
				continue;
			}

	    	iRet = FTP_SetTransferType(ptFtpConnectionInfo);
		    if (iRet < 0)
		    {
		    	fclose(fp);
	    		fp = NULL;
		        iValue = 0;
		        goto END;
		    }

		    iRet = FTP_OpenDataConntion(ptFtpConnectionInfo);
		    if (iRet < 0)
		    {
		        fclose(fp);
	    		fp = NULL;
		        iValue = 0;
		        goto END;
		    }
		    
		    if (ptFtpConnectionInfo->uploadFileOffset > 0)
		    {
		        iRet = FTP_SetStartOffset(ptFtpConnectionInfo);
		        if (iRet < 0)
		        {
					fclose(fp);
	    			fp = NULL;
		        	iValue = 0;
		        	goto END;
		        }
		    }

		    UTF_8ToGB2312(&iOutlen, acSendFilePath, tFtpPkt.acDstFileName, strlen(tFtpPkt.acDstFileName), 256);
		    sprintf(acBuf, "STOR %s\r\n", acSendFilePath); 
		    
		    iRet = FTP_CmdCtrl(ptFtpConnectionInfo, acBuf, "150");
		    if (iRet < 0)
		    {
		        fclose(fp);
	    		fp = NULL;
		        iValue = 0;
		        goto END;
		    }
		    
			//usleep(2*1000*1000);
			
		    while (1 == ptFtpConnectionInfo->threadRunFlag)
			{
				memset(acLocalDatabuf, 0, sizeof(acLocalDatabuf));
				iReadLen = fread(acLocalDatabuf, 1, sizeof(acLocalDatabuf), fp);
				if (iReadLen <= 0)
				{
					break;
				}
				llFileSize += iReadLen;

				iRet = send(ptFtpConnectionInfo->data_socket, acLocalDatabuf, iReadLen, 0);
				if (iRet <= 0)
				{
					break;
				}
			}
			fclose(fp);
    		fp = NULL;
	    }

		if (100 == (llFileSize*100/ptFtpConnectionInfo->uploadLocalFileSize))
		{
			iValue = 1;	
			if(ptFtpConnectionInfo->data_socket > 0)
		    {
		    	FTP_CloseClientDataSocket(ptFtpConnectionInfo);
		    }
			memset(acBuf, 0, sizeof(acBuf));
			FTP_RecvResponse(ptFtpConnectionInfo, acBuf, sizeof(acBuf), "226");
		}
		
    	usleep(100*1000);
    }

END:
	ptFtpConnectionInfo->pFtpProcFunc((PFTP_HANDLE)ptFtpConnectionInfo, iValue);
        	
    return NULL;
}

PFTP_HANDLE FTP_CreateConnect(char *pcIpAddr, int iPort, PF_FTP_PROC_CALLBACK pFtpProcFunc)
{
	int iRet = 0;
	PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = NULL;

	if (NULL == pcIpAddr || 0 == iPort)
	{
		return 0;
	}

	ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)malloc(sizeof(T_FTP_CONNECTION_INFO));
	if (NULL == ptFtpConnectionInfo)
    {
        return 0;	
    }
    memset(ptFtpConnectionInfo, 0, sizeof(T_FTP_CONNECTION_INFO));

    iRet = FTP_CreateClientCmdSocket();
    if (iRet < 0)
    {
    	free(ptFtpConnectionInfo);
        ptFtpConnectionInfo = NULL;
        return 0;
    }
    ptFtpConnectionInfo->client_socket = iRet;

	/*
    iRet = FTP_CreateClientDataSocket();
    if (iRet < 0)
    {
    	free(ptFtpConnectionInfo);
        ptFtpConnectionInfo = NULL;
        return 0;
    }  
    ptFtpConnectionInfo->data_socket = iRet;*/
    
    ptFtpConnectionInfo->pFtpProcFunc = pFtpProcFunc;
    ptFtpConnectionInfo->DownloadFileSize = 0;
    iRet = FTP_ServerLogin(ptFtpConnectionInfo, htonl(inet_addr(pcIpAddr)), iPort, "root", "hq123");   //匿名方式登录服务器
	if (iRet < 0)
	{
		free(ptFtpConnectionInfo);
    	ptFtpConnectionInfo = NULL;
    	return 0;
	}

	FD_ZERO(&ptFtpConnectionInfo->readSet);
	ptFtpConnectionInfo->selectTime.tv_sec = 0;
	ptFtpConnectionInfo->selectTime.tv_usec = 500000;

	pthread_mutex_init(&ptFtpConnectionInfo->tMutex, NULL);

	pthread_mutex_init(&ptFtpConnectionInfo->tQueueMutex, NULL);
    ptFtpConnectionInfo->ptQueue = FTP_CreateWorkQueue(&ptFtpConnectionInfo->tQueueMutex, 0);

    return (PFTP_HANDLE)ptFtpConnectionInfo;
}

int FTP_DestoryConnect(PFTP_HANDLE pFtpHandle)
{
    PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)pFtpHandle;
    if (NULL == ptFtpConnectionInfo)
    {
        return -1;	
    }

    if (ptFtpConnectionInfo->ThreadId != 0)
    {
		ptFtpConnectionInfo->threadRunFlag = 0;		
        pthread_join(ptFtpConnectionInfo->ThreadId, NULL);
        ptFtpConnectionInfo->ThreadId = 0;
    }
    
    pthread_mutex_destroy(&ptFtpConnectionInfo->tMutex);
	FTP_DestroyWorkQueue(ptFtpConnectionInfo->ptQueue);
    pthread_mutex_destroy(&ptFtpConnectionInfo->tQueueMutex);
    
    FTP_Quit(ptFtpConnectionInfo);

    FTP_CloseClientCmdSocket(ptFtpConnectionInfo);
    FTP_CloseClientDataSocket(ptFtpConnectionInfo);
    
    free(ptFtpConnectionInfo);
    ptFtpConnectionInfo = NULL;
    return 0;
}

int FTP_DestoryConnect2(PFTP_HANDLE pFtpHandle)
{
    PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)pFtpHandle;
    if (NULL == ptFtpConnectionInfo)
    {
        return -1;	
    }

    if (ptFtpConnectionInfo->ThreadId != 0)
    {
		ptFtpConnectionInfo->threadRunFlag = 0;
        pthread_join(ptFtpConnectionInfo->ThreadId, NULL);
        ptFtpConnectionInfo->ThreadId = 0;
    }
    
    pthread_mutex_destroy(&ptFtpConnectionInfo->tMutex);
	FTP_DestroyWorkQueue(ptFtpConnectionInfo->ptQueue);
    pthread_mutex_destroy(&ptFtpConnectionInfo->tQueueMutex);

    FTP_CloseClientCmdSocket(ptFtpConnectionInfo);
    FTP_CloseClientDataSocket(ptFtpConnectionInfo);
    
    free(ptFtpConnectionInfo);
    ptFtpConnectionInfo = NULL;
    return 0;
}


int FTP_AddDownLoadFile(PFTP_HANDLE pFtpHandle, char *pcSrcFileName, char *pcDstFileName)
{
	unsigned long long fileSize = 0;
	T_FTP_PACKET tPkt;
	PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)pFtpHandle;

    if (NULL == ptFtpConnectionInfo || NULL == pcSrcFileName || NULL == pcDstFileName)
    {
    	return FTP_ERR_ELSE;
    }

	FTP_GetFileSize(ptFtpConnectionInfo, pcSrcFileName, &fileSize); 

	ptFtpConnectionInfo->DownloadFileSize += fileSize;

    memset(tPkt.acSrcFileName, 0, sizeof(tPkt.acSrcFileName));
    memcpy(tPkt.acSrcFileName, pcSrcFileName, strlen(pcSrcFileName));
	memset(tPkt.acDstFileName, 0, sizeof(tPkt.acDstFileName));
    memcpy(tPkt.acDstFileName, pcDstFileName, strlen(pcDstFileName));
    tPkt.iCmdType = FTP_DOWNLOAD_CMD;
    FTP_PutNodeToWorkQueue(ptFtpConnectionInfo->ptQueue, &tPkt);
    return 0;
}

int FTP_FileDownLoad(PFTP_HANDLE pFtpHandle)
{
	int iRet = 0;
	PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)pFtpHandle;

    if (NULL == ptFtpConnectionInfo)
    {
    	return FTP_ERR_ELSE;
    }

	ptFtpConnectionInfo->ThreadId = 0;
	ptFtpConnectionInfo->threadRunFlag = 1;
    iRet = pthread_create(&ptFtpConnectionInfo->ThreadId, NULL, FTP_DownloadDataRecvThread, (void *)ptFtpConnectionInfo);
    if (iRet < 0)
    {
        return FTP_ERR_ELSE;
    }
	
    return 0;
}

int FTP_AddUpLoadFile(PFTP_HANDLE pFtpHandle, char *pcSrcFileName, char *pcDstFileName, int iType, int iMode)
{
	long lFileSize = 0;
	unsigned long long fileSize = 0;
	FILE *fpHandle = NULL;
	T_FTP_PACKET tPkt;
	PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)pFtpHandle;

    if (NULL == ptFtpConnectionInfo || NULL == pcSrcFileName || NULL == pcDstFileName)
    {
    	return FTP_ERR_ELSE;
    }

    if (FTP_CheckdirAndMakedir(ptFtpConnectionInfo, pcDstFileName) < 0)
    {
    	return FTP_ERR_ELSE;
    }

    fpHandle = fopen(pcSrcFileName, "rb");
    if (NULL == fpHandle)
    {
    	return FTP_ERR_ELSE;
    }
    fseek(fpHandle, 0L, SEEK_END);
	lFileSize = ftell(fpHandle);
	ptFtpConnectionInfo->uploadLocalFileSize += lFileSize;
	fseek(fpHandle, 0L, SEEK_SET);
	fclose(fpHandle);
	
	FTP_GetFileSize(ptFtpConnectionInfo, pcDstFileName, &fileSize);

    ptFtpConnectionInfo->iUploadType = iType;
	ptFtpConnectionInfo->iMode = iMode;
	ptFtpConnectionInfo->uploadFileOffset = fileSize;

    memset(tPkt.acSrcFileName, 0, sizeof(tPkt.acSrcFileName));
    memcpy(tPkt.acSrcFileName, pcSrcFileName, strlen(pcSrcFileName));
	memset(tPkt.acDstFileName, 0, sizeof(tPkt.acDstFileName));
    memcpy(tPkt.acDstFileName, pcDstFileName, strlen(pcDstFileName));
    tPkt.iCmdType = FTP_UPLOAD_CMD;
    FTP_PutNodeToWorkQueue(ptFtpConnectionInfo->ptQueue, &tPkt);

    return 0;
}

int FTP_FileUpLoad(PFTP_HANDLE pFtpHandle)  
{   
	int iRet = 0;
	PT_FTP_CONNECTION_INFO ptFtpConnectionInfo = (PT_FTP_CONNECTION_INFO)pFtpHandle;

    if (NULL == ptFtpConnectionInfo)
    {
    	return FTP_ERR_ELSE;
    }

    ptFtpConnectionInfo->threadRunFlag = 1;
    iRet = pthread_create(&ptFtpConnectionInfo->ThreadId, NULL, FTP_UploadDataSendThread, (void *)ptFtpConnectionInfo);
    if (iRet < 0)
    {
        return FTP_ERR_ELSE;
    }

    return 0;
}

