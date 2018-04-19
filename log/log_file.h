#ifndef __CONSOLE_LOG__
#define __CONSOLE_LOG__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <error.h>
#include <errno.h>


enum 
{
	LOG_FATAL = 0,
	LOG_ERROR,
	LOG_WARNING,	
	LOG_DEBUG,
	LOG_INFO,
	LOG_TRACE,
	LOG_MAX,
};


enum 
{
	CONSOLE_MODE_CLI_DEFAULT = 0,	//默认模式：C端 打印
	CONSOLE_MODE_CLI_LOG_FILE,		//C端 打印 + 保存
	CONSOLE_MODE_SER_FORCE,			//本地增强模式
	CONSOLE_MODE_NET_PLUG,			//开启网络组件
};

#define DLOGE
#define DLOGI
#define DLOGW
#define DLOGD


#define syslog_wrapper(level, format, arg...)\
	if(level < LOG_ERROR)\
	{\
		console_log_printf(level, "[%s:%d](%d %s):"format"", __FUNCTION__, __LINE__, errno, strerror(errno), ##arg);\
    }\
	else\
	{\
		console_log_printf(level, "[%s:%d]:"format"", __FUNCTION__, __LINE__, ##arg);\
	}

	
	
	
#ifdef __cplusplus
}
#endif

#endif
