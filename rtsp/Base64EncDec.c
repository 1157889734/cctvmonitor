//#include "stdafx.h"
#include <stdio.h>
#include <string.h>

#include "Base64EncDec.h"


/*************************************************************************
- 函数名称: drm_Base64Encode_ext
- 功能描述: 生成Base64编码(扩展)
- 输　　入: b64chars	: 可见字符列表，必须为以'\0'结尾的长度64的可见字符串
			inbuf		: 待处理的字节缓冲区指针
			inlen		: 待处理的字节缓冲区长度
- 输　　出: outbuf		: 计算结果指针
			poutlen  	: 计算结果字节长度指针（可以传入0）
- 返　　回: 1	- 成功
			-1	- 失败
- 全局变量: 无															 
- 注　　释: 可以自定义可见字符列表，避免生成含特殊字符如'+'和'/'的编码串
----------------------------------------------------------------------------									
****************************************************************************/
int drm_Base64Encode_ext(const char *b64chars, unsigned char *inbuf, int inlen, unsigned char *outbuf, int *poutlen)
{ 
	int i				= 0;
	int idx			= 0;
	int bit_offset	= 0;
	int byte_offset	= 0;
	unsigned char *d = (unsigned char *)inbuf;
	int outlen = (inlen*8 + 5)/6;
	
	if(0 == b64chars || 64 != strlen(b64chars))
	{
		return -1;
	}

	if(0 == inbuf || 0 == outbuf)
	{
		return -1;
	}
	for (i=0;i<outlen;i++) 
	{
		byte_offset = (i*6)/8;
		bit_offset  = (i*6)%8;
		if (bit_offset < 3) 
		{
			idx = (d[byte_offset] >> (2-bit_offset)) & 0x3F;
		} 
		else 
		{
			idx = (d[byte_offset] << (bit_offset-2)) & 0x3F;
			if (byte_offset+1 < inlen) 
			{
				idx |= (d[byte_offset+1] >> (8-(bit_offset-2)));
			}
		}
//		if(idx<0 || idx>63)
//			printf("\n\n+++idx=%d\n\n", idx);
		outbuf[i] = b64chars[idx];
	}
	
	switch(outlen%4)
	{
		case 2:
	    	outbuf[outlen++] = '=';
		case 3:
	    	outbuf[outlen++] = '=';
			break;
		default:
			break;
	}

	outbuf[outlen] = '\0';

	if(0 != poutlen)
	{
		*poutlen = outlen;
	}
	
	return 1; 
	
} /* end of drm_Base64Encode() */


/*************************************************************************
- 函数名称: drm_Base64Encode
- 功能描述: 生成Base64编码
- 输　　入: inbuf	 : 待处理的字节缓冲区指针
			inlen	 : 待处理的字节缓冲区长度
- 输　　出: outbuf	: 计算结果指针
			poutlen  : 计算结果字节长度指针（可以传入0）
- 返　　回: 1	- 成功
			-1	- 失败
- 全局变量: 无															 
- 注　　释: 
----------------------------------------------------------------------------									
****************************************************************************/
int drm_Base64Encode(unsigned char *inbuf, int inlen, unsigned char *outbuf, int *poutlen)
{ 
	static char *b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	return drm_Base64Encode_ext(b64chars, inbuf, inlen, outbuf, poutlen);
	
} /* end of drm_Base64Encode() */


/*************************************************************************
- 函数名称: drm_Base64Decode
- 功能描述: 生成Base64解码值
- 输　　入: inbuf	 : 待处理的字节缓冲区指针
			inlen	 : 待处理的字节缓冲区长度
- 输　　出: outbuf	: 计算结果指针
			poutlen  : 计算结果字节长度指针（可以传入0）
- 返　　回: 1	- 成功
			-1	- 失败
- 全局变量: 无															 
- 注　　释: 无
----------------------------------------------------------------------------
****************************************************************************/


int drm_Base64Decode(unsigned char *inbuf, int inlen, unsigned char *outbuf, int *poutlen)
{
    int  i_level = 0;
    int  last = 0;
    //int  i,j;
    unsigned char  *pinbuf  = 0;
    unsigned char  *poutbuf = 0;
    static unsigned char b64map[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
        52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
        15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
        -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
        41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
        };

	//printf("inbuf=%s,inlen=%d,outbuf=0x%x,poutlen=0x%x\n",inbuf,inlen,(unsigned int)outbuf,(unsigned int)poutlen);

	if((void*)0 == inbuf || (void*)0 == outbuf || 0 != (inlen%4))
	{
		printf("0 == inbuf(%s) || 0 == outbuf(%s) || 0 != inlen mod 4(%d)\n",inbuf,outbuf,inlen%4);
		return -1;
	}

    pinbuf  = inbuf;
    poutbuf = outbuf;

    for( i_level = 0; (pinbuf - inbuf) < inlen; pinbuf++ )
    {
        char  c;
        c = b64map[(unsigned char)*pinbuf];
        if( -1 == c )
        {
            continue;
        }

        switch( i_level )
        {
            case 0:
                i_level++;
                break;
                
            case 1:
                *poutbuf++ = ( last << 2 ) | ( ( c >> 4)&0x03 );
                i_level++;
                break;
                
            case 2:
                *poutbuf++ = ( ( last << 4 )&0xf0 ) | ( ( c >> 2 )&0x0f );
                i_level++;
                break;
                
            case 3:
                *poutbuf++ = ( ( last &0x03 ) << 6 ) | c;
                i_level = 0;
        }
        last = c;
    }

    /* 判断0 20050220 by HGD */
	if(0 != poutlen)
	{
		*poutlen = poutbuf - outbuf;
	}
	
	//add by yinchengshui 
	*poutbuf ='\0';
	return 1; 

} 

int  Base64Encode(unsigned char *inbuf, int inlen, unsigned char *outbuf, int *poutlen)
{
	return drm_Base64Encode(inbuf, inlen, outbuf, poutlen);
}

int  Base64Decode(unsigned char *inbuf, int inlen, unsigned char *outbuf, int *poutlen)
{
	return drm_Base64Decode(inbuf, inlen, outbuf, poutlen);
}




