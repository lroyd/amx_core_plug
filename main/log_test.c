//#include "console_log.h"

#include "log_file.h"

int main(int argc, char **argv) 
{
	unsigned char i = 0;

	console_log_init(CONSOLE_MODE_SER_FORCE);
	

	while(1)
	{

		//syslog_wrapper(LOG_ERROR, "count:%d", i++);
		syslog_wrapper(LOG_WARNING, "count:%d", i++);
		syslog_wrapper(LOG_DEBUG, "count:%d", i++);
		syslog_wrapper(LOG_INFO, "count:%d", i++);
		syslog_wrapper(LOG_TRACE, "count:%d", i++);
		sleep(1);

	}
 
    return 0;
}
