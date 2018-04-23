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



/* btu层 超时回调(每一个定时器 都有对应的cb) */
static void btu_task_timer_cback(uint16_t evt, uint16_t conid, void *p_msg)
{
	//syslog_wrapper(LOG_ERROR, "STACK_TASK time out");
	printf("asdfasdf data = %s\r\n",(uint8_t *)p_msg+TRANS_HDR);
	if(conid == STACK_TASK)
	{
		/* 自己需要处理 */
		
	}
	
	if(evt == PTLE_DATA_TIMEOUT)
	{
		/* 发送给对应的任务 */
		//TSK_SEND_MSG(conid, evt, p_msg);
	}

}
/******************************************************************
*			任务初始化
******************************************************************/
void btu_task_init(void** data)
{ 
	btu_init_timer();

	btu_cb.hci_cmd_cb[0].cmd_cmpl_timer.p_cback = (TIMER_CBACK *)&btu_task_timer_cback; //BTU task
	
	/*测试使用*/
	btu_cb.sand_mode = BTU_SEND_MODE_ANS;
}

/******************************************************************
*			主任务处理
*			注意 释放
******************************************************************/
void btu_task(void** data, EventId event, void* p_data)
{
	syslog_wrapper(LOG_ERROR, "btu_task event = 0x%04x", event);
	int ret  =0;
	UINT16	cmpl_event;
	switch(event)
	{
		case EVT_TO_TIMER_MSK:		//使用定时器的时候发送
		{
			/* user timer event 100ms */
			btu_process_timer(event); 
			break;
		}
		case BTU_RECV_DATA_EVT:	//data_ind 
		{
			/* 区分 应答数据 + new 数据 */
			trans_format_t p_msg = p_data;
			UINT16	task_id = p_msg->task_id;
			if(p_msg->event & BTU_ANS_EVT_RSP_MSK)
			{
				/* 应答数据，判断完 需要发送到对应的task */
				ret = btu_hcif_command_complete_evt(&(btu_cb.hci_cmd_cb[task_id]), p_data);
				if (ret)
				{
					/* 重传摘除，失败 */
					break;
				}
				cmpl_event = PRO_EVT_RECV_DATA;
			}
			else
			{
				/* 新数据,发送到对应的任务 */
				cmpl_event = PRO_EVT_NEW_DATA;
			}
			TSK_SEND_MSG(task_id, cmpl_event, p_msg);  //p_msg 不能释放
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
*			任务析构
******************************************************************/
void btu_task_deinit(void** data)
{
	
}


