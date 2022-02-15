#ifndef	__COMMON_TYPES_H__
#define	__COMMON_TYPES_H__

#ifdef __cplusplus
extern "C"{
#endif /* End of #ifdef __cplusplus */

#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#include	<ctype.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/ioctl.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<math.h>
#include	<inttypes.h>
#include	<sched.h>
#include	<sys/mman.h>
#include	<sys/time.h>
#include	<sys/timeb.h>
#include	<linux/ioctl.h>
#include	<time.h>
#include	<pthread.h>

typedef unsigned char           BYTE;
typedef unsigned short		WORD;
typedef int                     BOOL;
typedef unsigned int		UINT;
typedef long int		LONG;
typedef unsigned long           DWORD;
#define MIN(a,b) ((a < b) ? a : b)
#define MAX(a,b) ((a > b) ? a : b)
typedef void*                   LPOVERLAPPED;
typedef void*                   OVERLAPPED;
typedef void*                   LPVOID;
typedef void*                   PVOID;
typedef void*			DLL_HANDLE;
//typedef void                    VOID;

typedef char			TCHAR;
typedef char			INT8;
typedef char			CHAR;
typedef signed char		S8;
typedef unsigned char           UCHAR;
typedef unsigned char		UINT8;
typedef unsigned char*		PU8;
typedef unsigned char		U8;
typedef	unsigned char		uchar_t;

typedef short			INT16;
typedef short int		SHORT;
typedef signed short		S16;
typedef unsigned short		U16;
typedef unsigned short		UINT16;
typedef unsigned short          USHORT;
typedef	unsigned short		ushort_t;

typedef int                     HANDLE;         // note that handle here is assumed to be
typedef int                     BOOLEAN;
typedef int			PT_FILEHANDLE;
typedef int*                    PHANDLE;
typedef unsigned long		U32;
typedef	unsigned int		uint_t;

typedef	long			INT32;
typedef signed long		S32;
typedef unsigned long           ULONG;
typedef unsigned long		UINT32;
typedef unsigned long*          LPDWORD;

typedef long long               		INT64;
typedef unsigned long long         UINT64;



/* Define NULL pointer value */
#ifndef NULL
	#ifdef __cplusplus
		#define NULL    0
	#else
		#define NULL    ((void *)0)
	#endif
#endif


#define	Sleep	sleep



#ifndef	SUCCESS
	#define SUCCESS         0
#endif
#ifndef	FAILURE
	#define FAILURE        -1
#endif



#define STATIC		static

#ifndef	TRUE
	#define TRUE		1
#endif

#ifndef	FALSE
	#define FALSE		0
#endif

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif	//__COMMON_TYPES_H__
