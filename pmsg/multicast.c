#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <features.h>

#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/in.h>

#include <netinet/udp.h>
#include <netinet/ip.h>
#include <sys/select.h>
#include <fcntl.h>

#include "multicast.h"

	
int read_interface(const char *acInterface, int *ifindex, void *addr, unsigned char *arp)
{
    int fd;
    struct ifreq ifr;
    struct sockaddr_in *our_ip;
    u_int32_t *pAddr = (u_int32_t *)addr;

    memset(&ifr, 0, sizeof(struct ifreq));
    if((fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) 
    {
        ifr.ifr_addr.sa_family = AF_INET;
        strcpy(ifr.ifr_name, acInterface);

        if (pAddr)
        { 
            if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
            {
                our_ip = (struct sockaddr_in *) &ifr.ifr_addr;
                *pAddr = our_ip->sin_addr.s_addr;
                //printf("%s (our ip) = %s", ifr.ifr_name, 
                 //     inet_ntoa(our_ip->sin_addr));
            } 
            else 
            {
                printf("SIOCGIFADDR failed, is the interface up and configured?: %s\n", strerror(errno));
                close(fd);
                return -1;
            }
        }
        if (ifindex)
        {
            if (ioctl(fd, SIOCGIFINDEX, &ifr) == 0)
            {
                printf("adapter index %d\n", ifr.ifr_ifindex);
                *ifindex = ifr.ifr_ifindex;
            }
            else 
            {
                printf("SIOCGIFINDEX failed!: %s", strerror(errno));
                close(fd);
                return -1;
            }
        }
        if (arp)
        {
            if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)
            {
                memcpy(arp, ifr.ifr_hwaddr.sa_data, 6);
                printf("adapter hardware address %02x:%02x:%02x:%02x:%02x:%02x\n",
                    arp[0], arp[1], arp[2], arp[3], arp[4], arp[5]);
            } 
            else 
            {
                printf("SIOCGIFHWADDR failed!: %s", strerror(errno));
                close(fd);
                return -1;
            }
        }
    }
    else
    {
        printf("socket failed!: %s", strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

int CreateMultcastSocket(unsigned short usMultPort)
{
    int iSockFd = 0;
    int iLen = 0;
    int iOption = 0;  
    int iRet = 0;
    int iFlag = 1;
    socklen_t iAddrLen = sizeof(struct sockaddr_in );
    u_int32_t source = 0;
    struct in_addr inaddr;
    struct sockaddr_in	servaddr;
    struct sockaddr_in	cliaddr;
	
    iRet = read_interface("eth0", NULL, &source, NULL);
    if(iRet != 0)
    {
        return -2;
    }
    
    iSockFd = socket(PF_INET, SOCK_DGRAM, 0);
    if (iSockFd < 0)
    {
        printf("Opening mutlsocket error\n");
        return -1;
    }
	
    iOption = fcntl(iSockFd, F_GETFL, 0);  
    fcntl(iSockFd, F_SETFL, iOption | O_NONBLOCK); 

    inaddr.s_addr = source;//htonl(source);//
    if (setsockopt(iSockFd, IPPROTO_IP, IP_MULTICAST_IF, &inaddr, sizeof(struct in_addr)) < 0)
    {
        printf("fail when setsockopt\n");
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family        = AF_INET;
    servaddr.sin_addr.s_addr   = htonl(INADDR_ANY);
    servaddr.sin_port          = htons(usMultPort);
    
    iRet = setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, (char*)&iFlag, sizeof(int));
    if(iRet != 0)
    {
        printf("ERR: setsockopt socket error. err = %d,errno = %d[%s]\n", iRet, errno, strerror(errno));
        close(iSockFd);
        return -1;
    }
    if (bind(iSockFd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind fail");
        close(iSockFd);
        return -1;
    }
    
    return iSockFd;
}

int AddMulticastAddr(int iSockFd, char *pcMulticastAddr)
{
    struct ip_mreq ipmr;
    int iRet = 0;
    u_int32_t source = 0;
    
    if (iSockFd <= 0)
    {
        return -1;	

    iRet = read_interface("eth0", NULL, &source, NULL);
    if(iRet != 0)
    {
        return -2;
    }}
    
    ipmr.imr_interface.s_addr = source;//htonl(source);//htonl(INADDR_ANY);
    ipmr.imr_multiaddr.s_addr = inet_addr(pcMulticastAddr);
    iRet = setsockopt(iSockFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&ipmr, sizeof(ipmr));
    if (iRet < 0)
    {
        perror("add multi:");	
    }
    
    return iRet;
}

int DropMulticastAddr(int iSockFd, char *pcMulticastAddr)
{
    struct ip_mreq ipmr;
    int iRet = 0;
    u_int32_t source = 0;
    
    if (iSockFd <= 0)
    {
        return -1;	

    iRet = read_interface("eth0", NULL, &source, NULL);
    if(iRet != 0)
    {
        return -2;
    }}
    
    ipmr.imr_interface.s_addr = source;//htonl(source);//htonl(INADDR_ANY);
    ipmr.imr_multiaddr.s_addr = inet_addr(pcMulticastAddr);
    iRet = setsockopt(iSockFd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char*)&ipmr, sizeof(ipmr));
    if (iRet < 0)
    {
        perror("Drop multi:");	
    }
    
    return iRet;
}


int DestroyMultcastSocket(int iSockFd)
{
    if (iSockFd > 0)
    {
        close(iSockFd);
    }
    
    return 0;
}

int SendMultcastPacket(int iSockFd, char *pcMultAddr, unsigned short usMultPort, const char *pcBuf, int iLen)
{
    int iRet = 0;
    struct sockaddr_in address;

    if ((NULL == pcMultAddr) || (NULL == pcBuf) || (0 == iLen))
    {
        return 0;
    }
    
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(pcMultAddr);
    address.sin_port = htons(usMultPort);

    iRet = sendto(iSockFd, pcBuf, iLen, 0, (struct sockaddr *)&address, sizeof(struct  sockaddr));
    
    return iRet;
}
