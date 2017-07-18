#ifndef _stdThread_H_2B9D23FC_90BF_4898_B161_B894F01F6698
#define _stdThread_H_2B9D23FC_90BF_4898_B161_B894F01F6698

#include	<stdio.h>
#include	<stdlib.h>
#include	<pthread.h>
//#include	<unistd.h>
#include	<string.h>
//#include	<sys/time.h>
#include	<sys/select.h>
//#include	<netdb.h>
//#include	<sys/socket.h>
//#include	<netinet/in.h>
//#include	<arpa/inet.h>
#include	<errno.h>
//#include	<netinet/tcp.h> 
//#include	<sys/stat.h>
#include	<sys/types.h>
//#include	<fcntl.h>
#include	<limits.h>


#define		SOCKET_ERROR	(-1)
#define		INVALID_SOCKET	(-1)
#define		TRUE			(1)
#define		FALSE			(0)

typedef unsigned char		BYTE;
typedef int      			BOOL;
typedef unsigned int 		UINT;
typedef unsigned short		WORD;
// Linux  :LP64  long:64 int:32
// Windows:LLP64 long:32 int:64
//typedef unsigned long		DWORD;			// Windows
typedef unsigned int		DWORD;			// Linux
typedef BYTE				*LPBYTE;
typedef char				*LPSTR;
typedef const char			*LPCSTR;
typedef char				*LPTSTR;
typedef const char			*LPCTSTR;
typedef signed long long	SInt64;
typedef unsigned long long	UInt64;

typedef int      			SOCKET;


//-----------------------------------------------------------------------
#define min(a,b)			(((a) < (b)) ? (a) : (b))
#define max(a,b)			(((a) > (b)) ? (a) : (b))

#define SAFE_FREE(p)	{ if(p) { free(p); (p)=NULL; } }
#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p)  { if(p) { delete[] (p); (p)=NULL; } }
//-----------------------------------------------------------------------

#endif

