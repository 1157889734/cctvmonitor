#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "comm.h"
#include <iconv.h>


/****************************************
pInbuf： 源地址
iInlen:  源地址长度
pOutbuf：目的地址
iOutMaxlen：目的地址的最大缓存
返回值：0为成功
*****************************************/
int GBKToUTF8(char *pInbuf,int iInlen,char *pOutbuf,unsigned int iOutMaxlen)
{

//windows
#if 0  
	WCHAR * str1 = NULL;
	int len = MultiByteToWideChar(CP_ACP, 0, inbuf, inlen, NULL, 0);
	if(!len)
	{
		return -1;
	}
	str1 = new WCHAR[len+1];
	memset(str1, 0, sizeof(WCHAR) * (len+1));
	if(!MultiByteToWideChar(CP_ACP, 0, inbuf, inlen, str1, len))
	{
		delete[]str1;
		str1 = NULL;
		return -1;
	}
	len = WideCharToMultiByte(CP_UTF8, 0, str1, -1, outbuf, outMaxlen, NULL, NULL);
	delete[]str1;
	return len?0:-1;
#else
	iconv_t iConvHandle;
	char **pin = &pInbuf;
	char **pout = &pOutbuf;
	size_t iInputLen = iInlen;
	size_t iOutPutLen = iOutMaxlen; 
	
	iConvHandle = iconv_open("UTF-8","GBK");
	if (iConvHandle==0)
	{
		return -1;
	}
	memset(pOutbuf,0,iOutMaxlen);
	if (iconv(iConvHandle,pin,&iInputLen,pout,&iOutPutLen) == (size_t)(-1))
	{
		iconv_close(iConvHandle);
		return -2;
	}	
	iconv_close(iConvHandle);
	return iOutPutLen;
#endif
}

int UTF8ToGBK( char *pInbuf,int iInlen, char *pOutbuf,unsigned int iOutMaxlen)
{
#if 0
	int len = MultiByteToWideChar(CP_UTF8, 0, inbuf, inlen, NULL, 0);
	if(!len)
	{
		return -1;
	}
	unsigned short * wszGBK = new unsigned short[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	if(!MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)inbuf, inlen, (LPWSTR)wszGBK, len))
	{
		delete[]wszGBK;
		return -1;
	}
	len = WideCharToMultiByte(CP_ACP,0, (LPWSTR)wszGBK, -1, outbuf, outMaxlen, NULL, NULL);
	delete[]wszGBK;
	return len?0:-1;
#else
	iconv_t iConvHandle;
	char **pin = &pInbuf;
	char **pout = &pOutbuf;
	size_t iInputLen = iInlen;
	size_t iOutPutLen = iOutMaxlen; 

	iConvHandle = iconv_open("GB2312","UTF-8");
	if (iConvHandle==0)
	{
		return -1;
	}
	memset(pOutbuf,0,iOutMaxlen);
	if (iconv(iConvHandle,pin,&iInputLen,pout,&iOutPutLen) == (size_t)(-1))
	{
		iconv_close(iConvHandle);
		return -2;
	}	
	iconv_close(iConvHandle);
	return 0;
#endif
}

int GetMaxDay(int iYear,int iMonth)
{
	int iMaxDay = 31;
	switch(iMonth)
	{
		case 4:
		case 6:
		case 9:
		case 11:
		iMaxDay = 30;
		break;
		case 2:
		{
			if((iYear %400 == 0) || ((iYear %100 != 0) && (iYear %4 == 0 )))
			{
				iMaxDay = 29;
			}
			else
			{
				 iMaxDay = 28;
			}
					
		}
		break;
		default:
		iMaxDay = 31;
	}
	return iMaxDay;
}


