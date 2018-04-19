#ifndef _BASE_TYPE_H_
#define _BASE_TYPE_H_

#include <stdarg.h>

#include <pthread.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <sys/timeb.h>
#include <arpa/inet.h>

#include <sys/ipc.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <net/if_arp.h>

#include <sys/socket.h>
#include <sys/ioctl.h>             /* ioctl command */
#include <netinet/if_ether.h>      /* ethhdr struct */
#include <net/if.h>                /* ifreq struct */
#include <netinet/in.h>             /* in_addr structure */
#include <netinet/ip.h>             /* iphdr struct */
#include <netinet/udp.h>            /* udphdr struct */
#include <netinet/tcp.h>            /*tcphdr struct */

#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>

#define UDP_CONNECT
#define UDP_IPADDR   0x7f000001   
#define UDP_PORT     0x5555     

#define DLL_DECLARE  

#define FAST 


typedef signed char				INT8;
typedef unsigned char			UINT8;
typedef signed short			INT16;
typedef unsigned short			UINT16;
typedef signed int				INT32;
typedef unsigned int			UINT32;

typedef unsigned long long		UINT64;
typedef long long		INT64;

typedef signed char				BOOLEN;

typedef intptr_t        int_t;
typedef uintptr_t       uint_t;



#define pthread_info()\
	do{\
		time_t tUniqueName = time(NULL);\
		printf(BLUE"%s"NORMAL, ctime(&tUniqueName));\
		printf(BLUE"PID = %d, PPID = %d,Thread Name: %s\r\n"NORMAL, getpid(),getppid(), __func__);\
		printf(BLUE"Created at line %d, file %s\r\n"NORMAL, __LINE__, __FILE__);\
		printf(BLUE"=================================================================================\r\n"NORMAL);\
	}while(0)




#define SOCKET			  INT32


#define CALLBACK    
#define WINAPI      
#define WINAPIV     
#define APIENTRY    
#define APIPRIVATE  
#define PASCAL   
#define VarINLINE   inline   

#define MAX_ID_LEN      31
#define MAX_DEVNAME_LEN      63
#define MAX_SERVERNAME_LEN      63
#define thread_volatile   volatile
#define true            		1
#define false           		0


#ifdef LOCK_THREAD
	#define VarLock 			    pthread_mutex_t
	#define InitVarLock(lck)		pthread_mutex_init(&(lck), NULL)
	#define LockVarLock(lck)		pthread_mutex_lock(&(lck))
	#define TryLockVarLock(lck)		pthread_mutex_trylock(&(lck))
	#define UnLockVarLock(lck)	    pthread_mutex_unlock(&(lck))
#else
	#define VarLock 	 INT32
	#define InitVarLock(lck)		
	#define LockVarLock(lck)		
	#define TryLockVarLock(lck)		
	#define UnLockVarLock(lck)	   
#endif





#define MAX(a, b)           (((a) > (b)) ? (a) : (b))
#define MIN(a, b)           (((a) < (b)) ? (a) : (b))
#define IPPORT(ip, port) ((ip) >> 0) & 0xFF, ((ip) >> 8) & 0xFF, ((ip) >> 16) & 0xFF, ((ip) >> 24) & 0xFF, ntohs((port))
#define IP(ip) ((ip) >> 0) & 0xFF, ((ip) >> 8) & 0xFF, ((ip) >> 16) & 0xFF, ((ip) >> 24) & 0xFF
//#define repeat(a)   a; a; do { } while(0)
#define repeat(a)   a;

#define write2(s,b,l)           write((s),((char *)b),l)
#define read2(s,b,l)            read((s),((char *)b),l)
#define close2(s)               close((s)); (s) = (SOCKET) -1; do { } while(0)

#define MAX_CLIENTS     4096
#define MAX_BUF_LEN     64
#define MAX_IP_STR_LEN	23
#define STREAMSIZE 1350

#define SERVER_REPLY_NUM  4

#define TEST_DEBUG	0


#define act_on_error(x, s, a)																\
	if ((x) != 0) {                                                                        \
        if (g_pDbgFile != NULL) fprintf(g_pDbgFile, "%s:%d: %s ... ERROR(%d)\n", "client.c", __LINE__, (s), errno);	            \
        if (errno != 0) {                                                             \
            if (g_pDbgFile != NULL) fprintf(g_pDbgFile, "%s:%d: ERROR(%d)\n", "client.c", __LINE__, errno);		\
        }                                                                                   \
        a;                                                                                  \
    } else if (TEST_DEBUG) {                                                                 \
        if (g_pDbgFile != NULL) fprintf(g_pDbgFile, "%s:%d: %s ... ok\n", "client.c", __LINE__, (s));					\
    }                                                                                       \
    do {                                                                                    \
    } while(false)

#define readtimeout(s, socks, t, g)     \
    do {                                \
        Timeout.tv_sec = (t);           \
        Timeout.tv_usec = 0;            \
        FD_ZERO(&(socks));              \
        FD_SET((s), &(socks));          \
        if (select(1+(INT32)(s), &(socks), NULL, NULL, &Timeout) <= 0) goto g;    \
    } while(false)



static void XSleep(INT32 nSec, INT32 nUSec)
{
	struct timeval tv;
	tv.tv_sec = nSec;
	tv.tv_usec = nUSec;
	select(0, NULL, NULL, NULL, &tv);
}

#define cr_printf(a,b,...) 

#endif


