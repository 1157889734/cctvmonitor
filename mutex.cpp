#include "mutex.h"
// 互拆锁

CMutexLock::CMutexLock()
{
#ifdef _WIN32
	InitializeCriticalSection(&mutex);
#else
	pthread_mutex_init (&mutex, NULL);
#endif
}
CMutexLock::~CMutexLock()
{
#ifdef _WIN32
	DeleteCriticalSection(&mutex);
#else
	pthread_mutex_destroy (&mutex);
#endif
    
}

void CMutexLock::Lock()
{
#ifdef _WIN32
	EnterCriticalSection(&mutex);
#else
	pthread_mutex_lock (&mutex);
#endif
}
void CMutexLock::Unlock()
{
#ifdef _WIN32
	LeaveCriticalSection(&mutex);
#else
	pthread_mutex_unlock (&mutex);
#endif
    
}
