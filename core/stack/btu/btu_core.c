/******************************************************************
**
**
******************************************************************/
#include "btu_declare.h"
#include "task_declare.h"


tBTU_CB   btu_cb = {0}; 



/* btu�� ��ʱ�ص� */
static void btu_task_conn_timer_cback(uint16_t evt, uint8_t conid, void *p_msg)
{
	if(conid == STACK_TASK)
	{
		/* �Լ���Ҫ���� */
		syslog_wrapper(LOG_ERROR, "STACK_TASK time out  %s", msg);
	}
	
	if(evt == PTLE_DATA_TIMEOUT)
	{
		/* ���͸���Ӧ������ */
		TSK_SEND_MSG(conid, evt, p_msg);
	}

}
/******************************************************************
*			�����ʼ��
******************************************************************/
void btu_task_init(void** data)
{ 
	btu_init_timer();

	btu_cb.hci_cmd_cb.cmd_cmpl_timer.p_cback = (TIMER_CBACK *)&btu_task_conn_timer_cback;
	
}

/******************************************************************
*			��������
*
******************************************************************/
void btu_task(void** data, EventId event, void* msg)
{
	syslog_wrapper(LOG_ERROR, "btu_task  %s", msg);



}
/******************************************************************
*			��������
******************************************************************/
void btu_task_deinit(void** data)
{
	
}


