#ifndef _FTP_API_H_
#define _FTP_API_H_

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define SOCKET	int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define WSAGetLastError() errno
#define BOOL	unsigned int
#define MAX_FILENAME_LEN 128
#define USB_PATH	"/mnt/usb/"

typedef unsigned long PFTP_HANDLE;
typedef void (*PF_FTP_PROC_CALLBACK)(PFTP_HANDLE pFtpHandle, int iPos);

typedef enum {
	FTP_OK = 0,//成功
	FTP_NETOWR_ERR = -1,//网络错误
	FTP_CONNECT_SEVER_ERR = -2,//连接服务器错误
	FTP_RECV_ERR = -3,//接收错误
	FTP_RECV_45_ERR = -4,//收到服务器返回45开头的错误码,收到此错误,可以重新尝试发送命令
	FTP_SEND_CMD_ERR = -5,//发送命令错误
	FTP_SEND_USER_ERR = -6,//发送用户名错误
	FTP_SEND_PWD_ERR = -7,//发送密码错误
	FTP_USR_PWD_ERR = -8,//用户名密码错误
	FTP_UNSUPORT_TYPE = -9,//不支持的传输模式
	FTP_OPEN_DATA_ERR = -10,//数据连接错误
	FTP_ERR_ELSE = -11//其他错误
}FTP_ERROR_E;

typedef enum{
	KTYPE_BINARY = 0,//二进制
	KTYPE_ASCII,//
	KTYPE_EBCDIC
}TRANSFER_TYPE_E;//

typedef enum{
	PORT_MODE = 0,//主动模式
	PASV_MODE,//被动模式
}DATA_MODE_E;//

typedef enum{
	FTP_DOWNLOAD_CMD = 1,
	FTP_UPLOAD_CMD = 2,
}FTP_CMD_E;//

PFTP_HANDLE FTP_CreateConnect(char *pcIpAddr, int iPort, PF_FTP_PROC_CALLBACK pFtpProcFunc);

int FTP_DestoryConnect(PFTP_HANDLE pFtpHandle);

int FTP_DestoryConnect2(PFTP_HANDLE pFtpHandle);

int FTP_AddDownLoadFile(PFTP_HANDLE pFtpHandle, char *pcSrcFileName, char *pcDstFileName);

int FTP_FileDownLoad(PFTP_HANDLE pFtpHandle);

int FTP_AddUpLoadFile(PFTP_HANDLE pFtpHandle, char *pcSrcFileName, char *pcDstFileName, int iType, int iMode);

int FTP_FileUpLoad(PFTP_HANDLE pFtpHandle) ;

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif

