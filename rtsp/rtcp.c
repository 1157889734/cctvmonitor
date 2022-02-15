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
#include "rtcp.h"


#if 0
int RtcpSendPacket(int iSocket, char *pcRtcpData, int iRtcpLen, struct sockaddr *pClientAddr, int iClientAddrLen)
{
   /* int iLen = 0;
    
    if ((NULL == pcRtcpData) || (0 == iRtcpLen))
    {
        RTSP_DEBUG("Invalid input param\n");	
        return -1;
    }
    
    if (TCP == ptRtspConn->iRtpTransportProtocol)
    {
        iLen = send(iSocket, pcRtcpData, iRtcpLen, 0);
        if (iLen <= 0)
        {
            return -1;	
        }
    }
    else
    {
        iLen = sendto(iSocket, pcRtcpData, iRtcpLen, 0, pClientAddr, iClientAddrLen);
    }
    
    return iLen;*/
	return 0;
}
#endif



#ifdef WIN
	DWORD WINAPI RtcpRecvThread(void *arg)
#else
  void *RtcpRecvThread(void *arg)
#endif
{
    PT_RTCP_CONN ptRtcpConn      = (PT_RTCP_CONN)arg;
    PT_RTSP_CONN ptRtspConn      = NULL;
    char acRecvBuf[1024];
    char acSendBuf[1024];
    int iRecvLen = 0;
    struct sockaddr rtcpClientAddr;
    int iAddrLen = sizeof(struct sockaddr);
    unsigned int iUdpSRTime=0;

	rtcp_t *rtcp_packet;
	rtcp_t *rtcp_packet2;
    
    
    if (NULL == ptRtcpConn)
    {
        return NULL;
    }
    if (ptRtcpConn->ptRtspConn)
    {
        ptRtspConn = (PT_RTSP_CONN)ptRtcpConn->ptRtspConn;
    }
    
    while(1)
    {
        if (TCP == ptRtspConn->iRtpTransportProtocol)
        {
            goto RTCP_ERROR;
        }
        
        iRecvLen = recvfrom(ptRtcpConn->iRtcpSocket, acRecvBuf, sizeof(acRecvBuf),
                             0, &rtcpClientAddr, (socklen_t *)&iAddrLen);	
				if (iRecvLen <= 0)
				{
				    goto RTCP_ERROR;
				}
#if 1
        if (iRecvLen >= 28)
        {
            rtcp_t *rtcp_recv= (rtcp_t *)acRecvBuf;
            unsigned int ssrc = 0;
			
            rtcp_recv->r.sr.ntp_sec = ntohl(rtcp_recv->r.sr.ntp_sec);		
            rtcp_recv->r.sr.ntp_frac = ntohl(rtcp_recv->r.sr.ntp_frac);

            iUdpSRTime = (rtcp_recv->r.sr.ntp_sec<<16)&0xffff0000;
            iUdpSRTime |= (rtcp_recv->r.sr.ntp_frac>>16)&0x0000ffff;

            ssrc = ntohl(rtcp_recv->r.sr.ssrc);
				
            rtcp_packet = (rtcp_t*)acSendBuf;

            rtcp_packet->common.version = 2;		/* protocol version */
            rtcp_packet->common.p = 0;				/* padding flag */
            rtcp_packet->common.count = 1;			/* varies by packet type */
            rtcp_packet->common.pt = RTCP_RR;		/* RTCP packet type */

            rtcp_packet->common.length = 7;			/* pkt len in words, w/o this word */
            rtcp_packet->common.length = ntohs(rtcp_packet->common.length);

            rtcp_packet->r.rr.ssrc = LOCAL_SSRC;
            rtcp_packet->r.rr.ssrc = ntohl(rtcp_packet->r.rr.ssrc);

            rtcp_packet->r.rr.rr[0].ssrc = ssrc;
            rtcp_packet->r.rr.rr[0].ssrc = ntohl(rtcp_packet->r.rr.rr[0].ssrc);

            rtcp_packet->r.rr.rr[0].fraction = 0;
            rtcp_packet->r.rr.rr[0].lost = 0;

           // rtcp_packet->r.rr.rr[0].seq_count = 10;//g_udpSeqCount;
          //  rtcp_packet->r.rr.rr[0].seq_count = ntohs(rtcp_packet->r.rr.rr[0].seq_count);
           // rtcp_packet->r.rr.rr[0].Hseq_recv = 10;//g_udpPacketRecv;
           // rtcp_packet->r.rr.rr[0].Hseq_recv = ntohs(rtcp_packet->r.rr.rr[0].Hseq_recv);
			rtcp_packet->r.rr.rr[0].lastSeq = htonl(ptRtcpConn->ptRtpConn->iRtpPacketCount);

            rtcp_packet->r.rr.rr[0].jitter = 2000;
            rtcp_packet->r.rr.rr[0].jitter = ntohl(rtcp_packet->r.rr.rr[0].jitter);
            rtcp_packet->r.rr.rr[0].lsr = iUdpSRTime;  
            rtcp_packet->r.rr.rr[0].lsr = ntohl(rtcp_packet->r.rr.rr[0].lsr);
            rtcp_packet->r.rr.rr[0].dlsr = 126000;
            rtcp_packet->r.rr.rr[0].dlsr = ntohl(rtcp_packet->r.rr.rr[0].dlsr);

            rtcp_packet2 = (rtcp_t*)(acSendBuf+sizeof(rtcp_common_t)+sizeof(rtcp_rr_t)+sizeof(unsigned int));

            rtcp_packet2->common.version = 2;		
            rtcp_packet2->common.p = 0;				
            rtcp_packet2->common.count = 1;			
            rtcp_packet2->common.pt = RTCP_SDES;		

            rtcp_packet2->common.length = 4;//6;		
            rtcp_packet2->common.length = ntohs(rtcp_packet2->common.length);

            rtcp_packet2->r.sdes.ssrc = LOCAL_SSRC;
            rtcp_packet2->r.sdes.ssrc = ntohl(rtcp_packet2->r.sdes.ssrc);

            rtcp_packet2->r.sdes.item[0].type = 1;
            rtcp_packet2->r.sdes.item[0].length = 8;

            memset(rtcp_packet2->r.sdes.item[0].data, 0, 10);
            sprintf(rtcp_packet2->r.sdes.item[0].data, "cftc_nvr");

            int len= sizeof(rtcp_common_t)*2+sizeof(rtcp_rr_t)+sizeof(unsigned int)*2+sizeof(rtcp_sdes_item_t);
            sendto(ptRtcpConn->iRtcpSocket, acSendBuf, len, 0, &rtcpClientAddr, iAddrLen);
          //  RtcpSendPacket(ptRtcpConn->iRtcpSocket, rtcpSendBuf, len, &rtcpclientaddr, from_len);
        }
        #endif
    }	
    
RTCP_ERROR:
    if (ptRtcpConn->iRtcpSocket > 0)
    {
        RTSP_Close(ptRtcpConn->iRtcpSocket);
        ptRtcpConn->iRtcpSocket = -1;	
    }
    
    return NULL;
}
