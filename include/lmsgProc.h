#ifndef _MSGPROC_H_
#define _MSGPROC_H_

#ifdef __cplusplus
extern "C" {
#endif

//这些函数的功能是用来与DHMI通讯，用来决定当前显示那个界面
int LMSG_Init(void);
int LMSG_Uninit(void);
int LMSG_SendMsgToDHMI(int iCmd, char *pcBuf, int iLen);

#ifdef __cplusplus
}
#endif

#endif
