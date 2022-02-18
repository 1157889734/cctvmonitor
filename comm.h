#ifndef _CCTV_COMM_H
#define _CCTV_COMM_H


int GBKToUTF8( char *pInbuf,int iInlen,char *pOutbuf,unsigned int iOutMaxlen);
int UTF8ToGBK( char *pInbuf,int iInlen,char *pOutbuf,unsigned int iOutMaxlen);
int GetMaxDay(int iYear,int iMonth);


#endif
