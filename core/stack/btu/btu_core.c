/******************************************************************
**
**
******************************************************************/
#include "btu_declare.h"
#include "task_declare.h"



tBTU_ANS_EVT	btu_ans_table[ANS_MAX] = 
{
	{0, 0, 0, 0},
	{BTU_ANS_EVT_DATA_REQ, MAKE_ANS_RSP(BTU_ANS_EVT_DATA_REQ), BTU_DATA_TTYPE_TIMEOUT_EVT, BTU_DATA_CMPL_TIMEOUT},
};


tBTU_CB   btu_cb = {0}; 



/* btu�� ��ʱ�ص�(ÿһ����ʱ�� ���ж�Ӧ��cb) */
static void btu_task_timer_cback(uint16_t evt, uint16_t conid, void *p_msg)
{
	//syslog_wrapper(LOG_ERROR, "STACK_TASK time out");
	printf("asdfasdf data = %s\r\n",(uint8_t *)p_msg+TRANS_HDR);
	if(conid == STACK_TASK)
	{
		/* �Լ���Ҫ���� */
		
	}
	
	if(evt == PTLE_DATA_TIMEOUT)
	{
		/* ���͸���Ӧ������ */
		//TSK_SEND_MSG(conid, evt, p_msg);
	}

}
/******************************************************************
*			�����ʼ��
******************************************************************/
void btu_task_init(void** data)
{ 
	btu_init_timer();

	btu_cb.hci_cmd_cb[0].cmd_cmpl_timer.p_cback = (TIMER_CBACK *)&btu_task_timer_cback; //BTU task
	
	/*����ʹ��*/
	btu_cb.sand_mode = BTU_SEND_MODE_ANS;
}

/******************************************************************
*			��������
*			ע�� �ͷ�
******************************************************************/
void btu_task(void** data, EventId event, void* p_data)
{
	syslog_wrapper(LOG_ERROR, "btu_task event = 0x%04x", event);
	int ret  =0;
	UINT16	cmpl_event;
	switch(event)
	{
		case EVT_TO_TIMER_MSK:		//ʹ�ö�ʱ����ʱ����
		{
			/* user timer event 100ms */
			btu_process_timer(event); 
			break;
		}
		case BTU_RECV_DATA_EVT:	//data_ind 
		{
			/* ���� Ӧ������ + new ���� */
			trans_format_t p_msg = p_data;
			UINT16	task_id = p_msg->task_id;
			if(p_msg->event & BTU_ANS_EVT_RSP_MSK)
			{
				/* Ӧ�����ݣ��ж��� ��Ҫ���͵���Ӧ��task */
				ret = btu_hcif_command_complete_evt(&(btu_cb.hci_cmd_cb[task_id]), p_data);
				if (ret)
				{
					/* �ش�ժ����ʧ�� */
					break;
				}
				cmpl_event = PRO_EVT_RECV_DATA;
			}
			else
			{
				/* ������,���͵���Ӧ������ */
				cmpl_event = PRO_EVT_NEW_DATA;
			}
			TSK_SEND_MSG(task_id, cmpl_event, p_msg);  //p_msg �����ͷ�
			return;
		}
		default:
			syslog_wrapper(LOG_ERROR, "btu_task unknown event = 0x%04x, data = %s",event, p_data);
			break;
	}

    if(p_data)
    {
        GKI_freebuf(p_data);
    }	
	
}
/******************************************************************
*			��������
******************************************************************/
void btu_task_deinit(void** data)
{
	
}


