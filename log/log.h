#ifndef  _LOG_H_
#define  _LOG_H_

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#define LOG_FILE_NAME "sys.log"
#define LOG_FILE_DIR  "/mnt/mmc/log"
#define	MAX_LOG_NUMBER	10000

#define MAX_LOG_DESC_LEN 128

typedef enum _E_LOG_TYPE
{
    LOG_TYPE_SYS    = 0,
    LOG_TYPE_EVENT  = 1,
    LOG_TYPE_OTHER  = 2
}E_LOG_TYPE;

typedef struct _T_LOG_INFO
{
    int iLogType;  /* 0: system log, 1: event log, 2: other log */
    char acLogDesc[MAX_LOG_DESC_LEN];
}__attribute__((packed))T_LOG_INFO, *PT_LOG_INFO;

typedef struct  _T_LOG_TIME_INFO
{
    short year; 
    char month;
    char day;
	char hour;
	char minute;
	char second;
	char unusd; 
}__attribute__((packed))T_LOG_TIME_INFO, *PT_LOG_TIME_INFO;

typedef struct _T_MSG_QUERY_TIME_INFO
{
    char cCh;                 //bit0 : 0 ch, bit1: 1 ch, bit2: 2 ch, 为1时表示有效
    char unused[3];
    T_LOG_TIME_INFO tStartTime;
    T_LOG_TIME_INFO tEndTime;
}__attribute__((packed))T_MSG_QUERY_TIME_INFO, T_MSG_QUERY_LOG, T_MSG_QUERY_SNAPSHOT, T_MSG_QUERY_RECORD;

typedef struct _T_MSG_LOG_INFO
{
	T_LOG_TIME_INFO tLogTime;
    char acLog[MAX_LOG_DESC_LEN];	
}__attribute__((packed))T_MSG_LOG_INFO, *PT_MSG_LOG_INFO;

int LOG_Init(char *pLogDir);
void LOG_UnInit(void);
int LOG_WriteLog(PT_LOG_INFO ptLog);
int LOG_ReadLog(T_LOG_TIME_INFO *ptLogTime, PT_MSG_LOG_INFO ptLog, int iType);

int LOG_FsyncFile(void);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif
