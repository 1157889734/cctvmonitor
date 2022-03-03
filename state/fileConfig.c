#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "state/fileConfig.h"
#include "./debugout/debug.h"

static int strstrim(char *psrc,char *pdst)
{	
	while(*psrc)
	{
		if(*psrc != 13 && *psrc != 10 && *psrc != 20  ) // 回车  换行 空格
		{
			*pdst = *psrc;
			pdst++;
		}
		psrc++;
	}
	return 0;
}
static  int CopyFile(const char *pcDstFile, char *pcSrcFile)
{
    FILE *fpr = NULL;
    FILE *fpw = NULL;
    int iLen = 0;
    int iFileLen = 0;
    int iSize = 0;
    char acBuf[4096];
    
    if (NULL == pcDstFile || NULL == pcSrcFile)
    {
        return -1;	
    }
    fpr = fopen(pcSrcFile, "rb");
    if (fpr == NULL)
    {
        perror("open read file:");
        return -1;
    }

    fpw = fopen(pcDstFile, "wb");
    if (fpw == NULL)
    {
        perror("open write file error:");
        
        return -1;
    }

    while (1)
    {
        iLen = fread(acBuf, 1, sizeof(acBuf), fpr);
        if (iLen <= 0)
        {
            break;
        }
        iSize = fwrite(acBuf, 1, iLen, fpw);
        if (iSize != iLen)
        {
            perror("wirte file error:");
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


int ModifyParam(const char *pcFileName, const char *pcGroupKey, const char *pcKey, char *pcValue)
{
	FILE *pOldFile = NULL;
	FILE *pNewFile = NULL;
	char acTmpName[256];
	char acBuf[256];
	int iFlag = 0;
	int iGroupKeyFindFlag = 0;    //组关键字找到标识

	pOldFile = fopen(pcFileName, "rb");
	if (NULL == pOldFile)
	{
		DebugPrint(DEBUG_ERROR_PRINT, "[%s] open file %s error\n", __FUNCTION__, pcFileName);
		perror(":");
		return -1;
	}

	snprintf(acTmpName, sizeof(acTmpName)-1, "%s~", pcFileName);
	pNewFile = fopen(acTmpName, "wb");
	if (NULL == pNewFile)
	{
		DebugPrint(DEBUG_ERROR_PRINT, "[%s] open file %s error\n", __FUNCTION__, acTmpName);
		perror(":");
		fclose(pOldFile);
		return -1;
	}
	while (fgets(acBuf, sizeof(acBuf), pOldFile))
	{
		if (memcmp(acBuf, pcGroupKey, strlen(pcGroupKey)) == 0)
		{
			iGroupKeyFindFlag = 1;
			fputs(acBuf, pNewFile);
			continue;
		}
		if ((1 == iGroupKeyFindFlag) && (memcmp(acBuf, pcKey, strlen(pcKey)) == 0))
		{
			snprintf(acBuf, sizeof(acBuf), "%s=%s\n", pcKey, pcValue);
			fputs(acBuf, pNewFile); 
			iFlag++;
			iGroupKeyFindFlag = 0;   //防止后面有重复的标签内容也被修改
		}
		else
		{
			fputs(acBuf, pNewFile);
		}
	}
	
	// 如果没有此项，则添加一项
	if (0 == iFlag)
	{
		if (0 == iGroupKeyFindFlag)    //没找到组名则先添加组名
		{
			memset(acBuf, 0, sizeof(acBuf));
			snprintf(acBuf, sizeof(acBuf), "%s\n", pcGroupKey);
			fputs(acBuf, pNewFile); 
		}
		
		memset(acBuf, 0, sizeof(acBuf));
		snprintf(acBuf, sizeof(acBuf), "%s=%s\n", pcKey, pcValue);
		fputs(acBuf, pNewFile); 
	}
	fflush(pNewFile);
	fsync(fileno(pNewFile));
	fclose(pOldFile);
	fclose(pNewFile);
	//rename(acTmpName, pcFileName);

	CopyFile(pcFileName, acTmpName);
	
	return 0;
}


int ReadParam(const char *pcFileName, const char *pcGroupKey, const char *pcKey, char *pcValue)
{
	FILE *pFile = NULL;
	char acBuf[256] = {0};
	char *pcPos = NULL;
	int iGroupKeyFindFlag = 0;    //组关键字找到标识

	pFile = fopen(pcFileName, "rb");
	if (NULL == pFile)
	{
		DebugPrint(DEBUG_ERROR_PRINT, "[%s] open file %s error\n", __FUNCTION__, pcFileName);
		perror(":");
		return -1;
	}

	if(NULL == pcGroupKey)
	{
		iGroupKeyFindFlag = 1;
	}
	
	while (fgets(acBuf, sizeof(acBuf), pFile))
	{	
		if(0 == iGroupKeyFindFlag)
		{
			if (memcmp(acBuf, pcGroupKey, strlen(pcGroupKey)) == 0)
			{
				iGroupKeyFindFlag = 1;   //找到组关键字
				continue;
			}
		}
		
		if ((1 == iGroupKeyFindFlag) && (memcmp(acBuf, pcKey, strlen(pcKey)) == 0))    //先确保找到组关键字，才能继续找到组内关键字pcKey
		{
			pcPos = acBuf;
			strsep(&pcPos, "=");
			if (NULL == pcPos)
			{
				DebugPrint(DEBUG_ERROR_PRINT, "line:%s, param error!!!\n", acBuf);
				break;
			}
			strstrim(pcPos,pcValue);
			if(0 == strlen(pcValue) )
			{
				fclose(pFile);
				return -1;
			}
			fclose(pFile);
			return  strlen(pcValue);
		}
		memset(acBuf,0,sizeof(acBuf));
	}
	
	fclose(pFile);
	return -1;
}

int DeleteParam(const char *pcFileName, char *pcValue)
{
	FILE *pOldFile = NULL;
	FILE *pNewFile = NULL;
	char acTmpName[256];
	char acBuf[256];

	pOldFile = fopen(pcFileName, "rb");
	if (NULL == pOldFile)
	{
		DebugPrint(DEBUG_ERROR_PRINT, "[%s] open file %s error\n", __FUNCTION__, pcFileName);
		perror(":");
		return -1;
	}

	snprintf(acTmpName, sizeof(acTmpName), "%s~", pcFileName);
	pNewFile = fopen(acTmpName, "wb");
	if (NULL == pNewFile)
	{
		DebugPrint(DEBUG_ERROR_PRINT, "[%s] open file %s error\n", __FUNCTION__, acTmpName);
		perror(":");
		fclose(pOldFile);
		return -1;
	}
	while (fgets(acBuf, sizeof(acBuf), pOldFile))
	{
		if (memcmp(acBuf, pcValue, strlen(pcValue)) == 0)
		{
			strcpy(acBuf, "");
			fputs(acBuf, pNewFile); 
		}
		else
		{
			fputs(acBuf, pNewFile);
		}
	}
	
	fflush(pNewFile);
	fsync(fileno(pNewFile));
	fclose(pOldFile);
	fclose(pNewFile);
	//rename(acTmpName, pcFileName);

	CopyFile(pcFileName, acTmpName);
	
	return 0;
}

