#include "BaseType.h"
#include "log_file.h"
#include "usr_timer.h"


int tim_callback(int event, void *_pData)
{
	syslog_wrapper(LOG_DEBUG, "tim_callback fd = %d, var = %s", event, _pData);

	//USR_TimerDel(event);
	//return USR_TIMER_ONCE;
	return USR_TIMER_LOOP;
}


int main(int argc, char **argv) 
{
	int i,iRet;

	USR_TimerInit();
	

	for(i=0;i<10;i++)
	{
		iRet = USR_TimerNew(tim_callback, "123", "test", 1);
		syslog_wrapper(LOG_DEBUG, "USR_TimerNew iRet = %d", iRet);
	}
	
	USR_TimerPrint();
	while(1)
	{
		USR_TimerHandler();
		
		//sleep(1);
	}
	
	
    return 0;
}
