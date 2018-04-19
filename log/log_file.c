/*******************************************************************************
	> File Name: .h
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "BaseType.h"

#define TRUE	(1)
#define FALSE	(0)


#include "log_file.h"

//前景色:30（黑色）、31（红色）、32（绿色）、 33（黄色）、34（蓝色）、35（洋红）、36（青色）、37（白色）

#define CONSOLE_LOG_EPOLL_WAIT_TIMEOUT		(60*1000)

#define EPEVENT_SIZE_MAX					(8)
#define CONSOLE_LOG_UNIX_DOMAIN				("./console_log.socket")
#define CONSOLE_LOG_FILE_PATH				("./console_log.log")

#define LOG_FILE_SAVE_SIZE_MAX				(256*1024)  
#define LOG_FILE_PATH_SIZE_MAX				(128)

#define LOG_BUFFER_SIZE_MAX					(1024)

/**
	1.直接使用 0打印（默认） 0保存
	2.本地组合使用 C端 -> S端 打印/保存
	3.网络服务+
*/


typedef struct _tagLogFileInfo 
{
	FILE	*pFile;
	int		m_iOffset;
	char	aucPath[LOG_FILE_PATH_SIZE_MAX];
} T_LogFileInfo;

typedef struct _tagTraceLevel
{
	unsigned char	m_ucLevel;
	const char		*pHeader;
} T_TraceLevel;

T_TraceLevel g_tLogLVTable[] = 
{
	{LOG_FATAL     , "FATAL"},
	{LOG_ERROR     , "ERROR"},
	{LOG_WARNING   , "WARN "},
	{LOG_DEBUG     , "DEBUG"},
	{LOG_INFO      , "INFO "},
	{LOG_TRACE     , "TRACE"},
	{LOG_MAX       , "NONE"},
};


typedef struct _tagLogServerInfo
{   
	BOOLEN			flsEnable; 		
	volatile BOOLEN	flsKeepRunning;	
    pthread_t 		pThrId;

	INT32			m_iEpoll;

	T_LogFileInfo	*pLogInfo;
	/* unix socket */
	INT32			m_tUnSocket;		//-1
	INT32			m_tUnCliSocket;		//-1
	
	struct sockaddr_un in_tCliAddr;		//客户端发送的时候使用
	
	UINT8			m_aucBuf[LOG_BUFFER_SIZE_MAX];
	
	/* net socket */
	INT32			m_tNetSocket;	

	//NET_PROCESSRECVDATA	pRcvCbk;  //共用本地与网络

}T_LogServerInfo, *PT_LogServerInfo;

T_LogServerInfo	g_tLogServerInfo = {0};


typedef void (*LOG_FUNC_TB)(unsigned char *_pData, int _iLen);

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static char buf_time[32];

static char *getLogTime()
{
    struct timeval time;
    struct tm *tmp;

    /* Get Current time */
    gettimeofday(&time, NULL);
    tmp = localtime(&time.tv_sec);
    sprintf(buf_time, "%04d-%02d-%02d %02d:%02d:%02d:%03d", 
						tmp->tm_year + 1900, tmp->tm_mon + 1, tmp->tm_mday,
						tmp->tm_hour, tmp->tm_min, tmp->tm_sec, (int)(time.tv_usec/1000));
    return buf_time;
}

static void log_fatal(unsigned char *_pData, int _iLen)
{
	int iLength = _iLen;
	if (NULL == _pData) return;
	if (iLength <= 0) iLength = strlen(_pData);
	
    fprintf(stdout, "[\033[1;33;40m%s\033[m] \033[1;33;40m%s \033[m\n", getLogTime(), _pData); //33（黄色）
}

static void log_error(unsigned char *_pData, int _iLen)
{
    fprintf(stdout, "[\033[1;33;40m%s\033[m] \033[1;31;40m%s \033[m\n", getLogTime(), _pData); //31（红色）
}

static void log_warning(unsigned char *_pData, int _iLen)
{
    fprintf(stdout, "[\033[1;33;40m%s\033[m] \033[1;35;40m%s \033[m\n", getLogTime(), _pData); //35（洋红）
}

static void log_debug(unsigned char *_pData, int _iLen)
{
    fprintf(stdout, "[\033[1;33;40m%s\033[m] \033[1;36;40m%s \033[m\n", getLogTime(), _pData); //36（青色）
}

static void log_info(unsigned char *_pData, int _iLen)
{
    fprintf(stdout, "[\033[1;33;40m%s\033[m] \033[1;32;40m%s\033[m\n", getLogTime(), _pData); //32（绿色）
}

static void log_trace(unsigned char *_pData, int _iLen)
{
    fprintf(stdout, "[\033[1;33;40m%s\033[m] %s\n", getLogTime(), _pData); 
}

const LOG_FUNC_TB g_pLogFuncTable[LOG_MAX] = 
{
	log_fatal,
	log_error,
	log_warning,
	log_debug,
	log_info,
	log_trace,
};
/**************************************************************/
static int client_log_printf(unsigned char *_pData, int _iLen) 
{
	PT_LogServerInfo pServer = &g_tLogServerInfo;
	int iRet;
	if (NULL == _pData) return -1;
	if (_iLen <= 0) _iLen = strlen(_pData);
	
	if (0 != pthread_mutex_lock(&log_mutex)) 
	{
		perror("log mutex lock");
		return -1;
	}
	/* _pData[0] = level 这里没有时间戳*/
    iRet = sendto(pServer->m_tUnCliSocket, _pData, _iLen, 0, (struct sockaddr *)&pServer->in_tCliAddr, sizeof(pServer->in_tCliAddr));
	if(iRet < 0) 
	{
		
	}
	pthread_mutex_unlock(&log_mutex);

	return iRet;
}

void console_log_printf(unsigned char _ucLevel, const char* _pFormat, ...) 
{
	unsigned char aucBuf[LOG_BUFFER_SIZE_MAX] = {0};
	int iRet = 0, iLen = 0;
	
	if (_ucLevel > LOG_TRACE) return;
	
	iRet = sprintf(aucBuf, "%X[%s] ",_ucLevel&0xF, g_tLogLVTable[_ucLevel].pHeader);
	iLen += iRet;
	
	va_list argp;
    va_start( argp, _pFormat );
    iRet = vsprintf(aucBuf + iLen, _pFormat, argp);
    va_end( argp );
	
	iLen += iRet;

#ifndef PENELOPE	
	/* 直接打印 */
	if (g_pLogFuncTable[_ucLevel])
	{
		g_pLogFuncTable[_ucLevel](aucBuf + 1, iLen -1); //首字符去掉
	}
#else
	client_log_printf(aucBuf, iLen);

#endif	
}

static local_log_printf(unsigned char _ucLevel, const char* _pFormat, ...) 
{
	unsigned char aucBuf[LOG_BUFFER_SIZE_MAX] = {0};
	int iRet = 0, iLen = 0;
	
	if (_ucLevel > LOG_TRACE) return;
	
	iRet = sprintf(aucBuf, "%X[%s] ",_ucLevel&0xF, g_tLogLVTable[_ucLevel].pHeader);
	iLen += iRet;
	
	va_list argp;
    va_start( argp, _pFormat );
    iRet = vsprintf(aucBuf + iLen, _pFormat, argp);
    va_end( argp );
	
	iLen += iRet;	
	if (g_pLogFuncTable[_ucLevel])
	{
		g_pLogFuncTable[_ucLevel](aucBuf + 1, iLen -1); //首字符去掉
	}	
}

static unsigned long get_file_size(const char *_pPath) 
{
	unsigned long dwFilesize = -1;
	struct stat statbuff;
	if (stat(_pPath, &statbuff) < 0)
	{
		return dwFilesize;
	}
	else
	{
		dwFilesize = statbuff.st_size;
	}
	return dwFilesize;
}

static int open_log_file(T_LogFileInfo *_pThis, const char *_pPath)
{
	long dwOffset = 0;
	FILE *pFile;
	
	strncpy(_pThis->aucPath, _pPath, strlen(_pPath) + 1);
	
	if (0 == access(_pPath, 0)) 
	{
		dwOffset = get_file_size(_pPath);
		if(dwOffset >= LOG_FILE_SAVE_SIZE_MAX) 
		{
			/* 当前log文件，满 */
			dwOffset = 0;
			char aucBackup[LOG_FILE_PATH_SIZE_MAX] = "";
			sprintf(aucBackup, "%s.0", _pPath);		//bug 已经存在的情况下会被覆盖
			if(-1 == rename(_pPath, aucBackup))
			{
				perror("-1 == rename");
			}
			pFile = fopen(_pPath, "w"); ///< cannot fseek
		} 
		else 
		{
			pFile = fopen(_pPath, "a"); ///< cannot fseek
		}
	}
	else 
	{
		pFile = fopen(_pPath, "w"); ///< can fseek
	}
	if(NULL == pFile) 
	{
		perror("log_file = NULL");
		return -1;
	}

	_pThis->m_iOffset = dwOffset;
	_pThis->pFile = pFile;

	return 0;
}


/* 打印 + 保存 */
static void server_log_printf(T_LogFileInfo *_pThis, int _ucLevel, unsigned char *_pData, int _iLen) 
{
	INT32 iRet = 0, iLen = 0;
	UINT8 ucLevel = _ucLevel - '0'; //参数是字符型->int
	
	/* 1.打印 */
	if (g_pLogFuncTable[ucLevel])
	{
		g_pLogFuncTable[ucLevel](_pData + 1, _iLen -1); //首字符去掉
	}

	/* 2.拼装时间戳 */
	UINT8 aucBuf[LOG_BUFFER_SIZE_MAX] = {0};
	sprintf(aucBuf, "[%s] %s\n", getLogTime(), _pData + 1); 
	iLen = strlen(aucBuf);
	/* 2.保存 */
	if(ucLevel <= LOG_INFO && NULL != _pThis->pFile) 
	{ /* log file */
		if(_pThis->m_iOffset + iLen > LOG_FILE_SAVE_SIZE_MAX) 
		{
			static long long dwIndex = 0;
			fclose(_pThis->pFile);
			char aucBackUpPath[128] = "";
			sprintf(aucBackUpPath, "%s.%lld", _pThis->aucPath, dwIndex++);

			if(-1 == rename(_pThis->aucPath, aucBackUpPath)) 
			{
				perror("-1 == rename");
			}
			_pThis->m_iOffset = 0;
			_pThis->pFile = fopen(_pThis->aucPath, "w"); ///< cannot fseek
			if(NULL == _pThis->pFile) 
			{
				perror("NULL == _pThis->pFile");
			}
		}
		if(_pThis->m_iOffset + iLen <= LOG_FILE_SAVE_SIZE_MAX) 
		{
			fprintf(_pThis->pFile, "%s", aucBuf);
			/* commend tmpfs */
			 fflush(_pThis->pFile);
			_pThis->m_iOffset += iLen;

		}
	}

}

static void *ConsoleLogServerListen(void* _pThis)
{
	PT_LogServerInfo pServer = _pThis;
	INT32 i,iRet = -1, iEvtCnt,iAddLen = sizeof(struct sockaddr_un);
	struct epoll_event	ev, events[EPEVENT_SIZE_MAX];  	
	
	struct sockaddr_un tAddCli;

	ev.data.fd = pServer->m_tUnSocket, ev.events = (EPOLLIN | EPOLLET/*|EPOLLET*/);
	if(epoll_ctl(pServer->m_iEpoll, EPOLL_CTL_ADD, pServer->m_tUnSocket, &ev) == -1)
	{
		perror("log server epoll_ctl error");
		goto LAB_EXIT;
	}	
	
	while(pServer->flsKeepRunning)
	{
		iEvtCnt = epoll_wait(pServer->m_iEpoll, events, EPEVENT_SIZE_MAX, CONSOLE_LOG_EPOLL_WAIT_TIMEOUT);  //1min
		if (iEvtCnt <= 0)  
		{  
			//local_log_printf(LOG_INFO, "log server epoll_wait..");
			continue;  
		}  		

		for (i = 0; i < iEvtCnt; i++)  
		{
			if (events[i].events & EPOLLIN) 
			{
				if (pServer->m_tUnSocket == events[i].data.fd) 
				{
					iRet = recvfrom(events[i].data.fd, pServer->m_aucBuf, LOG_BUFFER_SIZE_MAX - 1, 0, (struct sockaddr *)&tAddCli, &iAddLen); 
					//local_log_printf(LOG_TRACE, "read data size = %d", iRet);
					if(iRet >= 0) 
					{
						pServer->m_aucBuf[iRet] = 0;
						server_log_printf(pServer->pLogInfo, pServer->m_aucBuf[0], pServer->m_aucBuf, iRet);
					}
				}
#if 0				
				/* net socket */
				if (pServer->m_tNetSocket == events[i].data.fd) 
				{
					
				}
#endif
			}
			else 
			{
				local_log_printf(LOG_WARNING, "events %d", events[i].events);
			}
		} 		

	}

	local_log_printf(LOG_ERROR, "Please note that ConsoleLogServerListen has exit!!");
LAB_EXIT:
	pServer->flsEnable = FALSE;
	pServer->flsKeepRunning = FALSE;
	close(pServer->m_tUnSocket);
	close(pServer->m_iEpoll);
	unlink(CONSOLE_LOG_UNIX_DOMAIN);
	return;
}

INT32 console_log_init(INT32 _iMode)
{
	INT32 iRet = -1;
	PT_LogServerInfo pServer = &g_tLogServerInfo;
	
	if (pServer->flsEnable)
	{
		perror("log server has exist!!");
		return 0;
	}
	
	/* 1.初始化 log file */
	pServer->pLogInfo = (T_LogFileInfo *)malloc(sizeof(T_LogFileInfo));
	if (NULL == pServer->pLogInfo) 
	{
		perror("log file _malloc error");
		return -1;
	}
	if (-1 == open_log_file(pServer->pLogInfo, CONSOLE_LOG_FILE_PATH)) 
	{
		perror("_open_log_file error");
		return -1;
	}
	
	/* 2.1 初始化 本地服务器socket */
	struct sockaddr_un tAddServ;
	pServer->m_tUnSocket = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (pServer->m_tUnSocket < 0) 
	{  
		perror("UNIX server socket error");
		goto LAB_EXIT;
	}
	
	tAddServ.sun_family = AF_UNIX;
	strncpy(tAddServ.sun_path, CONSOLE_LOG_UNIX_DOMAIN, sizeof(tAddServ.sun_path)-1);
	unlink(CONSOLE_LOG_UNIX_DOMAIN);

	iRet = bind(pServer->m_tUnSocket, (struct sockaddr*)&tAddServ, sizeof(tAddServ));
	if(-1 == iRet) 
	{  
		perror("UNIX server socket bind error");
		goto LAB_EXIT;
	}

	/* 2.2 初始化 本地客户socket */
	pServer->m_tUnCliSocket = socket(PF_UNIX, SOCK_DGRAM, 0);
	if(pServer->m_tUnCliSocket < 0) 
	{  
		perror("UNIX client socket error");
		return -1;
	}     
	pServer->in_tCliAddr.sun_family = AF_UNIX;
	strncpy(pServer->in_tCliAddr.sun_path, CONSOLE_LOG_UNIX_DOMAIN, sizeof(pServer->in_tCliAddr.sun_path)-1);

	/* 3.初始化 epoll 事件 */
	if (-1 == (pServer->m_iEpoll = epoll_create(EPEVENT_SIZE_MAX)))
	{
		perror("epoll_create error");
		goto LAB_EXIT;
	}	

	/* 4.初始化线程 */
	pServer->flsEnable = TRUE;
	pServer->flsKeepRunning = TRUE;
	iRet = pthread_create(&pServer->pThrId, NULL, ConsoleLogServerListen, &g_tLogServerInfo);        
	if(0 != iRet)
	{	
		close(pServer->m_iEpoll);
		perror("_pthread_create error");
		goto LAB_EXIT;
	}
	pthread_detach(pServer->pThrId);	

	return 0;
	

LAB_EXIT:	
	close(pServer->m_tUnSocket);
	pServer->flsEnable		= FALSE;
	pServer->flsKeepRunning = FALSE;	
	unlink(CONSOLE_LOG_UNIX_DOMAIN);
	return iRet;
}

INT32 console_log_main(INT32 _iArgc, UINT8 **_pArgv) 
{
	int iRet;
	iRet = console_log_init(CONSOLE_MODE_SER_FORCE);
	
	local_log_printf(LOG_INFO, "console_log_init ret = %d", iRet);
	return 0;
}















