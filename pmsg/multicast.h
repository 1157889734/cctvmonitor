#ifndef  __MULTICAST_H__
#define __MULTICAST_H__

int CreateMultcastSocket(unsigned short usMultPort);
int DestroyMultcastSocket(int iSockFd);
int AddMulticastAddr(int iSockFd, char *pcMulticastAddr);
int DropMulticastAddr(int iSockFd, char *pcMulticastAddr);
int SendMultcastPacket(int iSockFd, char *pcMultAddr, unsigned short usMultPort, const char *pcBuf, int iLen);

#endif
