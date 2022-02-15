#ifndef _GB2312TOUTF8_H
#define _GB2312TOUTF8_H
#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */
void GB2312ToUTF_8(int *pOutLen, char *pOut, char *pText, int pLen, int OutBuflen);
void UTF_8ToGB2312(int *pOutLen, char *pOut, char *pText, int pLen, int OutBuflen);
#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */
#endif