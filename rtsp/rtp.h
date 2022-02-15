#ifndef _RTP_H_
#define _RTP_H_

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */
#define RTP_HEADER_LEN        12           /* Rtp header size           */
#define NALU_HEADER_SIZE      4            /* H264 Nalu header size     */
#define H264_FU_SIZE          2            /* H264 FU size              */
#define H265_FU_SIZE          3            /* H265 FU size              */

typedef struct _T_RTP_HEADER{

   unsigned short cc:4;        // CSRC count	
   unsigned short x:1;         // header extension flag
   unsigned short p:1;         // padding flag
   unsigned short version:2;   // protocol version
   unsigned short pt:7;        // payload type	
   unsigned short m:1;         // marker bit
   unsigned short seq;         // sequence number
   unsigned int ts;            // timestamp
   unsigned int ssrc;          // synchronization savource
   //unsigned int csrc[1];       // optional CSRC list
} T_RTP_HEADER, *PT_RTP_HEADER;


int RtpParseH265Data(PT_RTSP_CONN ptRtspConn, char *pcFrameBuf, int iFrameLen, char *pcRtpData, int iDataLen, int iFrameLostFlag, int *piReqIFrameFlag, unsigned int uiTs);
int RtpParseH264Data(PT_RTSP_CONN ptRtspConn, char *pcFrameBuf, int iFrameLen, char *pcRtpData, int iDataLen, int iFrameLostFlag, int *piReqIFrameFlag, unsigned int uiTs);
int RtpParseMPAData(PT_RTSP_CONN ptRtspConn, char **ppFrame, int iFrameLen, char *pcRtpData, int iDataLen, int iMaxPacketLen); 

#ifdef WIN
DWORD WINAPI RtpRecvThread(void *arg);
#else
void *RtpRecvThread(void *arg);
#endif

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif
