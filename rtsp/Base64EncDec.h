#ifndef _BASE64_ENC_DEC_H
#define _BASE64_ENC_DEC_H


int  Base64Encode(unsigned char *inbuf, int inlen, unsigned char *outbuf, int *poutlen);
int  Base64Decode(unsigned char *inbuf, int inlen, unsigned char *outbuf, int *poutlen);

#endif
