/******************************************************************
**
**
******************************************************************/
#include "btu_declare.h"
#include "task_declare.h"


tBTU_CB   btu_cb = {0}; 



/* btu层 超时回调 */
static void btu_task_conn_timer_cback(uint16_t evt, uint8_t conid, void *p_msg)
{
	if(conid == STACK_TASK)
	{
		/* 自己需要处理 */
		syslog_wrapper(LOG_ERROR, "STACK_TASK time out  %s", msg);
	}
	
	if(evt == PTLE_DATA_TIMEOUT)
	{
		/* 发送给对应的任务 */
		TSK_SEND_MSG(conid, evt, p_msg);
	}

}
/******************************************************************
*			任务初始化
******************************************************************/
void btu_task_init(void** data)
{ 
	btu_init_timer();

	btu_cb.hci_cmd_cb.cmd_cmpl_timer.p_cback = (TIMER_CBACK *)&btu_task_conn_timer_cback;
	
}

/******************************************************************
*			主任务处理
*
******************************************************************/
void btu_task(void** data, EventId event, void* msg)
{
	syslog_wrapper(LOG_ERROR, "btu_task  %s", msg);



}
/******************************************************************
*			任务析构
******************************************************************/
void btu_task_deinit(void** data)
{
	
}


