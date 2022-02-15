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

#include "mutex.h"
#include "types.h"
#include "stdio.h"
#include "rtspComm.h"
#include "rtsp.h"
#include "rtp.h"
#include "rtcp.h"
#include "Base64EncDec.h"

void RtspFreeDataBuf(PT_RTSP_BUFF ptRtspBuf)
{
    if (ptRtspBuf)
    {
        if (ptRtspBuf->pcData)
        {
            free(ptRtspBuf->pcData);
            ptRtspBuf->pcData = NULL;
        }
        free(ptRtspBuf);
        ptRtspBuf = NULL;
    }
}

/*******************************************************************************
 * Function:     FreeRequestMsg
 *
 * Description:  This function frees the request message pointer memory.
 *
 * Parameters :
 *      [IN] RTSPRequestMsg *RequestMsg - Pointer of RTSP request message.
 *
 * Return Value:
 *       None
 ******************************************************************************/
void FreeRequestMsg (PT_RTSP_REQUEST_MSG ptRequestMsg)
{
    INT32       indexCnt;            /* Local index counter                   */

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\n[%s]ENTER : \n\tRequestMsg = %p\n", __FUNCTION__, ptRequestMsg);

    /* Free memory used by local variables                                    */
    if( ptRequestMsg )
    {
        /* Free the response msg memory                                       */
        for (indexCnt = 0; indexCnt < ptRequestMsg->ReqHeadersCount; indexCnt++)
        {
            if(ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderName)
                free(ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderName);
            if(ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderValue)
                free(ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderValue);
        }
       /* if( ptRequestMsg->ReqMsgURI )
        {
            free(ptRequestMsg->ReqMsgURI);
            ptRequestMsg->ReqMsgURI = NULL;
        }*/

        ptRequestMsg->ReqHeadersCount = 0;


    }

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nEXIT : \n\tReturn Successfully\n");
}

/*******************************************************************************
 * Function:     FreeResponseMsg
 *
 * Description:  This function frees the response message pointer memory.
 *
 * Parameters :
 *      [IN] RTSPResponseMsg *ResponseMsg - Pointer of RTSP response message.
 *
 * Return Value:
 *       None
 ******************************************************************************/
void FreeResponseMsg(T_RTSP_RESPONSE_MSG *ptResponseMsg)
{
    INT32       indexCnt;            /* Local index counter                   */

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\n[%s]ENTER : \n\tResponseMsg = %p\n", __FUNCTION__, ptResponseMsg);

    if( ptResponseMsg)
    {
        /* Free the response msg memory                                       */
        for (indexCnt = 0; indexCnt < ptResponseMsg->ResHeadersCount; indexCnt++)
        {
            if(ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderName)
            {
                free(ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderName);
                ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderName = NULL;
            }
            if(ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderValue)
            {
                free(ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderValue);
                ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderValue = NULL;
            }
        }

       /* for (indexCnt = 0; indexCnt < ptResponseMsg->ResSdpHeaderCount; indexCnt++)
        {
            if(ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderName)
            {
                free(ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderName);
                ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderName = NULL;
            }
            if(ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderValue)
            {
                free(ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderValue);
                ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderValue = NULL;
            }
        }*/

        if (ptResponseMsg->pSdpValue)
        {
            free(ptResponseMsg->pSdpValue);
            ptResponseMsg->pSdpValue = NULL;
        }

        if (ptResponseMsg->pResRtspVersion)
        {
            free(ptResponseMsg->pResRtspVersion);
            ptResponseMsg->pResRtspVersion = NULL;
        }
        if (ptResponseMsg->pResReasonPhrase)
        {
            free(ptResponseMsg->pResReasonPhrase);
            ptResponseMsg->pResReasonPhrase = NULL;
        }

        free(ptResponseMsg);
        ptResponseMsg = NULL;
    }

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nEXIT : \n\tReturn Successfully\n");
}

void FreeSdpInfo(T_SDP_INFO *ptSdpInfo)
{
    INT32       indexCnt;            /* Local index counter                   */

    if( ptSdpInfo)
    {
        /* Free the response msg memory                                       */
        for (indexCnt = 0; indexCnt < ptSdpInfo->ResSdpHeaderCount; indexCnt++)
        {

            if(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderName)
            {
                free(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderName);
                ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderName = NULL;
            }
            if(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue)
            {
                free(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue);
                ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue = NULL;
            }
        }

        free(ptSdpInfo);
        ptSdpInfo = NULL;
    }

}

static INT32 FindCharInBuffer ( char *Buffer,  char FindChar,
                         char EndChar,  INT32 BufLen )
{
    INT32 retVal   = 0;                 /* Store the return value of function */

    /* Loop to compare the FindChar and EndChar with each member of buffer
       and return the index of find char                                      */
    for (;;)
    {
        if (Buffer[retVal] == FindChar)
        {
            break;
        }
        else if (Buffer[retVal] == EndChar)
        {
            retVal = 0;
            break;
        }
        else
            retVal++;

        /* Reached to end of buffer and encChar or FindChar not found         */
        if (retVal >= BufLen)
        {
            retVal = 0;
            break;
        }
    }
    return retVal;
}


T_SDP_INFO *RtspParseSdp(char *pcBuf, int iLen)
{
    char             *buffPtr;      /* Pointer of received data buffer        */
    INT32            buffLen;       /* Store the length of buffer             */
    INT32            indexCnt = 0;  /* Index counter                          */
    T_SDP_INFO       *ptSdpInfo; /* Holds Parsed Data from Request Message */

    if ((NULL == pcBuf) || (0 == iLen))
    {
        return NULL;
    }

    buffPtr = pcBuf;
    buffLen = iLen;

    /* Allocate Memory to Request Message's pointer                           */
    ptSdpInfo = (T_SDP_INFO *) malloc (sizeof (T_SDP_INFO));
    if (ptSdpInfo == NULL)
    {
        RTSP_DEBUG (DEBUG_RTSP_LEVEL, "Cannot Allocate Memory to Request Message's Pointer\n");
        return NULL;
    }
    memset(ptSdpInfo, 0x00, sizeof(T_SDP_INFO));
    for (;;)
    {
        if (buffLen <= 2)
            break;

        if ((indexCnt = FindCharInBuffer (buffPtr, '=', '\r', buffLen)))
        {
            INT32 iHeaderCount = ptSdpInfo->ResSdpHeaderCount;

            ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName = (char *)malloc (indexCnt + 1);
            if (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName == NULL)
            {
                RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Memory Allocation Failed \n");
                break;
            }

            memset (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName,
                0x00, indexCnt + 1);
            memcpy (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName,
                buffPtr, indexCnt);
            //buffPtr += indexCnt + 1;
            //buffLen -= indexCnt + 1;

            /* Get the value of header                                        */
            if ((indexCnt = FindCharInBuffer (buffPtr, '\r', '\n', buffLen)))
            {
                ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderValue =
                    (char *)malloc (indexCnt + 1);
                if (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderValue == NULL)
                {
                    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Memory Allocation Failed \n");

                    free (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName);
                    ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName = NULL;

                    break;
                }

                memset (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderValue, 0x00, indexCnt + 1);
                memcpy (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderValue, buffPtr, indexCnt);
                buffPtr     += indexCnt + 2;
                buffLen     -= indexCnt + 2;

                RTSP_DEBUG(DEBUG_RTSP_LEVEL, "%s = %s\n",
                    ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName,
                    ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderValue);

                ptSdpInfo->ResSdpHeaderCount++;
            }
            else
            {
                free (ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName);
                ptSdpInfo->ResMsgSdpHeaders[iHeaderCount].pHeaderName = NULL;
                break;
            }
        }
    }

    return ptSdpInfo;
}


T_RTSP_RESPONSE_MSG *RtspParseResponseMsg(char *pcBuf, int iLen)
{
    char             *tempArray;    /* Temp buffer pointer                    */
    char             *buffPtr;      /* Pointer of received data buffer        */
    INT32            buffLen;       /* Store the length of buffer             */
    INT32            indexCnt = 0;  /* Index counter                          */
    T_RTSP_RESPONSE_MSG   *ptResponseMsg; /* Holds Parsed Data from Request Message */

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nRtspParseMessage ENTER : \n\t");

    if ((NULL == pcBuf) || (0 == iLen))
    {
        return NULL;
    }

    buffPtr = pcBuf;
    buffLen = iLen;

    /* Allocate Memory to Request Message's pointer                           */
    ptResponseMsg = (T_RTSP_RESPONSE_MSG *) malloc (sizeof (T_RTSP_RESPONSE_MSG));
    if (ptResponseMsg == NULL)
    {
        RTSP_DEBUG (DEBUG_RTSP_LEVEL, "Cannot Allocate Memory to Request Message's Pointer\n");
        return NULL;
    }
    memset(ptResponseMsg, 0x00, sizeof(T_RTSP_RESPONSE_MSG));

    /* Get rtsp version from buffer and store it in to the message structure    */
    if ((indexCnt = FindCharInBuffer (buffPtr, ' ', '\r', buffLen)))
    {
        buffLen  -= indexCnt + 1;
        tempArray = (char*)malloc (indexCnt + 1);
        if(tempArray != NULL)
        {
            memset (tempArray, 0x00, indexCnt + 1);
            memcpy (tempArray, buffPtr, indexCnt);
            buffPtr              += indexCnt + 1;
            ptResponseMsg->pResRtspVersion = tempArray;

        }
        else
        {
            RTSP_DEBUG (DEBUG_RTSP_LEVEL, "Cannot Allocate Memory for tempArray Pointer\n");
            free(ptResponseMsg);
            ptResponseMsg = NULL;
            return NULL;
        }
    }
    else
    {
        free (ptResponseMsg);
        ptResponseMsg = NULL;
        return NULL;
    }

    /* Get rtsp response status code from buffer */
    if ((indexCnt = FindCharInBuffer (buffPtr, ' ', '\r', buffLen)))
    {
        buffLen  -= indexCnt + 1;
        tempArray = (char*)malloc (indexCnt + 1);
        if(tempArray != NULL)
        {
            memset (tempArray, 0x00, indexCnt + 1);
            memcpy (tempArray, buffPtr, indexCnt);
            buffPtr              += indexCnt + 1;
            ptResponseMsg->ResStatusCode = atoi(tempArray);
            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Requested Status code = %d\n", ptResponseMsg->ResStatusCode);
            free(tempArray);
            tempArray = NULL;

        }
        else
        {
            if (ptResponseMsg->pResReasonPhrase)
            {
                free(ptResponseMsg->pResReasonPhrase);
                ptResponseMsg->pResReasonPhrase = NULL;
            }
            free(ptResponseMsg);
            ptResponseMsg = NULL;
            return NULL;
        }
    }
    else
    {
        if (ptResponseMsg->pResReasonPhrase)
        {
            free(ptResponseMsg->pResReasonPhrase);
            ptResponseMsg->pResReasonPhrase = NULL;
        }
        free(ptResponseMsg);
        ptResponseMsg = NULL;
        return NULL;
    }

    /* Get rtsp response ReasonPhrase from buffer */
    if ((indexCnt = FindCharInBuffer (buffPtr, '\r', '\n', buffLen)))
    {
        buffLen  -= indexCnt + 2;
        tempArray = (char*)malloc (indexCnt + 1);
        if(tempArray != NULL)
        {
            memset (tempArray, 0x00, indexCnt + 1);
            memcpy (tempArray, buffPtr, indexCnt);
            buffPtr              += indexCnt + 2;
            ptResponseMsg->pResReasonPhrase = tempArray;
            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Response reason  %s\n", ptResponseMsg->pResReasonPhrase);

        }
        else
        {
            if (ptResponseMsg->pResRtspVersion)
            {
                free(ptResponseMsg->pResRtspVersion);
                ptResponseMsg->pResRtspVersion = NULL;
            }
            free(ptResponseMsg);
            ptResponseMsg = NULL;
            return NULL;
        }
    }
    else
    {
        if (ptResponseMsg->pResRtspVersion)
        {
            free(ptResponseMsg->pResRtspVersion);
            ptResponseMsg->pResRtspVersion = NULL;
        }
        free(ptResponseMsg);
        ptResponseMsg = NULL;
        return NULL;
    }

    /* Now all next request contain the headers so loop to extract name and
       value of each header                                                   */
    for (;;)
    {
        if (buffLen <= 2)
            break;
        if (buffPtr[0] == '\r')
        {
            buffPtr += 2;
            buffLen -= 2;
        }

        /* Get the name of header                                             */
        if ((indexCnt = FindCharInBuffer (buffPtr, ':', '\r', buffLen)))
        {
            INT32 iHeaderCount = ptResponseMsg->ResHeadersCount;

            ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName = (char *)malloc (indexCnt + 1);
            if (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName == NULL)
            {
                RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Memory Allocation Failed \n");
                break;
            }

            memset (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName,
                0x00, indexCnt + 1);
            memcpy (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName,
                buffPtr, indexCnt);
            buffPtr += indexCnt + 2;
            buffLen -= indexCnt + 2;

            /* Get the value of header                                        */
            if ((indexCnt = FindCharInBuffer (buffPtr, '\r', '\n', buffLen)))
            {


                ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderValue =
                    (char *)malloc (indexCnt + 1);
                if (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderValue == NULL)
                {
                    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Memory Allocation Failed \n");

                    free (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName);
                    ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName = NULL;

                    break;
                }

                memset (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderValue, 0x00, indexCnt + 1);
                memcpy (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderValue, buffPtr, indexCnt);
                buffPtr     += indexCnt + 2;
                buffLen     -= indexCnt + 2;

                RTSP_DEBUG(DEBUG_RTSP_LEVEL, "%s = %s\n",
                    ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName,
                    ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderValue);
                if (!strcmp
                    ((char*)ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName,
                    "CSeq"))
                {
                    ptResponseMsg->ResSeqNumber =
                        atoi ((char*)ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderValue);
                }
                ptResponseMsg->ResHeadersCount++;
            }
            else
            {
                free (ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName);
                ptResponseMsg->ResMsgHeaders[iHeaderCount].pHeaderName = NULL;
                break;
            }
        }
        else if ((indexCnt = FindCharInBuffer (buffPtr, '=', '\r', buffLen)))
        {
            ptResponseMsg->pSdpValue = (char *)malloc(buffLen);
            memset(ptResponseMsg->pSdpValue, 0, buffLen);
            memcpy(ptResponseMsg->pSdpValue, buffPtr, buffLen);
            ptResponseMsg->iSdpLen = buffLen;
            break;
        }

    }


    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nEXIT : \n\tRequestMsg = %p \n", ptResponseMsg);
    return ptResponseMsg;
}

int GetVideoTrackId(PT_RTSP_CONN ptRtspConn, T_SDP_INFO *ptSdpInfo)
{
	const char *mtag = "m=video";
	const char *ctag = "a=control";
	char *ptr = NULL;
	int indexCnt = 0;
	int iFlag = 0;
	int iTagFlag = 0;

    for (indexCnt = 0; indexCnt < ptSdpInfo->ResSdpHeaderCount; indexCnt++)
    {
        if( !strncmp(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, mtag,strlen(mtag)))
        {
            iFlag = 1;
        }
        if(!strncmp(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, ctag, strlen(ctag)) && iFlag)
        {
            ptr = strchr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, ':');
            ptr++;
            while( *ptr == ' ' ) ptr++;

            // ???¡¤????/??
            if (ptr)
            {
                int i;
                int len = strlen(ptr);
                char acBuf[256];
                int findFlag=0;

                strcpy(acBuf, ptr);

                for (i=len-1;i>=0;--i)
                {
                    if (acBuf[i]=='/')
                    {
                        findFlag=1;
                        strcpy(ptRtspConn->acVideoTrackId, &acBuf[i+1]);
                        break;
                    }
                }
                if (0 == findFlag)
                    strcpy(ptRtspConn->acVideoTrackId, acBuf);
            }

			iTagFlag |= 0x1;
			if (0x3 == iTagFlag)
			{
				return 0;
                     }
		}
              if(!strncmp(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, "a=rtpmap", 8) && iFlag)
		{
			if (strstr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, "H264") ||strstr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, "h264"))
			{
				ptRtspConn->iVideoStreamType = E_STREAM_TYPE_H264;
			}
			else if (strstr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, "H265") ||strstr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, "h265"))
			{
				ptRtspConn->iVideoStreamType = E_STREAM_TYPE_H265;
			}
			else
			{
				ptRtspConn->iVideoStreamType = E_STREAM_TYPE_H264;
			}
			
			iTagFlag |= 0x2;
			if (0x3 == iTagFlag)
			{
				return 0;
                     }
              }

    }

    return -1;
}

int GetAudioTrackId(PT_RTSP_CONN ptRtspConn, T_SDP_INFO *ptSdpInfo)
{
    const char *mtag = "m=audio";
    const char *ctag = "a=control";
    char *ptr = NULL;
    int indexCnt = 0;
    int iFlag = 0;

    for (indexCnt = 0; indexCnt < ptSdpInfo->ResSdpHeaderCount; indexCnt++)
    {
        if( !strncmp(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, mtag,strlen(mtag)))
        {
            iFlag = 1;
        }
        if(!strncmp(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, ctag, strlen(ctag)) && iFlag)
        {
            ptr = strchr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, ':');
            ptr++;
            while( *ptr == ' ' ) ptr++;
            if (ptr)
            {
                int i;
                int len = strlen(ptr);
                char acBuf[256];
                int findFlag=0;

                strcpy(acBuf, ptr);

                for (i=len-1;i>=0;--i)
                {
                    if (acBuf[i]=='/')
                    {
                        findFlag=1;
                        strcpy(ptRtspConn->acAudioTrackId, &acBuf[i+1]);
                        break;
                    }
                }
                if (0 == findFlag)
                    strcpy(ptRtspConn->acAudioTrackId, acBuf);
            }

            return 0;
        }

    }

    return -1;
}

int GetSpsPpsData(PT_RTSP_CONN ptRtspConn, T_SDP_INFO *ptSdpInfo)
{
    const char *mtag = "a=fmtp";
    const char *pRanageTag = "a=range";
    char *ptr = NULL;
    int indexCnt = 0;

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s] %d\n", __FUNCTION__, ptSdpInfo->ResSdpHeaderCount);
    for (indexCnt = 0; indexCnt < ptSdpInfo->ResSdpHeaderCount; indexCnt++)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "%s\n", ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue);
        if( !strncmp(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, mtag,strlen(mtag)))
        {

            ptr = strstr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, "sprop-parameter-sets=");
            if (ptr)
            {
                unsigned char acSpsBuf[512];
                unsigned char acPpsBuf[512];
                int iSpsLen = 0, iPpsLen = 0;
                char *pcStartPos = ptr + strlen("sprop-parameter-sets=");
                char *pcEndPos = NULL;
                char *pcSpsSrc = NULL;
                char *pcPpsSrc = NULL;
                int iSpsSrcLen = 0, iPpsSrcLen = 0;

                pcEndPos = strstr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, ",");
                if (pcEndPos && (pcEndPos > pcStartPos))
                {
                    char acNalHead[4] = {0, 0, 0, 1};
                    int iSpsPpsLen = 0;

                    // sps
                    memset(acSpsBuf, 0, sizeof(acSpsBuf));
                    pcSpsSrc = pcStartPos;
                    iSpsSrcLen = pcEndPos - pcStartPos;
                    Base64Decode((unsigned char *)pcSpsSrc, iSpsSrcLen, acSpsBuf, &iSpsLen);

                    memset(ptRtspConn->acSpsPpsData, 0, sizeof(ptRtspConn->acSpsPpsData));
                    memcpy(ptRtspConn->acSpsPpsData, acNalHead, 4);
                    iSpsPpsLen = 4;
                    memcpy(&ptRtspConn->acSpsPpsData[iSpsPpsLen], acSpsBuf, iSpsLen);
                    iSpsPpsLen += iSpsLen;

                    //pps
                    pcStartPos = pcEndPos + 1;
                    pcEndPos = ptr + strlen(ptr) - 1;
                    if (';' == *pcEndPos)
                    {
                        pcEndPos--;
                    }
                    memset(acPpsBuf, 0, sizeof(acPpsBuf));
                    pcPpsSrc = pcStartPos;
                    iPpsSrcLen = pcEndPos - pcStartPos + 1;
                    Base64Decode((unsigned char *)pcPpsSrc, iPpsSrcLen, acPpsBuf, &iPpsLen);

                    memcpy(&ptRtspConn->acSpsPpsData[iSpsPpsLen], acNalHead, 4);
                    iSpsPpsLen += 4;
                    if (iPpsLen)
                    {
                        memcpy(&ptRtspConn->acSpsPpsData[iSpsPpsLen], acPpsBuf, iPpsLen);
                    }
                    iSpsPpsLen += iPpsLen;
                    ptRtspConn->iSpsPpsLen = iSpsPpsLen;

                    /*sprintf(str,"iSpsPpsLen %d\n", ptRtspConn->iSpsPpsLen);
                    OutputDebugStringA(str);
                    for(i = 0; i < iSpsPpsLen; i++)
                    {
                        char str[20];
                        sprintf(str,"%x ", ptRtspConn->acSpsPpsData[i]);
                        OutputDebugStringA(str);
                    }*/
                }
                continue;

            }

        }

        if(!strncmp(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, pRanageTag,strlen(pRanageTag)))
        {
            ptr = strstr(ptSdpInfo->ResMsgSdpHeaders[indexCnt].pHeaderValue, "-");
            if (ptr)
            {
                char *pcStartPos = ptr + 1;
                ptRtspConn->iPlayRange = atoi(pcStartPos);
            }
        }

    }

    return -1;
}

char* GetResponseMsgHeader(T_RTSP_RESPONSE_MSG  *ptResponseMsg, char *HeaderName)
{
    INT32 indexCnt;                 /* Local counter                          */

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nENTER : \n\tRequestMsg = %p \n\tHeaderName = %s",
                        ptResponseMsg, HeaderName);

    if (ptResponseMsg)
    {
        for(indexCnt=0; indexCnt < ptResponseMsg->ResHeadersCount; indexCnt++)
        {
            if (!strcmp ((char*)ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderName, (char*)HeaderName))
            {
                RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nEXIT : \n\tMsgHeader Value = %s\n",
                    ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderValue);

                return ptResponseMsg->ResMsgHeaders[indexCnt].pHeaderValue;
            }
        }
    }

    return NULL;
}
#if 0
char* GetResponseMsgSdpHeader(T_RTSP_RESPONSE_MSG  *ptResponseMsg, char *HeaderName)
{
    INT32 indexCnt;                 /* Local counter                          */

    RTSP_DEBUG("\nENTER : \n\tRequestMsg = %d \n\tHeaderName = %s",
                        ptResponseMsg, HeaderName);

    if (ptResponseMsg)
    {
        for(indexCnt=0; indexCnt < ptResponseMsg->ResSdpHeaderCount; indexCnt++)
        {
            if (!strcmp ((char*)ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderName, (char*)HeaderName))
            {
                RTSP_DEBUG("\nEXIT : \n\tMsgHeader Value = %s\n",
                    ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderValue);

                return ptResponseMsg->ResMsgSdpHeaders[indexCnt].pHeaderValue;
            }
        }
    }

    return NULL;
}
#endif

INT32 AddRequestHeader (T_RTSP_REQUEST_MSG *ptRequestMsg, char *HeaderName, char *HeaderValue)
{

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nENTER : \n\tResponseMsg = %p\n\tHeaderName = %s\n\tHeaderValue = %s\n",
        ptRequestMsg, HeaderName, HeaderValue);

    /* Maximum Number of Header Fields are added or not                       */
    if (ptRequestMsg->ReqHeadersCount == MAX_HEADER_NUMBER)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Invalid header count of response message\n");
        printf("Invalid header count of response message\n");
        return RET_ERROR;
    }

    /* Store Name of Header Field in MsgHeader                                */
    ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderName =
                                         (char *) malloc (strlen((char*)HeaderName) + 1);

    if( ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderName == NULL )
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Can not allocate memory for response header name\n");
        printf("Can not allocate memory for response header name\n");

        return RET_ERROR;
    }
    memset(ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderName, 0x00,
                                                       strlen((char*)HeaderName) + 1);
    memcpy (ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderName,
                                               HeaderName, strlen ((char*)HeaderName));

    /* Store Value of Header Field in MsgHeader                               */
    ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderValue =
                                        (char *) malloc (strlen((char*)HeaderValue) + 1);

    if( ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderValue == NULL )
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Can not allocate memory for response header value\n");
        printf("Can not allocate memory for response header value\n\n");

        free(ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderName);
        ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderName = NULL;
        return RET_ERROR;
    }
    memset(ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderValue, 0x00,
                                                         strlen((char*)HeaderValue) + 1);
    memcpy (ptRequestMsg->ReqMsgHeaders[ptRequestMsg->ReqHeadersCount].pHeaderValue,
                                             HeaderValue, strlen ((char*)HeaderValue));

    /* Increment the Message Header Count                                     */
    ptRequestMsg->ReqHeadersCount++;

//    printf("Return Successfully==%d\n\n",ptRequestMsg->ReqHeadersCount);

    RTSP_DEBUG(DEBUG_RTSP_LEVEL,  "\nEXIT : \n\tReturn Successfully\n");

    return RET_OK;
}

int GetRequestMethod(INT32 iReqMethod, char *pcMethodString, INT32 iLen)
{
    switch(iReqMethod)
    {
        case RTSP_METHOD_OPTIONS:
        {
            strncpy(pcMethodString, "OPTIONS", iLen);
            break;
        }
        case RTSP_METHOD_DESCRIBE:
        {
            strncpy(pcMethodString, "DESCRIBE", iLen);
            break;
        }
        case RTSP_METHOD_SETUP:
        {
            strncpy(pcMethodString, "SETUP", iLen);
            break;
        }
        case RTSP_METHOD_PLAY:
        {
            strncpy(pcMethodString, "PLAY", iLen);
            break;
        }
        case RTSP_METHOD_PAUSE:
        {
            strncpy(pcMethodString, "PAUSE", iLen);
            break;
        }
        case RTSP_METHOD_TEARDOWN:
        {
            strncpy(pcMethodString, "TEARDOWN", iLen);
            break;
        }
        case RTSP_METHOD_GET_PARAMETER:
        {
            strncpy(pcMethodString, "GET_PARAMETER", iLen);
            break;
        }
        default:
        {

            return -1;
        }

    }

    return 0;
}

INT32 RtspSendRequest (PT_RTSP_CONN ptRtspConn)
{
    char    *pcRequestBuf = NULL;       /* RTSP request buffer pointer           */
    INT32   requestSize = 0;            /* RTSP response size                     */
    INT32   indexCnt = 0;               /* index counter                          */
    char acReqMethod[32] = {0};
    PT_RTSP_REQUEST_MSG ptRequestMsg = &ptRtspConn->tRequestMsg;

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nENTER : \n\tClientConn = %p\n", ptRtspConn);

    if (ptRtspConn->iRtspSocket <= 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "RtspSendRequest send error! rtsp socket closed\n");
        return -1;
    }

    GetRequestMethod(ptRequestMsg->ReqMethod, acReqMethod, sizeof(acReqMethod));
    /* Calculating Request Message size */
    requestSize = strlen(acReqMethod) + strlen(ptRequestMsg->pReqMsgURI) +
        sizeof("RTSP/1.0") + (INT32) strlen("  \r\n");

    for (indexCnt = 0; indexCnt < ptRequestMsg->ReqHeadersCount; indexCnt++)
    {
        requestSize += (INT32) strlen ((char*)ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderName)  +
                        (INT32) strlen ((char*)ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderValue) +
                        (INT32) strlen (": \r\n");
    }
    requestSize += (INT32) strlen("\r\n") + 1;

    /* Allocating memory to rtspResponse                                      */
    pcRequestBuf = (char *) malloc (requestSize);

    if( pcRequestBuf == NULL )
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Can not allocate memory for response string\n");
        return RET_ERROR;
    }
    memset(pcRequestBuf, 0x00, requestSize);

    requestSize = sprintf ((char*)pcRequestBuf, "%s %s %s\r\n",
                            acReqMethod,
                            ptRequestMsg->pReqMsgURI,
                            DEFAULT_RTSP_VERSION);

    /* Loop to add all response header in the response                        */
    for (indexCnt = 0; indexCnt < ptRequestMsg->ReqHeadersCount; indexCnt++)
    {
        requestSize +=sprintf ((char*)(pcRequestBuf + requestSize), "%s: %s\r\n",
                                ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderName,
                                ptRequestMsg->ReqMsgHeaders[indexCnt].pHeaderValue);
    }
    requestSize += sprintf ((char*)(pcRequestBuf + requestSize), "\r\n");
    requestSize  =  strlen((char*)pcRequestBuf);

    /* Send Response to the respective client                                 */
    /* MSG_NOSIGNAL flag to diasble SIGPIPE, in case of Socket error while
       attempting to write on a closed destination socket                     */
    MUTEX_LOCK(&ptRtspConn->tMutex);
    if(send (ptRtspConn->iRtspSocket, (const char*)pcRequestBuf, requestSize , 0) == -1 )
    {
        MUTEX_UNLOCK(&ptRtspConn->tMutex);

        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "RtspSendRequest send error errno=0x%x, %s\n", errno, strerror(errno));
        free(pcRequestBuf);
        pcRequestBuf = NULL;
        RTSP_Close(ptRtspConn->iRtspSocket);
        ptRtspConn->iRtspSocket = -1;
        return RET_ERROR;
    }
    MUTEX_UNLOCK(&ptRtspConn->tMutex);

    /* Free the memory                                                        */
    free(pcRequestBuf);
    pcRequestBuf = NULL;

    FreeRequestMsg(ptRequestMsg);

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "\nEXIT : \n\tReturn Successfully\n");
    return RET_OK;
}

#ifdef WIN
    DWORD WINAPI RtspRecvThread(void *arg)
#else
  void *RtspRecvThread(void *arg)
#endif
{
    PT_RTSP_CONN ptRtspConn = (PT_RTSP_CONN)arg;
    char acRecvBuf[10240];
    char cChIdentifier = 0;
    char *pcRtpData = NULL;
    char *pcVideoFrame = NULL;
    char *pcAudioFrame = NULL;
    int iVideoFrameLen = 0;
    int iAudioFrameLen = 0;
    int iPreLeaveLen = 0;
    int iOffset = 0;
    int iRecvLen = 0;
    unsigned int iLeaveLen = 0;
    unsigned int iRtpLen = 0;
    char *pcData = NULL;
    char *pcLeaveBuf = NULL;
    BOOL bRtpRtcpFlag = FALSE;
    UINT16 u16OldSeq            = 0;
    int iFrameLostFlag          = 0;
    int iSpsFlag = 0;
    int iErrorFlag = 0;


    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[RtspRecvThread] enter\n");

    if (TCP == ptRtspConn->iRtpTransportProtocol)
    {
        pcVideoFrame = (char *)malloc(820000);
        if (NULL == pcVideoFrame)
        {
            goto RTSP_RECV_ERROR;
        }

        pcAudioFrame = (char *)malloc(4096);
        if (NULL == pcAudioFrame)
        {
            goto RTSP_RECV_ERROR;
        }

    }

    pcLeaveBuf = (char *)malloc(10240);
    if (NULL == pcLeaveBuf)
    {
        goto RTSP_RECV_ERROR;
    }

    while (ptRtspConn->iRtspThreadRunFlag)
    {
        if ((E_RTSP_STATE_PLAY != ptRtspConn->iRtspState) && (E_RTSP_STATE_PAUSE != ptRtspConn->iRtspState)
            && (E_RTSP_STATE_TEARDOWN != ptRtspConn->iRtspState))
        {
              RTSP_MSleep(20);
              continue;
        }

        if (iPreLeaveLen == (sizeof(acRecvBuf) - 1))
        {
            DebugPrint(DEBUG_RTSP_LEVEL, "preleavelen %d", iPreLeaveLen);
            iPreLeaveLen = 0;
        }

        iRecvLen = recv(ptRtspConn->iRtspSocket, &acRecvBuf[iPreLeaveLen], sizeof(acRecvBuf) - iPreLeaveLen - 1, 0);
        //RTSP_DEBUG(DEBUG_NORMAL_PRINT, "[%s:%d] recv return %d, sizeof(acRecvBuf)=%d, iPreLeaveLen=%d\n", __FUNCTION__, __LINE__, iRecvLen, sizeof(acRecvBuf), iPreLeaveLen);
        if (iRecvLen <= 0)
        {
            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d] recv return %d, sizeof(acRecvBuf)=%d, iPreLeaveLen=%d\n", __FUNCTION__, __LINE__, iRecvLen, sizeof(acRecvBuf), iPreLeaveLen);

            /*if ((errno == EAGAIN) || (errno == EINTR))
            {
                RTSP_DEBUG(DEBUG_ERROR_PRINT, "[%s:%d]recv error, recvlen %d, errno=0x%x, %s\n", __FUNCTION__, __LINE__,iRecvLen, errno, strerror(errno));
                continue;
            }*/

            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]recv error, recvlen %d, errno=0x%x, %s\n", __FUNCTION__, __LINE__,iRecvLen, errno, strerror(errno));
            //perror(":");
            // procsee error
            goto RTSP_RECV_ERROR;

        }

        iLeaveLen = iPreLeaveLen + iRecvLen;
        iOffset = 0;

        //if (TCP == ptRtspConn->iRtpTransportProtocol)
        {
            char *pcBuf = NULL;

            if (iPreLeaveLen > 0)
            {
                memcpy(acRecvBuf, pcLeaveBuf, iPreLeaveLen);
            }

            pcBuf = acRecvBuf;
            iPreLeaveLen = 0;

            while (iLeaveLen > 0)
            {
                if (iLeaveLen < 4)
                {
                    memcpy(pcLeaveBuf, &pcBuf[iOffset], iLeaveLen);
                    iPreLeaveLen = iLeaveLen;
                    bRtpRtcpFlag = TRUE;
                    break;
                }

                pcData = &pcBuf[iOffset];

                if((pcData[0] & 0xFF) == '$')
                {
                    unsigned char data1, data2;

                    data1 = (unsigned char)pcData[2];
                    data2 = (unsigned char)pcData[3];
                    cChIdentifier = pcData[1];
                    iRtpLen = pcData[2];
                    iRtpLen = (data1 << 8) | (data2 & 0xFF);//(iRtpLen << 8) | pcData[3];

//                    RTSP_DEBUG(DEBUG_RTSP, "[%s:%d] iRtpLen=%d\n", __FUNCTION__, __LINE__, iRtpLen + 4);
                    if (iLeaveLen < (iRtpLen + 4))
                    {
                        memcpy(pcLeaveBuf, &pcBuf[iOffset], iLeaveLen);
                        iPreLeaveLen = iLeaveLen;
                        bRtpRtcpFlag = TRUE;
                        break;
                    }
                    else
                    {
                        pcRtpData = &pcBuf[iOffset + 4];
                        iOffset += iRtpLen + 4;
                        iLeaveLen -= iRtpLen + 4;
                        iPreLeaveLen = 0;
                    }

                    if ((cChIdentifier % 2) == 0)
                    {
                        // rtp
                        PT_RTP_HEADER ptRtpHeader   = NULL;
                        char cCurPayloadType        = 0;

                        ptRtpHeader = (PT_RTP_HEADER)pcRtpData;
                        cCurPayloadType = ptRtpHeader->pt;
                        ptRtpHeader->ts = ntohl(ptRtpHeader->ts);
                        ptRtpHeader->seq = ntohs(ptRtpHeader->seq);
                        if (0 == iSpsFlag)
                        {

							/*if ((TCP == ptRtspConn->iRtpTransportProtocol) && (ptRtspConn->iSpsPpsLen))
							{
                                DebugPrint(DEBUG_RTSP_9, "Send h264 sps frame, frame len %d\n", ptRtspConn->iSpsPpsLen);
								ptRtspConn->pfRtpSetDataCB(STREAM_TYPE_VIDEO, ptRtspConn->acSpsPpsData, ptRtspConn->iSpsPpsLen, ptRtpHeader->ts, ptRtspConn->pRtpCbArg);
							}*/
							iSpsFlag = 1;
						}

						if (96 != cCurPayloadType)
						{
						//	TRACE("aaaa %d\n", cCurPayloadType);
						}

                        if ((98 == cCurPayloadType) || (96 == cCurPayloadType))
                        {
                            if ((ptRtpHeader->seq != u16OldSeq + 1) && (u16OldSeq != 0))//(ptRtpHeader->seq != 0))
                            {
                                iVideoFrameLen = 0;
                                iFrameLostFlag = 1;

                            }
                            u16OldSeq = ptRtpHeader->seq;

                            // ???I?????
                            if (iVideoFrameLen + iRtpLen > 800000)
                            {
                                iVideoFrameLen = 0;
                                iFrameLostFlag = 1;
                            }
							if (E_STREAM_TYPE_H265 == ptRtspConn->iVideoStreamType)
							{
								// h265
								iVideoFrameLen = RtpParseH265Data(ptRtspConn, pcVideoFrame, iVideoFrameLen, pcRtpData, iRtpLen, iFrameLostFlag, NULL, ptRtpHeader->ts);
								if (0 == iVideoFrameLen)
								{
									iFrameLostFlag = 0;
								}
							}
							else
							{
                                iVideoFrameLen = RtpParseH264Data(ptRtspConn, pcVideoFrame, iVideoFrameLen, pcRtpData, iRtpLen, iFrameLostFlag, NULL, ptRtpHeader->ts);
                                if (0 == iVideoFrameLen)
                                {
                                    iFrameLostFlag = 0;
                                }
							}
						}

                        else if (14 == cCurPayloadType)
                        {

                        }
                    }
                    else
                    {
                        // rtcp
                        rtcp_t *rtcp_recv= (rtcp_t *)pcRtpData;
                        unsigned int ssrc = 0;
                        unsigned int iUdpSRTime=0;
                        rtcp_t *rtcp_packet;
                        rtcp_t *rtcp_packet2;
                        char acSendBuf[1024];

                        memset(acSendBuf, 0, sizeof(acSendBuf));
                        rtcp_recv->r.sr.ntp_sec = ntohl(rtcp_recv->r.sr.ntp_sec);
                        rtcp_recv->r.sr.ntp_frac = ntohl(rtcp_recv->r.sr.ntp_frac);

                        iUdpSRTime = (rtcp_recv->r.sr.ntp_sec<<16)&0xffff0000;
                        iUdpSRTime |= (rtcp_recv->r.sr.ntp_frac>>16)&0x0000ffff;

                        ssrc = ntohl(rtcp_recv->r.sr.ssrc);

                        rtcp_packet = (rtcp_t*)(acSendBuf + 4);

                        rtcp_packet->common.version = 2;		/* protocol version */
                        rtcp_packet->common.p = 0;				/* padding flag */
                        rtcp_packet->common.count = 1;			/* varies by packet type */
                        rtcp_packet->common.pt = RTCP_RR;		/* RTCP packet type */

                        rtcp_packet->common.length = 7;			/* pkt len in words, w/o this word */
                        rtcp_packet->common.length = htons(rtcp_packet->common.length);

                        rtcp_packet->r.rr.ssrc = LOCAL_SSRC;
                        rtcp_packet->r.rr.ssrc = htonl(rtcp_packet->r.rr.ssrc);

                        rtcp_packet->r.rr.rr[0].ssrc = ssrc;
                        rtcp_packet->r.rr.rr[0].ssrc = htonl(rtcp_packet->r.rr.rr[0].ssrc);

                        rtcp_packet->r.rr.rr[0].fraction = 0;
                        rtcp_packet->r.rr.rr[0].lost = 0;

                        // rtcp_packet->r.rr.rr[0].seq_count = 10;//g_udpSeqCount;
                        //  rtcp_packet->r.rr.rr[0].seq_count = ntohs(rtcp_packet->r.rr.rr[0].seq_count);
                        // rtcp_packet->r.rr.rr[0].Hseq_recv = 10;//g_udpPacketRecv;
                        // rtcp_packet->r.rr.rr[0].Hseq_recv = ntohs(rtcp_packet->r.rr.rr[0].Hseq_recv);
                        rtcp_packet->r.rr.rr[0].lastSeq = 0;//htonl(ptRtcpConn->ptRtpConn->iRtpPacketCount);

                        rtcp_packet->r.rr.rr[0].jitter = 2000;
                        rtcp_packet->r.rr.rr[0].jitter = htonl(rtcp_packet->r.rr.rr[0].jitter);
                        rtcp_packet->r.rr.rr[0].lsr = iUdpSRTime;
                        rtcp_packet->r.rr.rr[0].lsr = htonl(rtcp_packet->r.rr.rr[0].lsr);
                        rtcp_packet->r.rr.rr[0].dlsr = 126000;
                        rtcp_packet->r.rr.rr[0].dlsr = htonl(rtcp_packet->r.rr.rr[0].dlsr);

                        rtcp_packet2 = (rtcp_t*)(acSendBuf + 4 +sizeof(rtcp_common_t)+sizeof(rtcp_rr_t)+sizeof(unsigned int));

                        rtcp_packet2->common.version = 2;
                        rtcp_packet2->common.p = 0;
                        rtcp_packet2->common.count = 1;
                        rtcp_packet2->common.pt = RTCP_SDES;

                        rtcp_packet2->common.length = 3;//6;
                        rtcp_packet2->common.length = htons(rtcp_packet2->common.length);

                        rtcp_packet2->r.sdes.ssrc = LOCAL_SSRC;
                        rtcp_packet2->r.sdes.ssrc = htonl(rtcp_packet2->r.sdes.ssrc);

                        rtcp_packet2->r.sdes.item[0].type = 1;
                        rtcp_packet2->r.sdes.item[0].length = 4;

                        memset(rtcp_packet2->r.sdes.item[0].data, 0, 6);
                        sprintf(rtcp_packet2->r.sdes.item[0].data, "cftc");

                        int len= sizeof(rtcp_common_t)*2+sizeof(rtcp_rr_t)+sizeof(unsigned int)*2+sizeof(rtcp_sdes_item_t);
                        acSendBuf[0] = '$';
                        acSendBuf[1] = cChIdentifier;
                        acSendBuf[2] = (len >> 8) & 0xff;
                        acSendBuf[3] = len & 0xff;

                        MUTEX_LOCK(&ptRtspConn->tMutex);
                        len = send (ptRtspConn->iRtspSocket, acSendBuf, len + 4 , 0);
                        MUTEX_UNLOCK(&ptRtspConn->tMutex);

                        //  RtcpSendPacket(PT_RTSP_CONN ptRtspConn, char *pcRtcpData, int iRtcpLen, struct sockaddr *pClientAddr, int iClientAddrLen);
                    }

                    continue;
                }
                else
                {
                    char *ptr = NULL;
                    char acTmp[10240];
                    int iTmpLen = iLeaveLen;
                    char cPrevStr = 0;
                    unsigned char ucChId = 0;

                    memset(acTmp, 0, sizeof(acTmp));
                    ptr = pcData;
                    if (0 == iErrorFlag)
                    {
                        while(iLeaveLen > 0)
                        {
                            iLeaveLen--;
                            iOffset++;

                            if (('\r' == cPrevStr) && ('\n' == *ptr))
                            {
                                break;
                            }
                            cPrevStr = *ptr;
                            ptr++;
                        }

                        memcpy(acTmp, pcData, iTmpLen - iLeaveLen);

                        if ((0 == iLeaveLen) && ('\r' != cPrevStr) && ('\n' != *ptr))
                        {
                            cPrevStr = 0;
                            iPreLeaveLen = 0;
                            iErrorFlag = 1;
                            DebugPrint(DEBUG_RTSP_LEVEL, "rtsp response error\n");
                        }
                    }
                    else
                    {
                        while (iLeaveLen > 0)
                        {
                            iLeaveLen--;
                            iOffset++;
                            ucChId = *ptr;
                            if (('$' == cPrevStr) && ucChId < 4)
                            {
                                iLeaveLen += 2;
                                iOffset -= 2;
                                iErrorFlag = 0;
                                DebugPrint(DEBUG_RTSP_LEVEL, "rtsp response error restore, %x, %x\n", cPrevStr, ucChId);
                                break;
                            }
                            cPrevStr = *ptr;
                            ptr++;
                        }
                    }
                }
            }
        }

        if (bRtpRtcpFlag)
        {
            continue;
        }

        /* Procsee rtsp response */


      //  RTSPResponseMsg *RtspParseMessage(acRecvBuf, iRecvLen);
     //   pcHeaderValue = GetResponseMsgHeader(ptResponseMsg, "Session");


    }

RTSP_RECV_ERROR:

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[RtspRecvThread] exit\n");

    if (pcVideoFrame)
    {
        free(pcVideoFrame);
        pcVideoFrame = NULL;
    }

    if (pcAudioFrame)
    {
        free(pcAudioFrame);
        pcAudioFrame = NULL;
    }

    if (pcLeaveBuf)
    {
        free(pcLeaveBuf);
        pcLeaveBuf = NULL;
    }

    if (ptRtspConn->iRtspSocket > 0)
    {
        RTSP_Close(ptRtspConn->iRtspSocket);
        ptRtspConn->iRtspSocket = -1;
    }

    return 0;
}

T_RTSP_BUFF *RtspGetResponse(PT_RTSP_CONN ptRtspConn)
{
    PT_RTSP_BUFF ptRtspBuf = NULL;
    int iRecvLen = 0;
    fd_set	tAllSet, tTmpSet;
    struct timeval tv;

    ptRtspBuf = (PT_RTSP_BUFF)malloc(sizeof(T_RTSP_BUFF));
    if (NULL == ptRtspBuf)
    {
        return NULL;
    }

    ptRtspBuf->pcData = (char *)malloc(MAX_TCP_DATA_SIZE);
    if (NULL == ptRtspBuf->pcData)
    {
        free(ptRtspBuf);
        return NULL;
    }

    memset(ptRtspBuf->pcData, 0, MAX_TCP_DATA_SIZE);

    FD_ZERO(&tAllSet);
    FD_SET(ptRtspConn->iRtspSocket, &tAllSet);
    


    while (1)
    {
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        tTmpSet = tAllSet;	//??????¦Ë.
        if (select(ptRtspConn->iRtspSocket + 1, &tTmpSet, NULL, NULL, &tv) > 0)
        {
            if (FD_ISSET(ptRtspConn->iRtspSocket, &tTmpSet))
            {
                iRecvLen = recv (ptRtspConn->iRtspSocket, (char*)ptRtspBuf->pcData, MAX_TCP_DATA_SIZE, 0);
                if (iRecvLen <= 0)
                {
                    if ((errno == EAGAIN) || (errno == 4))
                    {
                        continue;
                    }

                    if (ptRtspBuf->pcData)
                    {
                        free(ptRtspBuf->pcData);
                        ptRtspBuf->pcData = NULL;
                    }
                    if (ptRtspBuf)
                    {
                        free(ptRtspBuf);
                        ptRtspBuf = NULL;
                    }

                    return NULL;
                }
            }
        }
        else
        {
            if (ptRtspBuf->pcData)
            {
                 free(ptRtspBuf->pcData);
                 ptRtspBuf->pcData = NULL;
            }
            if (ptRtspBuf)
            {
                 free(ptRtspBuf);
                 ptRtspBuf = NULL;
            }

            return NULL;
        }
        break;
    }

    ptRtspBuf->iDataLen = iRecvLen;

    return ptRtspBuf;
}

int ParseHostFromUri(const char *pcUri, char *pcHost, int *piServPost)
{
    char *pcBuf = NULL;
    int iLen = 0;
    int indexCnt = 0;

    pcBuf = (char *)pcUri;
    iLen = strlen(pcUri) + 1;

    /* rtsp:// */
    pcBuf += 7;
    iLen -= 7;

    /* parse host */
    if ((indexCnt = FindCharInBuffer (pcBuf, ':', '/', iLen )))
    {
        memcpy(pcHost, pcBuf, indexCnt);
        pcHost[indexCnt] = 0;
        pcBuf += indexCnt + 1;
        iLen -= indexCnt + 1;
    }
    else
    {
        if ((indexCnt = FindCharInBuffer (pcBuf, '/', 0, iLen )))
        {
            memcpy(pcHost, pcBuf, indexCnt);
            pcHost[indexCnt] = 0;
            pcBuf += indexCnt + 1;
            iLen -= indexCnt + 1;
        }
        else
        {
            strcpy(pcHost, pcBuf);
        }

        return 0;
    }

    /* parse port */
    if ((indexCnt = FindCharInBuffer (pcBuf, '/', 0, iLen )))
    {
        char acPort[12];

        memset(acPort, 0, sizeof(acPort));
        memcpy(acPort, pcBuf, indexCnt);
        *piServPost = atoi(acPort);
    }


    return 0;
}

int RtspConnect(PT_RTSP_CONN ptRtspConn, const char *pcUri)
{
    struct sockaddr_in ServAddr;
    int iRet = 0;

    if (0 == strncmp(pcUri, "rtsp://", 7)) //rtsp://192.168.0.105:8554/live.sdp
    {
        strncpy(ptRtspConn->acUri, pcUri, sizeof(ptRtspConn->acUri));
    }
    else
    {
        sprintf(ptRtspConn->acUri, "rtsp://%s", pcUri);
    }

    iRet = ParseHostFromUri(ptRtspConn->acUri, ptRtspConn->acServHost, &ptRtspConn->iRtspPort);
    if (iRet < 0)
    {
        return -1;
    }

    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "url %s Host %s, Port %d\n", ptRtspConn->acUri,ptRtspConn->acServHost, ptRtspConn->iRtspPort);

    if (0 == ptRtspConn->iRtspPort)
    {
        ptRtspConn->iRtspPort = 554;
    }

    ptRtspConn->iRtspSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ptRtspConn->iRtspSocket < 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Create rtsp socket error\n");
        return -1;
    }

    //set timeout
#ifdef WIN
    int timeout = 5000;

#else
	struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

#endif
    if(setsockopt(ptRtspConn->iRtspSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout))==-1)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Set SO_SNDTIMEO error\n");
        RTSP_Close(ptRtspConn->iRtspSocket);
        ptRtspConn->iRtspSocket = -1;
        return -4;
    }
    if(setsockopt(ptRtspConn->iRtspSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout))==-1)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Set SO_RCVTIMEO error\n");
        RTSP_Close(ptRtspConn->iRtspSocket);
        ptRtspConn->iRtspSocket = -1;
        return -4;
    }

    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = inet_addr(ptRtspConn->acServHost);
    ServAddr.sin_port = htons((UINT16)(ptRtspConn->iRtspPort));

    iRet = connect(ptRtspConn->iRtspSocket, (struct sockaddr *)&ServAddr, sizeof(ServAddr));
    if(iRet != 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Connet error\n");
        RTSP_Close(ptRtspConn->iRtspSocket);
        ptRtspConn->iRtspSocket = -1;
        return -1;
    }

    return 0;
}

int RtspRequestOptions(PT_RTSP_CONN ptRtspConn)
{
    int iRet = 0;
    PT_RTSP_BUFF ptRtspBuf = NULL;
    PT_RTSP_RESPONSE_MSG ptResponseMsg = NULL;


    ptRtspConn->tRequestMsg.ReqMethod = RTSP_METHOD_OPTIONS;
    ptRtspConn->tRequestMsg.pReqMsgURI = ptRtspConn->acUri;

    iRet = RtspSendRequest(ptRtspConn);
    if (iRet < 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp send request error\n", __FUNCTION__, __LINE__);
        return -1;
    }
    ptRtspConn->iRtspState = E_RTSP_STATE_OPTION;

    ptRtspBuf = RtspGetResponse(ptRtspConn);
    if ((NULL == ptRtspBuf) || (0 == ptRtspBuf->iDataLen))
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Rtsp get option response msg error\n");
        if (ptRtspBuf)
        {
            RtspFreeDataBuf(ptRtspBuf);
            ptRtspBuf = NULL;
        }

        return -1;
    }

    ptResponseMsg = RtspParseResponseMsg(ptRtspBuf->pcData, ptRtspBuf->iDataLen);

    /* process responsemsg */


    FreeResponseMsg(ptResponseMsg);
    ptResponseMsg = NULL;

    RtspFreeDataBuf(ptRtspBuf);
    ptRtspBuf = NULL;

    return 0;
}

int RtspRequestDescribe(PT_RTSP_CONN ptRtspConn)
{
    int iRet = RET_OK;
    T_RTSP_BUFF *ptRtspBuf = NULL;
    T_RTSP_RESPONSE_MSG *ptResponseMsg = NULL;
    char *pcHeaderValue = NULL;

    ptRtspConn->tRequestMsg.ReqMethod = RTSP_METHOD_DESCRIBE;
    ptRtspConn->tRequestMsg.pReqMsgURI = ptRtspConn->acUri;

    iRet = RtspSendRequest(ptRtspConn);
    if (iRet < 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp send request error\n", __FUNCTION__, __LINE__);
        return -1;
    }
    ptRtspConn->iRtspState = E_RTSP_STATE_DESCRIBE;

    //RTSP_MSleep(500);

    ptRtspBuf = RtspGetResponse(ptRtspConn);
    if ((NULL == ptRtspBuf) || (0 == ptRtspBuf->iDataLen))
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Rtsp get discribe response msg error\n");
        if (ptRtspBuf)
        {
            RtspFreeDataBuf(ptRtspBuf);
            ptRtspBuf = NULL;
        }

        return -1;
    }

    ptResponseMsg = RtspParseResponseMsg(ptRtspBuf->pcData, ptRtspBuf->iDataLen);

    /* process responsemsg */
    if (ptResponseMsg->ResStatusCode == 401)
    {
        // ???????
        pcHeaderValue = GetResponseMsgHeader(ptResponseMsg, "WWW-Authenticate");
        if (pcHeaderValue)
        {
            char *pcPos = NULL;
            int indexCnt = 0;
            int iLen = 0;

            if (strstr(pcHeaderValue, "Digest"))
            {
                ptRtspConn->iAuthorizeFlag = AUTHORIZATE_TYPE_DIGEST;
                pcPos = strstr(pcHeaderValue, "realm");
                if (pcPos)
                {
                    pcPos += 7; // realm="
                    iLen = strlen(pcPos);
                    if ((indexCnt = FindCharInBuffer (pcPos, '"', '\n', iLen )))
                    {
                        memcpy(ptRtspConn->acAuthRealm, pcPos, indexCnt);
                        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "auth realm %s\n", ptRtspConn->acAuthRealm);
                    }
                }

                pcPos = strstr(pcHeaderValue, "nonce");
                if (pcPos)
                {
                    pcPos += 7; // nonce="
                    iLen = strlen(pcPos);
                    if ((indexCnt = FindCharInBuffer (pcPos, '"', '\n', iLen )))
                    {
                        memcpy(ptRtspConn->acAuthNonce, pcPos, indexCnt);
                        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "auth nonce %s\n", ptRtspConn->acAuthNonce);
                    }
                }
            }
            else if (strstr(pcHeaderValue, "Basic"))
            {
                ptRtspConn->iAuthorizeFlag = AUTHORIZATE_TYPE_BASIC;
            }
        }

        iRet = RET_UNAUTHORIZATION;
    }
    pcHeaderValue = GetResponseMsgHeader(ptResponseMsg, "Content-Base");
    if (pcHeaderValue)
    {
        strcpy(ptRtspConn->acNewUri, pcHeaderValue);
    }
    pcHeaderValue = GetResponseMsgHeader(ptResponseMsg, "Content-Length");
    if (NULL == pcHeaderValue)
    {
        pcHeaderValue = GetResponseMsgHeader(ptResponseMsg, "Content-length");
    }
    RTSP_DEBUG(DEBUG_RTSP_LEVEL, "sdplen %s\n", pcHeaderValue);

    if (pcHeaderValue)
    {
        int iSdpLen = atoi(pcHeaderValue);
        T_SDP_INFO *ptSdpInfo = NULL;

        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "sdplen %d recv sdpLen %d\n", iSdpLen, ptResponseMsg->iSdpLen);

        if ((ptResponseMsg->iSdpLen < iSdpLen) && (iSdpLen > 0))
        {
            T_RTSP_BUFF *ptSdpBuf = NULL;

            ptSdpBuf = RtspGetResponse(ptRtspConn);
            if (NULL == ptSdpBuf)
            {
                RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp get response msg error\n", __FUNCTION__, __LINE__);
                FreeResponseMsg(ptResponseMsg);
                ptResponseMsg = NULL;

                RtspFreeDataBuf(ptRtspBuf);
                ptRtspBuf = NULL;
                return -1;
            }
            ptResponseMsg->pSdpValue = (char *)realloc(ptResponseMsg->pSdpValue, iSdpLen + 1);

            memcpy(&ptResponseMsg->pSdpValue[ptResponseMsg->iSdpLen], ptSdpBuf->pcData, iSdpLen - ptResponseMsg->iSdpLen);
            ptResponseMsg->pSdpValue[iSdpLen] = 0;
            ptResponseMsg->iSdpLen = iSdpLen;

            //ParseSdp
            ptSdpInfo = RtspParseSdp(ptResponseMsg->pSdpValue, ptResponseMsg->iSdpLen);

            RtspFreeDataBuf(ptSdpBuf);
            ptSdpBuf = NULL;
        }
        else
        {
            ptSdpInfo = RtspParseSdp(ptResponseMsg->pSdpValue, ptResponseMsg->iSdpLen);
        }

        if (ptSdpInfo)
        {
            GetVideoTrackId(ptRtspConn, ptSdpInfo);

            GetAudioTrackId(ptRtspConn, ptSdpInfo);

			GetSpsPpsData(ptRtspConn, ptSdpInfo);
			FreeSdpInfo(ptSdpInfo);
			ptSdpInfo = NULL;
		}

    }

    FreeResponseMsg(ptResponseMsg);
    ptResponseMsg = NULL;

    RtspFreeDataBuf(ptRtspBuf);
    ptRtspBuf = NULL;

    return iRet;
}

int RtspRequestSetup(PT_RTSP_CONN ptRtspConn, int iTrackType)
{
    int iRet = 0;
    char *pcHeaderValue = NULL;
    char acUri[256];
    T_RTSP_BUFF *ptRtspBuf = NULL;
    T_RTSP_RESPONSE_MSG *ptResponseMsg = NULL;

    memset(acUri, 0, sizeof(acUri));
    if (E_VIDEO_TRACK == iTrackType)
    {
        if (strstr(ptRtspConn->acVideoTrackId, "rtsp"))
        {
            sprintf(acUri, "%s", ptRtspConn->acVideoTrackId);
        }
        else
        {
            sprintf(acUri, "%s/%s", ptRtspConn->acUri, ptRtspConn->acVideoTrackId);
        }
    }
    else if (E_AUDIO_TRACK == iTrackType)
    {
        if (strstr(ptRtspConn->acAudioTrackId, "rtsp"))
        {
            sprintf(acUri, "%s", ptRtspConn->acAudioTrackId);
        }
        else
        {
            sprintf(acUri, "%s/%s", ptRtspConn->acUri, ptRtspConn->acAudioTrackId);
        }
    }

    ptRtspConn->tRequestMsg.ReqMethod = RTSP_METHOD_SETUP;
    ptRtspConn->tRequestMsg.pReqMsgURI = acUri;

    iRet = RtspSendRequest(ptRtspConn);
    if (iRet < 0)
    {
          RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp send request error\n", __FUNCTION__, __LINE__);
          return -5;
    }

    ptRtspConn->iRtspState = E_RTSP_STATE_SETUP;

    ptRtspBuf = RtspGetResponse(ptRtspConn);
    if ((NULL == ptRtspBuf) || (0 == ptRtspBuf->iDataLen))
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Rtsp get setup response msg error\n");
        if (ptRtspBuf)
        {
            RtspFreeDataBuf(ptRtspBuf);
            ptRtspBuf = NULL;
        }
        return -6;
    }

    ptResponseMsg = RtspParseResponseMsg(ptRtspBuf->pcData, ptRtspBuf->iDataLen);
    if (NULL == ptResponseMsg)
    {
        RtspFreeDataBuf(ptRtspBuf);
        ptRtspBuf = NULL;
        return -7;
    }

    /* rtsp setup response error */
    if (200 != ptResponseMsg->ResStatusCode)
    {
        FreeResponseMsg(ptResponseMsg);
        ptResponseMsg = NULL;

        RtspFreeDataBuf(ptRtspBuf);
        ptRtspBuf = NULL;

        return -8;
    }

    /* process response messsage */
    pcHeaderValue = GetResponseMsgHeader(ptResponseMsg, "Session");
    if (pcHeaderValue)
    {
        if (strstr(pcHeaderValue, "timeout"))
        {
            char *pcFirst = NULL;
            char *pcSecond = NULL;

            pcFirst = strstr(pcHeaderValue, ";");
            if (NULL != pcFirst)
            {
                memcpy(ptRtspConn->acSession, pcHeaderValue, (pcFirst - pcHeaderValue));
            }

            pcSecond = strstr(pcHeaderValue, "=");
            if (pcSecond)
            {
                ptRtspConn->iRtspTimeout = atoi(pcSecond + 1);
            }
            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Session: %s, Timeout = %d\n", ptRtspConn->acSession, ptRtspConn->iRtspTimeout);
        }
        else
        {
            sprintf(ptRtspConn->acSession, "%s", pcHeaderValue);
            RTSP_DEBUG(DEBUG_RTSP_LEVEL, "Session: %s\n", ptRtspConn->acSession);
        }
    }
    else
    {
        FreeResponseMsg(ptResponseMsg);
        ptResponseMsg = NULL;

        RtspFreeDataBuf(ptRtspBuf);
        ptRtspBuf = NULL;

        return -1;
    }

    FreeResponseMsg(ptResponseMsg);
    ptResponseMsg = NULL;

    RtspFreeDataBuf(ptRtspBuf);
    ptRtspBuf = NULL;

    return 0;
}

int RtspRequestPlay(PT_RTSP_CONN ptRtspConn)
{
    int iRet = 0;

    ptRtspConn->tRequestMsg.ReqMethod = RTSP_METHOD_PLAY;
    ptRtspConn->tRequestMsg.pReqMsgURI = ptRtspConn->acUri;

    iRet = RtspSendRequest(ptRtspConn);
    if (iRet < 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp send request error\n", __FUNCTION__, __LINE__);
        return -1;
    }

    /*ptRtspBuf = RtspGetResponse(ptRtspConn);
    if (NULL == ptRtspBuf)
    {
        RTSP_DEBUG(RTSP_DEBUG_LEVEL, "[%s:%d]Rtsp get response msg error\n", __FUNCTION__, __LINE__);
        return -6;
    }

    RtspFreeDataBuf(ptRtspBuf);
    ptRtspBuf = NULL;*/

    ptRtspConn->iRtspState = E_RTSP_STATE_PLAY;

    return 0;
}

int RtspRequestPause(PT_RTSP_CONN ptRtspConn)
{
    int iRet = 0;

    ptRtspConn->tRequestMsg.ReqMethod = RTSP_METHOD_PAUSE;
    ptRtspConn->tRequestMsg.pReqMsgURI = ptRtspConn->acUri;

      iRet = RtspSendRequest(ptRtspConn);
      if (iRet < 0)
      {
          RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp send request error\n", __FUNCTION__, __LINE__);
          return -1;
      }
      ptRtspConn->iRtspState = E_RTSP_STATE_PAUSE;

    return 0;
}

int RtspRequestTeardown(PT_RTSP_CONN ptRtspConn)
{
    int iRet = 0;

    ptRtspConn->tRequestMsg.ReqMethod = RTSP_METHOD_TEARDOWN;
    ptRtspConn->tRequestMsg.pReqMsgURI = ptRtspConn->acUri;


    iRet = RtspSendRequest(ptRtspConn);
    if (iRet < 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp send request error\n", __FUNCTION__, __LINE__);
        return -1;
    }
    ptRtspConn->iRtspState = E_RTSP_STATE_TEARDOWN;

    return 0;
}

int RtspRequestGetParameter(PT_RTSP_CONN ptRtspConn)
{
    int iRet = 0;

    ptRtspConn->tRequestMsg.ReqMethod = RTSP_METHOD_GET_PARAMETER;
    ptRtspConn->tRequestMsg.pReqMsgURI = ptRtspConn->acUri;

    iRet = RtspSendRequest(ptRtspConn);
    if (iRet < 0)
    {
        RTSP_DEBUG(DEBUG_RTSP_LEVEL, "[%s:%d]Rtsp send request error\n", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

