#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <errno.h>
#include        <unistd.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#include        <pthread.h>

#include 		"log/log.h"
#include		"debugout/debug.h"

static int g_iLogCount = 0;
static int g_iSyncFlag = 0;
static pthread_mutex_t	g_tLogMutex;
static char	g_acLogPath[128] = {0};

static int CopyFile(char *pcDstFile, char *pcSrcFile)
{
    FILE *fpr = NULL;
    FILE *fpw = NULL;
    int iLen = 0;
    int iFileLen = 0;
    int iSize = 0;
    char acBuf[4096];
    
    if(NULL == pcDstFile || NULL == pcSrcFile)
    {
        return -1;	
    }
    fpr = fopen(pcSrcFile, "rb");
    if (fpr == NULL)
    {
    	DebugPrint(DEBUG_ERROR_PRINT,"open file %s failed:%s",pcSrcFile,strerror(errno));
        return -1;
    }

    fpw = fopen(pcDstFile, "wb");
    if (fpw == NULL)
    {
        //printf("[%s]open write file %s\n", __FUNCTION__, pcDstFile);
        //perror("open write file error:"); 
        DebugPrint(DEBUG_ERROR_PRINT,"open file %s failed:%s",pcDstFile,strerror(errno));
        return -1;
    }

    while (1)
    {
        iLen = fread(acBuf, 1, sizeof(acBuf), fpr);
        if (iLen <= 0)
        {
            //printf("read file over\n");
            break;
        }
        iSize = fwrite(acBuf, 1, iLen, fpw);
        if (iSize != iLen)
        {
			DebugPrint(DEBUG_ERROR_PRINT,"wirte file %s error:%s",pcDstFile,strerror(errno));
            fclose(fpr);
            fclose(fpw);
            return -1;
        }
        iFileLen += iLen;
    }
    fflush(fpw);
    fsync(fileno(fpw));
    fclose(fpr);
    fclose(fpw);
    
    return iFileLen;
}

int LOG_Init(char *pLogDir)
{
    FILE *fp = NULL;
    char acBuf[256];

    pthread_mutexattr_t mutexattr;

	sprintf(g_acLogPath,"%s/%s",pLogDir,LOG_FILE_NAME);
    fp = fopen(g_acLogPath, "rb");
    if (NULL == fp)
    {
    	if(access(pLogDir, NULL)!=0 )  
  		{  
      		if(mkdir(pLogDir, 0755)==-1)  
      		{  
      			DebugPrint(DEBUG_ERROR_PRINT,"open file %s error:%s",g_acLogPath,strerror(errno));
                return   -1;   
      		}  
  		}  
        fp = fopen(g_acLogPath, "ab");
    }

    if (NULL == fp)
    {
    	DebugPrint(DEBUG_ERROR_PRINT,"open file %s error:%s",g_acLogPath,strerror(errno));
        //printf("open file %s error\n", LOG_FILE_NAME);
       	//perror(":");
        return -1;
    }
	
    while (fgets(acBuf, sizeof(acBuf), fp))
    {
        g_iLogCount ++;	
    }
    fclose(fp);

    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_TIMED_NP);
    pthread_mutex_init(&g_tLogMutex, &mutexattr);
    pthread_mutexattr_destroy(&mutexattr);
        
    return 0;
}

void LOG_UnInit(void)
{
    pthread_mutex_destroy(&g_tLogMutex);
}

int LOG_WriteLog(PT_LOG_INFO ptLog)
{
    FILE *pFile, *pNewFile;
    char acLogTime[24];
    char acBuf[256];
    char acTmpName[128];
    int iFd;
    int iLogCount = 0;
    
    if (NULL == ptLog)
    {
        DebugPrint(DEBUG_ERROR_PRINT,"[%s] param error\n", __FUNCTION__);
        return -1;
    }
    
    struct	tm	ptr;
    time_t tTime = time(NULL);

    if (localtime_r(&tTime, &ptr) == NULL)
    {
        return(-1);
    }

    snprintf(acLogTime, sizeof(acLogTime), "%4d%02d%02d %02d:%02d:%02d", 
             (ptr.tm_year+1900), ptr.tm_mon+1, ptr.tm_mday,
             ptr.tm_hour, ptr.tm_min, ptr.tm_sec);
    
    pthread_mutex_lock(&g_tLogMutex);
    /* <daymonyear hour:min:second> log description */
    if (g_iLogCount < MAX_LOG_NUMBER + 1000)
    {
        pFile = fopen(g_acLogPath, "ab");
        if (NULL == pFile)
        {
            DebugPrint(DEBUG_ERROR_PRINT,"[%s] open file %s error\n", __FUNCTION__, g_acLogPath);
            pthread_mutex_unlock(&g_tLogMutex);
            return -1;
        }
       // fseek(pFile, 0, SEEK_END);
        if (LOG_TYPE_EVENT == ptLog->iLogType)
        {
            snprintf(acBuf, sizeof(acBuf), "e%s  %s\r\n",
                         acLogTime, ptLog->acLogDesc);
        }
        else
        {
            snprintf(acBuf, sizeof(acBuf), "s%s  %s\r\n",
                         acLogTime, ptLog->acLogDesc);
        }
        //printf("[%s] %s\n", __FUNCTION__, acBuf);
        fputs(acBuf, pFile);
        fflush(pFile);
        fclose(pFile);
        g_iLogCount ++;
        
        if (0 == (g_iLogCount % 20))
        {
            //snprintf(acTmpName, sizeof(acTmpName), "%s~", LOG_FILE_NAME);
            //CopyFile(acTmpName, LOG_FILE_NAME);
        }
    }
    else
    {
        // delete the oldest 100 logs
        pFile = fopen(g_acLogPath, "rb");
        if (NULL == pFile)
        {
            DebugPrint(DEBUG_ERROR_PRINT,"[%s] open file %s error\n", __FUNCTION__, g_acLogPath);
            //perror(":");
            pthread_mutex_unlock(&g_tLogMutex);
            return -1;
        }
        snprintf(acTmpName, sizeof(acTmpName), "%s~", g_acLogPath);
        pNewFile = fopen(acTmpName, "wb");
        if (NULL == pNewFile)
        {
            DebugPrint(DEBUG_ERROR_PRINT,"[%s] open file %s error\n", __FUNCTION__, acTmpName);
            //perror(":");
            fclose(pFile);
            pthread_mutex_unlock(&g_tLogMutex);
            return -1;
        }
        while (fgets(acBuf, sizeof(acBuf), pFile))
        {
            iLogCount ++;
            if (1000 == iLogCount)
            {
                break;	
            }
        }
        g_iLogCount -= 1000;
        while (fgets(acBuf, sizeof(acBuf), pFile))
        {
            fputs(acBuf, pNewFile);	
        }
        DebugPrint(DEBUG_ERROR_PRINT,"[%s] Log over max count, and delete 1000\n", __FUNCTION__);

        if (LOG_TYPE_EVENT == ptLog->iLogType)
        {
            snprintf(acBuf, sizeof(acBuf), "e%s  %s\r\n",
                         acLogTime, ptLog->acLogDesc);
        }
        else
        {
            snprintf(acBuf, sizeof(acBuf), "s%s  %s\r\n",
                         acLogTime, ptLog->acLogDesc);
        }
		fputs(acBuf, pNewFile);
        
        iFd = fileno(pNewFile);	
        fsync(iFd);
        fclose(pNewFile);
        fclose(pFile);
        //rename(acTmpName, LOG_FILE_NAME);
        CopyFile(g_acLogPath, acTmpName);
        g_iLogCount ++;
    }
    g_iSyncFlag ++;

    pthread_mutex_unlock(&g_tLogMutex);
	
    return 0;
}

static int ParseTimeFromLog(PT_LOG_TIME_INFO ptTime, char *pcBuf)
{
    char acStr[256];
    char acYear[5];
    char acMon[3];
    char acDay[3];
    char *pcTmp = NULL;
    char *pcPos = NULL;
    
    if (NULL == ptTime || NULL == pcBuf)
    {
        return -1;	
    }
    strncpy(acStr, pcBuf, sizeof(acStr));
    pcPos = acStr;
    pcTmp = strsep(&pcPos, " ");

    if (pcTmp)
    {
        // day mon year
        memcpy(acYear, pcTmp, 4);
		memcpy(acMon, pcTmp+4, 2);
        memcpy(acDay, pcTmp+6, 2);

        acYear[4] = '\0';
        acMon[2] = '\0';
        acDay[2] = '\0';

        ptTime->year = atoi(acYear);
        ptTime->month = atoi(acMon);
        ptTime->day = atoi(acDay);
    }
    if (pcPos)
    {
    	  // hour min second
        pcTmp = strsep(&pcPos, ":");
        if (pcTmp)
        {
            ptTime->hour = atoi(pcTmp);
        }
        if (pcPos)
        {
            pcTmp = strsep(&pcPos, ":");
            if (pcTmp)
            {
                ptTime->minute = atoi(pcTmp);	
            }
            if (pcPos)
            {
                ptTime->second = atoi(pcPos);	
            }
        }
    }
    
    return 0;
}

static time_t COMM_MakeTime(int year, int month, int day, int hour, int minute, int second)
{
    time_t  t;
    struct  tm      ptr;

    ptr.tm_sec = second;
    ptr.tm_min = minute;
    ptr.tm_hour = hour;
    ptr.tm_mday = day;
    ptr.tm_mon = month - 1;
    ptr.tm_year = year - 1900;
    ptr.tm_isdst = -1;
    if ((t = mktime(&ptr)) < 0)
    {
        return(-1);
    }
    
    return(t);
}



int CompareTime(PT_LOG_TIME_INFO ptLogTime, PT_LOG_TIME_INFO ptSearchTime)
{
   time_t tLogTime;
    time_t tSearchTime;
    
    tLogTime 	= COMM_MakeTime(ptLogTime->year, ptLogTime->month, ptLogTime->day,0,0,0);
    tSearchTime = COMM_MakeTime(ptLogTime->year, ptLogTime->month, ptLogTime->day,0,0,0);
    if(ptLogTime->year == ptSearchTime->year && ptLogTime->month == ptSearchTime->month
		&& ptLogTime->day == ptSearchTime->day)
    {
    	return 0;
    }
    return -1;
}


int LOG_ReadLog(T_LOG_TIME_INFO *ptLogTime, PT_MSG_LOG_INFO ptLog, int iType)
{
	
   FILE *fpr = NULL;
   int iLogCount = 0;
   int iRet = 0;
   char acBuf[256];
   T_LOG_TIME_INFO tTime;

   if (NULL == ptLogTime || NULL == ptLog)
   {
	   DebugPrint(DEBUG_ERROR_PRINT,"[%s]param error", __FUNCTION__);
	   return 0;
   }
   pthread_mutex_lock(&g_tLogMutex);
   fpr = fopen(g_acLogPath, "rb");
   if (fpr == NULL)
   {
   	   pthread_mutex_unlock(&g_tLogMutex);
	   DebugPrint(DEBUG_ERROR_PRINT,"[%s]open read file:", __FUNCTION__);
	   //perror("[FindLog]open read file:");
	   return 0;
   }

   while (fgets(acBuf, sizeof(acBuf), fpr) && iLogCount<10000)
   {
	   if (((LOG_TYPE_EVENT == iType)  &&  ('e' == acBuf[0]) )
	   	|| ((LOG_TYPE_SYS == iType)  &&  ('s' == acBuf[0]) ))
	   {
	   	   acBuf[strlen(acBuf)-2] = 0;
		   ParseTimeFromLog(&tTime, &acBuf[1]);
		   iRet = CompareTime(&tTime, ptLogTime);
		   if (iRet == 0)
		   {
		   	   ptLog[iLogCount].tLogTime = tTime;
			   strncpy(ptLog[iLogCount].acLog, &acBuf[19], sizeof(ptLog[iLogCount].acLog));
			   //sprintf("log:%s\n",ptLog[iLogCount].acLog);
			   iLogCount ++;
		   }
	   }
	   memset(acBuf,0,sizeof(acBuf));
   }
   
   pthread_mutex_unlock(&g_tLogMutex);
   fclose(fpr);
   return iLogCount;
}


int LOG_FsyncFile(void)
{
    int iFd = 0;
    
    if (g_iSyncFlag)
    { 
        pthread_mutex_lock(&g_tLogMutex);
        iFd = open(g_acLogPath, O_RDWR);
        if (iFd < 0)
        {
			pthread_mutex_unlock(&g_tLogMutex);
            return -1;
        }
        fsync(iFd);
        close(iFd);
        g_iSyncFlag = 0;
        pthread_mutex_unlock(&g_tLogMutex);
    }
    return 0;
}

