#ifdef WIN
  #include "stdafx.h"
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <sys/socket.h>
#endif

#include "types.h"
#include "rtspComm.h"
#include "rtsp.h"
#include "rtp.h"

static char g_acNaluHeader[4]={0,0,0,1};


#if 0

int RtpParseH264Data(PT_RTSP_CONN ptRtspConn, char **ppFrame, int iFrameLen, char *pcRtpData, int iDataLen, int iMaxPacketLen)
{
    char cNaluStart     = 0 ;
    char cNaluEnd       = 0;
	char cNaluUri       = 0;
    char cPayloadType   = 0;
	char cPacketType    = 0;
    char cRtpMFlag      = 0;
	char cNaluType      = 0;
    char *pcRtpPacket   = NULL;
    char *pcFrameBuf    = NULL;
    int iLeaveLen       = iDataLen;
    int iNaluLen        = 0;
    int iRtpLen         = 0;
	int iRtpPacketNum   = 0;

    if (NULL == ppFrame)
    {
        return 0;	
    }
    
    if (NULL == *ppFrame)
    {
        iRtpPacketNum = iDataLen / iMaxPacketLen;
        *ppFrame = (char *)malloc(iDataLen - (iRtpPacketNum * (RTP_HEADER_LEN - NALU_HEADER_SIZE)));
    }
    else
    {
        iRtpPacketNum = iDataLen / iMaxPacketLen;
        *ppFrame = (char *)realloc(*ppFrame, iFrameLen + (iDataLen - (iRtpPacketNum * (RTP_HEADER_LEN - NALU_HEADER_SIZE))));
    }
    if (NULL == *ppFrame)
    {
        return 0;	
    }
    
    pcFrameBuf = *ppFrame;
    
    while (iLeaveLen > 0)
    {
        iRtpLen = iLeaveLen > iMaxPacketLen ? iMaxPacketLen : iLeaveLen;
        pcRtpPacket = &pcRtpData[iDataLen - iLeaveLen];
        cRtpMFlag = pcRtpPacket[1] >> 7;
        cPacketType = pcRtpPacket[RTP_HEADER_LEN] & 0x1f;
        iNaluLen = iRtpLen - RTP_HEADER_LEN; 
 
 
        if ((cPacketType > 0) && (cPacketType <= 24))
        {
            memcpy(&pcFrameBuf[iFrameLen], g_acNaluHeader, NALU_HEADER_SIZE);
            iFrameLen += NALU_HEADER_SIZE;
            memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN], iNaluLen);
            iFrameLen += iNaluLen;

        }
        else if (cPacketType == 28)
    	{
	        // FU
            cNaluUri   = pcRtpPacket[RTP_HEADER_LEN] & 0x60;
            cNaluStart = pcRtpPacket[RTP_HEADER_LEN + 1] >> 7;
            cNaluEnd   = (pcRtpPacket[RTP_HEADER_LEN + 1] >> 6) & 0x1;
            cNaluType  = pcRtpPacket[RTP_HEADER_LEN + 1] & 0x1f;
    
            cNaluType |= cNaluUri;
		  
	        iNaluLen -= H264_FU_SIZE;
            if (1 == cNaluStart)
            {
                memcpy(&pcFrameBuf[iFrameLen], g_acNaluHeader, NALU_HEADER_SIZE);
                iFrameLen += NALU_HEADER_SIZE;
                pcFrameBuf[iFrameLen] = cNaluType;
                iFrameLen++;
                memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN + H264_FU_SIZE], iNaluLen);
                iFrameLen += iNaluLen;
	          }
	          else
	          {
                memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN + H264_FU_SIZE], iNaluLen);
                iFrameLen += iNaluLen;
            }
        }
        
        iLeaveLen -= iRtpLen;
        
        if (cRtpMFlag)
        {
            /* frame end	*/
            if (ptRtspConn->pfRtpSetDataCB)
            {
                ptRtspConn->pfRtpSetDataCB(STREAM_TYPE_VIDEO, pcFrameBuf, iFrameLen, ptRtspConn->pRtpCbArg);
            }
            free(pcFrameBuf);
            pcFrameBuf = NULL;
            *ppFrame = NULL;
            iFrameLen = 0;
            
            return 0;
        }
    }

    return (iFrameLen);
}

#endif
int RtpParseMPAData(PT_RTSP_CONN ptRtspConn, char **ppFrame, int iFrameLen, char *pcRtpData, int iDataLen, int iMaxPacketLen)
{
    char *pcRtpPacket   = NULL;
    char *pcFrameBuf    = NULL;
    int iLeaveLen       = iDataLen;
    int iRtpLen         = 0;
    int iSliceLen       = 0;
	int iRtpPacketNum   = 0;
    char cRtpMFlag      = 0;

    if (NULL == ppFrame)
    {
        return 0;	
    }
    
    if (NULL == *ppFrame)
    {
        iRtpPacketNum = iDataLen / iMaxPacketLen;
        *ppFrame = (char *)malloc(iDataLen - (iRtpPacketNum * RTP_HEADER_LEN));
    }
    else
    {
        iRtpPacketNum = iDataLen / iMaxPacketLen;
        *ppFrame = (char *)realloc(*ppFrame, iFrameLen + (iDataLen - (iRtpPacketNum * RTP_HEADER_LEN)));
    }
    if (NULL == *ppFrame)
    {
        return 0;	
    }
    
    pcFrameBuf = *ppFrame;
    
    while (iLeaveLen > 0)
    {
        iRtpLen = iLeaveLen > iMaxPacketLen ? iMaxPacketLen : iLeaveLen;
        pcRtpPacket = &pcRtpData[iDataLen - iLeaveLen];
        cRtpMFlag = pcRtpPacket[1] >> 7;
        iSliceLen = iRtpLen - RTP_HEADER_LEN - 4;
        
        if ((pcRtpPacket[RTP_HEADER_LEN] == 0) 
        	  && (pcRtpPacket[RTP_HEADER_LEN + 1] == 0) 
        	  && (pcRtpPacket[RTP_HEADER_LEN + 2] == 0) 
        	  && (pcRtpPacket[RTP_HEADER_LEN + 3] == 0))
        {
            /* MPA frame start */

            
        }
        memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN + 4], iSliceLen);
        iFrameLen += iSliceLen;
        
        if (cRtpMFlag)
        {
            /* frame end	*/
          /*  if (ptRtspConn->pfRtpSetDataCB)
            {
                ptRtspConn->pfRtpSetDataCB(STREAM_TYPE_AUDIO, pcFrameBuf, iFrameLen, 0, ptRtspConn->pRtpCbArg);
            }*/
            free(pcFrameBuf);
            pcFrameBuf = NULL;
            *ppFrame = NULL;
            iFrameLen = 0;
            
            return 0;
        }
    }
    
    return iFrameLen;
}

#ifdef WIN
	DWORD WINAPI RtpRecvThread(void *arg)
#else
  void *RtpRecvThread(void *arg)
#endif
{
    PT_RTP_CONN ptRtpConn       = (PT_RTP_CONN)arg;
    PT_RTSP_CONN ptRtspConn     = NULL;
    PT_RTP_HEADER ptRtpHeader   = NULL;
    char *pcRecvBuf             = NULL;
    char *pcFrame               = NULL;
    char *pcRtpData             = NULL;
    char cCurPayloadType        = 0;
    char cPrePayloadType        = 0;
    UINT32 u32PreTs             = 0;
	UINT16 u16OldSeq            = 0;
    int iFrameLen               = 0;
    int iRecvLen                = 0;
	int iPacketNum              = 0;
	int iFrameLostFlag          = 0;
	int iReqIFrameFlag          = 0;
    
    
    if (NULL == ptRtpConn)
    {
        return NULL;
    }
    if (ptRtpConn->ptRtspConn)
    {
        ptRtspConn = (PT_RTSP_CONN)ptRtpConn->ptRtspConn;
    }
    
    pcRecvBuf = (char *)malloc(MAX_UDP_DATA_SIZE);
    if (NULL == pcRecvBuf)
    {
        goto RTP_ERROR;
    }
    pcRtpData = (char *)malloc(820000);
    if (NULL == pcRtpData)
    {
        goto RTP_ERROR;
    }    
    
    pcFrame = pcRtpData;
    while(ptRtpConn->iRtpThreadRunFlag)
    {
        if (TCP == ptRtspConn->iRtpTransportProtocol)
        {
            goto RTP_ERROR;
        }
        iRecvLen = recvfrom(ptRtpConn->iRtpSocket, pcRecvBuf, MAX_UDP_DATA_SIZE, 0, NULL, NULL);
        if (iRecvLen <= 0)
        {
            goto RTP_ERROR;
        }

        ptRtpConn->iRtpPacketCount++;
        
        ptRtpHeader = (PT_RTP_HEADER)pcRecvBuf;
        cCurPayloadType = ptRtpHeader->pt;
        ptRtpHeader->ts = ntohl(ptRtpHeader->ts);
        ptRtpHeader->seq = ntohs(ptRtpHeader->seq);

		if (96 == cCurPayloadType)
		{
			if ((ptRtpHeader->seq != u16OldSeq + 1) && (ptRtpHeader->seq != 0))
			{
				iFrameLen = 0;
				iFrameLostFlag = 1;
				iReqIFrameFlag = 1;


			}
			u16OldSeq = ptRtpHeader->seq;

			// 防止I帧过大
			if (iFrameLen > 800000)
			{
				iFrameLen = 0;
				iFrameLostFlag = 1;
			}
			/* H264 */
			iFrameLen = RtpParseH264Data(ptRtspConn, pcFrame, iFrameLen, pcRecvBuf, iRecvLen, iFrameLostFlag, &iReqIFrameFlag, ptRtpHeader->ts);
			if (0 == iFrameLen)
			{
			    iFrameLostFlag = 0;
			}
		}
		else if (14 == cPrePayloadType)
		{
		
		}
		
#if 0
        if ((u32PreTs == 0) || ((ptRtpHeader->ts != u32PreTs) || (cCurPayloadType != cPrePayloadType)))
        {
            // 有可能一个帧结束，或有一个包结束
            if (iPacketNum == (ptRtpHeader->seq - u16FirstSeq))
       	    {
       	        if (96 == cPrePayloadType)
                {
                    iFrameLen = RtpParseH264Data(ptRtspConn, &pcFrame, iFrameLen, pcRtpData, iRtpDataLen, iMaxPacketLen);
                }
                else if (14 == cPrePayloadType)
                {
                    iFrameLen = RtpParseMPAData(ptRtspConn, &pcFrame, iFrameLen, pcRtpData, iRtpDataLen, iMaxPacketLen);
                }
        	   
            }
            else
            {
        	    //丢弃1	帧数据
                RTSP_DEBUG("descard a frame data\n");

                if (pcFrame)
                {
                    free(pcFrame);
                    pcFrame = NULL;
                    iFrameLen = 0;
                }

            }
        	  
            iPacketNum = 0;
            iMaxPacketLen = 1450;
            u16FirstSeq = ptRtpHeader->seq;
            iFrameNo = 0;
            memcpy(&pcRtpData[iFrameNo * iMaxPacketLen], pcRecvBuf, iRecvLen);
            iRtpDataLen = iRecvLen;

		}
        else
        {
            if (ptRtpHeader->seq < u16FirstSeq)
            {
                iFrameNo = 0xFFFF - u16FirstSeq + ptRtpHeader->seq + 1;
            }
            else
            {
                iFrameNo = ptRtpHeader->seq - u16FirstSeq;
            }
            memcpy(&pcRtpData[iFrameNo * iMaxPacketLen], pcRecvBuf, iRecvLen);
            iRtpDataLen += iRecvLen;
		}
#endif
        cPrePayloadType = cCurPayloadType;
        u32PreTs = ptRtpHeader->ts;
        iPacketNum ++;
	}

RTP_ERROR:
	
    if (pcRecvBuf)
    {
	    free(pcRecvBuf);
        pcRecvBuf = NULL;
    }
    if (pcRtpData)
    {
	    free(pcRtpData);
        pcRtpData = NULL;
    }
    
    if (ptRtpConn->iRtpSocket > 0)
    {
        RTSP_Close(ptRtpConn->iRtpSocket);
        ptRtpConn->iRtpSocket = -1;	
    }
    
    return NULL;
}


int RtpParseH264Data(PT_RTSP_CONN ptRtspConn, char *pcFrameBuf, int iFrameLen, char *pcRtpData, int iDataLen, int iFrameLostFlag, int *piReqIFrameFlag, unsigned int uiTs)
{
	unsigned char cNaluStart     = 0 ;
	unsigned char cNaluEnd       = 0;
	unsigned char cNaluUri       = 0;
	unsigned char cPayloadType   = 0;
	unsigned char cPacketType    = 0;
	unsigned char cRtpMFlag      = 0;
	unsigned char cNaluType      = 0;
	char *pcRtpPacket   = NULL;
	int iLeaveLen       = iDataLen;
	int iNaluLen        = 0;
	int iRtpLen         = 0;
	int iRtpPacketNum   = 0;
	int iReqIFrameFlag = 0;

	if (NULL == pcFrameBuf)
	{
		return 0;	
	}
	
	if (piReqIFrameFlag)
	{
	    iReqIFrameFlag = *piReqIFrameFlag;
	}

	while (iLeaveLen > 0)
	{
		//iRtpLen = iLeaveLen > iMaxPacketLen ? iMaxPacketLen : iLeaveLen;
		iRtpLen = iDataLen;
		pcRtpPacket = &pcRtpData[iDataLen - iLeaveLen];
		cRtpMFlag = (pcRtpPacket[1] >> 7) & 0x1;;
		cPacketType = pcRtpPacket[RTP_HEADER_LEN] & 0x1f;
		iNaluLen = iRtpLen - RTP_HEADER_LEN; 


		if ((cPacketType > 0) && (cPacketType <= 24))
		{
			memcpy(&pcFrameBuf[iFrameLen], g_acNaluHeader, NALU_HEADER_SIZE);
			iFrameLen += NALU_HEADER_SIZE;
			memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN], iNaluLen);
			iFrameLen += iNaluLen;
		}
		else if (cPacketType == 28)
		{
			// FU
			//unsigned char fu1 = pcRtpPacket[RTP_HEADER_LEN];
			//unsigned char fu2 = pcRtpPacket[RTP_HEADER_LEN + 1];
			cNaluUri   = pcRtpPacket[RTP_HEADER_LEN] & 0x60;
			cNaluStart = (pcRtpPacket[RTP_HEADER_LEN + 1] >> 7) & 0x1;
			cNaluEnd   = (pcRtpPacket[RTP_HEADER_LEN + 1] >> 6) & 0x1;
			cNaluType  = pcRtpPacket[RTP_HEADER_LEN + 1] & 0x1f;

			cNaluType |= cNaluUri;

			iNaluLen -= H264_FU_SIZE;
			if (1 == cNaluStart)
			{
				memcpy(&pcFrameBuf[iFrameLen], g_acNaluHeader, NALU_HEADER_SIZE);
				iFrameLen += NALU_HEADER_SIZE;
				pcFrameBuf[iFrameLen] = cNaluType;
				iFrameLen++;
				memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN + H264_FU_SIZE], iNaluLen);
				iFrameLen += iNaluLen;
			}
			else
			{
				memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN + H264_FU_SIZE], iNaluLen);
				iFrameLen += iNaluLen;
			}
		}

		iLeaveLen -= iRtpLen;

		if (cRtpMFlag)
		{
			/* frame end	*/
			if (!iFrameLostFlag && iReqIFrameFlag)
			{
				char cFrameType = pcFrameBuf[4] & 0x1f;

			    if (((cFrameType & 0x1f) == 0x7) || ((cFrameType & 0x1f) == 0x9))
				{
					// I Frame
					*piReqIFrameFlag = 0;
				}
				else
				{
					iFrameLen = 0;

					return 0;
				}
				
			}
			if (ptRtspConn->pfRtpSetDataCB && !iFrameLostFlag && ptRtspConn->iEnableCBFunFlag)
			{
				ptRtspConn->pfRtpSetDataCB(STREAM_TYPE_VIDEO, E_STREAM_TYPE_H264, pcFrameBuf, iFrameLen, uiTs, ptRtspConn->pRtpCbArg);
				//TRACE("a frame\n");
			}

			iFrameLen = 0;

			return 0;
		}
	}

	return (iFrameLen);
}


int RtpParseH265Data(PT_RTSP_CONN ptRtspConn, char *pcFrameBuf, int iFrameLen, char *pcRtpData, int iDataLen, int iFrameLostFlag, int *piReqIFrameFlag, unsigned int uiTs)
{
	unsigned char cNaluStart     = 0 ;
	unsigned char cNaluEnd       = 0;
	unsigned char cNaluUri       = 0;
	unsigned char cPayloadType   = 0;
	unsigned char cPacketType    = 0;
	unsigned char cRtpMFlag      = 0;
	unsigned char cNaluType      = 0;
	char *pcRtpPacket   = NULL;
	int iLeaveLen       = iDataLen;
	int iNaluLen        = 0;
	int iRtpLen         = 0;
	int iRtpPacketNum   = 0;
	int iReqIFrameFlag = 0;

	if (NULL == pcFrameBuf)
	{
		return 0;	
	}
	
	if (piReqIFrameFlag)
	{
	    iReqIFrameFlag = *piReqIFrameFlag;
	}

	while (iLeaveLen > 0)
	{
		//iRtpLen = iLeaveLen > iMaxPacketLen ? iMaxPacketLen : iLeaveLen;
		iRtpLen = iDataLen;
		pcRtpPacket = &pcRtpData[iDataLen - iLeaveLen];
		cRtpMFlag = (pcRtpPacket[1] >> 7) & 0x1;
		
		cPacketType = (pcRtpPacket[RTP_HEADER_LEN] >> 1) & 0x3F;
		iNaluLen = iRtpLen - RTP_HEADER_LEN; 


		if ((cPacketType != 49))
		{
			memcpy(&pcFrameBuf[iFrameLen], g_acNaluHeader, NALU_HEADER_SIZE);
			iFrameLen += NALU_HEADER_SIZE;
			memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN], iNaluLen);
			iFrameLen += iNaluLen;
		}
		else
		{
			// FU
			unsigned char fu1 = pcRtpPacket[RTP_HEADER_LEN];
			unsigned char fu2 = pcRtpPacket[RTP_HEADER_LEN + 1];
			unsigned char fu3 = pcRtpPacket[RTP_HEADER_LEN + 2];
			unsigned char acNalu[2] = {0, 0};
			
			cNaluStart = (fu3 >> 7) & 0x1;
			cNaluEnd   = (fu3 >> 6) & 0x1;
			cNaluType  = fu3 & 0x3f;

			acNalu[0] = cNaluType << 1;
			acNalu[1] = 1;

			iNaluLen -= H265_FU_SIZE;
			if (1 == cNaluStart)
			{
				memcpy(&pcFrameBuf[iFrameLen], g_acNaluHeader, NALU_HEADER_SIZE);
				iFrameLen += NALU_HEADER_SIZE;
				pcFrameBuf[iFrameLen] = acNalu[0];
				iFrameLen++;
				pcFrameBuf[iFrameLen] = acNalu[1];
				iFrameLen++;
				memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN + H265_FU_SIZE], iNaluLen);
				iFrameLen += iNaluLen;
			}
			else
			{
				memcpy(&pcFrameBuf[iFrameLen], &pcRtpPacket[RTP_HEADER_LEN + H265_FU_SIZE], iNaluLen);
				iFrameLen += iNaluLen;
			}
		}

		iLeaveLen -= iRtpLen;

		if (cRtpMFlag)
		{
			/* frame end	*/
			/*if (!iFrameLostFlag && iReqIFrameFlag)
			{
				char cFrameType = (pcFrameBuf[4] >> 1) & 0x3f;

			    if (((cFrameType & 0x1f) == 0x7) || ((cFrameType & 0x1f) == 0x9))
				{
					// I Frame
					*piReqIFrameFlag = 0;
				}
				else
				{
					iFrameLen = 0;
					*piNaluPos = 0;

					return 0;
				}
				
			}*/
			

			
			if (ptRtspConn->pfRtpSetDataCB && !iFrameLostFlag)
			{
				ptRtspConn->pfRtpSetDataCB(STREAM_TYPE_VIDEO, E_STREAM_TYPE_H265, pcFrameBuf, iFrameLen, uiTs, ptRtspConn->pRtpCbArg);
			}

   
			iFrameLen = 0;

			return 0;
		}
	}

	return (iFrameLen);
}
