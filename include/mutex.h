#ifndef MUTEX_H
#define MUTEX_H

#ifdef WIN
    typedef HANDLE Mutex;
#else
    typedef pthread_mutex_t Mutex;
#endif

// initialize mutex
#ifdef WIN
    #define MUTEX_INIT( MutexPtr ) *MutexPtr = CreateMutex( NULL, false, NULL )
#else
    #define MUTEX_INIT( MutexPtr ) pthread_mutex_init( MutexPtr, NULL )
#endif

// lock the mutex variable
#ifdef WIN
    #define MUTEX_LOCK( MutexPtr ) WaitForSingleObject( *MutexPtr, INFINITE )
#else
    #define MUTEX_LOCK( MutexPtr ) pthread_mutex_lock( MutexPtr )
#endif

// unlock the mutex variable
#ifdef WIN
    #define MUTEX_UNLOCK( MutexPtr ) ReleaseMutex( *MutexPtr )
#else
    #define MUTEX_UNLOCK( MutexPtr ) pthread_mutex_unlock( MutexPtr )
#endif


// destroy, making sure mutex is unlocked
#ifdef WIN
    #define MUTEX_DESTROY( MutexPtr ) CloseHandle( *MutexPtr )
#else
    #define MUTEX_DESTROY( MutexPtr )  do {         \
        int rc = pthread_mutex_destroy( MutexPtr ); \
        if ( rc == EBUSY ) {                        \
            MUTEX_UNLOCK( MutexPtr );               \
            pthread_mutex_destroy( MutexPtr );      \
        }                                           \
    } while ( 0 )    
#endif


#endif // MUTEX_H
