#ifdef WIN
  #include "stdafx.h"
#else
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <pthread.h>
  #include<sys/socket.h>

#endif
#include "rtspComm.h"


void RTSP_Close(int iSocket)
{
    if (iSocket > 0)
    {
    #ifdef WIN
        closesocket(iSocket);
    
    #else
        shutdown(iSocket, SHUT_RDWR);
        close(iSocket);
    
    #endif
    }
}

int RTSP_ThreadJoin(THREAD_HANDLE threadHandle)
{
#ifdef WIN
	if (WAIT_OBJECT_0 == WaitForSingleObject(threadHandle, 2000))
	{
		CloseHandle(threadHandle);
	} 
	else
	{
		DWORD nExitCode;
		GetExitCodeThread(threadHandle, &nExitCode);
		if (STILL_ACTIVE == nExitCode)
		{
			TerminateThread(threadHandle, nExitCode);
			CloseHandle(threadHandle);
		}						
	}
#else
    pthread_join(threadHandle, NULL);
#endif

    return 0;
}


void RTSP_MSleep(int iMsec)
{
#ifdef WIN
    Sleep(iMsec);
#else
    usleep(iMsec * 1000);
#endif
}
