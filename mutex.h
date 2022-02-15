#ifndef __MUTEX_H_H
#define __MUTEX_H_H

#ifdef _WIN32
#include <atlbase.h>
#else
#include <pthread.h>
#include <unistd.h>
typedef void* HANDLE;
#define strtok_s strtok_r
#define Sleep(x) usleep(x*1000)
#endif // _WIN32

// 互拆锁
class CMutexLock
{
    CMutexLock(const CMutexLock &);
    CMutexLock &operator=(const CMutexLock &);
public:
	CMutexLock();
	virtual ~CMutexLock();

	void Lock();
	void Unlock();

private:
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#else
	pthread_mutex_t  mutex;
#endif
};

#endif//__MUTEX_H_H
