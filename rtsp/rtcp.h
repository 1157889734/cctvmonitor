#ifndef _RTCP_H_
#define _RTCP_H_

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */
#define LOCAL_SSRC  0x12345678

typedef enum {
    RTCP_SR   = 200,
    RTCP_RR   = 201,
    RTCP_SDES = 202,
    RTCP_BYE  = 203,
    RTCP_APP  = 204
} rtcp_type_t;

typedef enum {
   RTCP_SDES_END   = 0,
   RTCP_SDES_CNAME = 1,
   RTCP_SDES_NAME  = 2,
   RTCP_SDES_EMAIL = 3,
   RTCP_SDES_PHONE = 4,
   RTCP_SDES_LOC   = 5,
   RTCP_SDES_TOOL  = 6,
   RTCP_SDES_NOTE  = 7,
   RTCP_SDES_PRIV  = 8
} rtcp_sdes_type_t;
/*
 * RTCP common header word
*/
typedef struct {
	unsigned char count:5;     /* varies by packet type */
	unsigned char p:1;         /* padding flag */
	unsigned char version:2;   /* protocol version */
	unsigned char pt;			/* RTCP packet type */
	unsigned short length;      /* pkt len in words, w/o this word */
} rtcp_common_t;

/*
 * Big-endian mask for version, padding bit and packet type pair
 */
#define RTCP_VALID_MASK (0xc000 | 0x2000 | 0xfe)
#define RTCP_VALID_VALUE ((RTP_VERSION << 14) | RTCP_SR)

/*
 * Reception report block
 */
typedef struct {
	unsigned int ssrc;             /* data source being reported */	
	
	unsigned int lost:24;				/* cumul. no. pkts lost (signed!) */	
	unsigned int fraction:8;		  /* fraction lost since last SR/RR */			
	
	unsigned int lastSeq;        /*Extended last sequence number received*/	

	unsigned int jitter;           /* inter arrival jitter */
	unsigned int lsr;              /* last SR packet from this source *///从reportee端最后收到的Sender Report中NTP timestamp的中32bits.(无则为0)
	unsigned int dlsr;             /* delay since last SR packet */ //最后收到SR和发送RR之间的间隔,以1/65536为单位(否则为0) 
} rtcp_rr_t;

/*
 * SDES item
 */
typedef struct {
   unsigned char type;              /* type of item (rtcp_sdes_type_t) */
   unsigned char length;            /* length of item (in octets) */
   char data[6];             /* text, not null-terminated */
} rtcp_sdes_item_t;

/*
 * One RTCP packet
 */
typedef struct
{
   rtcp_common_t common;     /* common header */
   union
   {
      /* sender report (SR) */
      struct
	  {
         unsigned int ssrc;     /* sender generating this report */
         unsigned int ntp_sec;  /* NTP timestamp */ // most significant word
         unsigned int ntp_frac;                     // least significant word 
         unsigned int rtp_ts;   /* RTP timestamp */
         unsigned int psent;    /* packets sent */
         unsigned int osent;    /* octets sent */
         rtcp_rr_t rr[1];  /* variable-length list */
       } sr;

       /* reception report (RR) */
       struct 
	   {
          unsigned int ssrc;     /* receiver generating this report */
          rtcp_rr_t rr[1];  /* variable-length list */
       } rr;

       /* source description (SDES) */
       struct rtcp_sdes
	   {
          unsigned int ssrc;      /* first SSRC/CSRC */
          rtcp_sdes_item_t item[1]; /* list of SDES items */
       } sdes;

       /* BYE */
       struct
	   {
          unsigned int src[1];   /* list of sources */
                           /* can't express trailing text for reason */
       } bye;
   } r;
} rtcp_t;


int RtcpSendPacket(PT_RTP_CONN ptRtcpConn, char *pcRtcpData, int iRtcpLen, struct sockaddr *pClientAddr, int iClientAddrLen);
#ifdef WIN
DWORD WINAPI RtcpRecvThread(void *arg);
#else
void *RtcpRecvThread(void *arg);
#endif

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif
