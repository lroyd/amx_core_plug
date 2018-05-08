/*************************************************************************
	> File Name: stack_act.c
	> Author: lroyd
	> Mail: 
	> Created Time: 2016年01月27日 星期三 10时19分42秒
 ************************************************************************/
#include "os_declare.h"
 #include"btu_act.h"



void btu_init_timer(void)
{
	GKI_init_timer_list(&btu_cb.timer_queue);
}
/*******************************************************************************
**
** Function         btu_start_timer
**
** Description      Start a timer for the specified amount of time.
**                  NOTE: The timeout resolution is in SECONDS! (Even
**                          though the timer structure field is ticks)
** 				开启定时器/ms单位
**
** Returns          void
**
*******************************************************************************/
void btu_start_timer (TIMER_LIST_ENT *p_tle, uint16_t type, uint32_t timeout)
{

    //GKI_disable();

    if (btu_cb.timer_queue.p_first == NULL)
    {

        /* 需要btu开启定时器，如果当前不是btu，则给btu发一个开启定时事件 */
        if (GKI_get_taskid() != BTU_TASK)
        {
			TaskSendMessage(STACK_TASK, EVT_TO_START_TIMER, NULL);
        }
        else
        {
            GKI_start_timer (TIMER_0, GKI_MS_TO_TICKS (100), TRUE);
        }
    }
    /* 定时器列表不为空，需要移除重复的定时器，重新挂载 */

    GKI_remove_from_timer_list (&btu_cb.timer_queue, p_tle);

    p_tle->event = type;
    p_tle->ticks = timeout;         /* Save the number of seconds for the timer */

    GKI_add_to_timer_list (&btu_cb.timer_queue, p_tle);
    //GKI_enable();

}

/*******************************************************************************
**
** Function         btu_remaining_time
**
** Description      Return amount of time to expire
**
** Returns          time in second
**
*******************************************************************************/
uint32_t btu_remaining_time (TIMER_LIST_ENT *p_tle)
{
    return(GKI_get_remaining_ticks (&btu_cb.timer_queue, p_tle));
}

/*******************************************************************************
**
** Function         btu_stop_timer
**
** Description      Stop a timer.
**
** Returns          void
**
*******************************************************************************/
void btu_stop_timer (TIMER_LIST_ENT *p_tle)
{
    //GKI_disable();
    GKI_remove_from_timer_list (&btu_cb.timer_queue, p_tle);

    /* if timer is stopped on other than BTU task */
    if (GKI_get_taskid() != BTU_TASK)
    {
		TaskSendMessage(STACK_TASK, EVT_TO_STOP_TIMER, NULL);
    }
    else
    {
        /* if timer list is empty stop periodic GKI timer */
        if (btu_cb.timer_queue.p_first == NULL)
        {
            GKI_stop_timer(TIMER_0);
        }
    }
    //GKI_enable();
}

/*******************************************************************************
**
** Function         btu_hcif_store_cmd
**
** Description     
**
** Returns          void
**
*******************************************************************************/
void btu_hcif_store_cmd(tHCI_CMD_CB *p_cmd, trans_format_t p_buf)
{
    tHCI_CMD_CB *p_hci_cmd_cb = p_cmd;
    trans_format_t p_msg;

    /* allocate buffer, Mount the retransmission queue*/ 
    if((p_msg = GKI_getbuf(TRANS_HDR + p_buf->len))==NULL)
    {
		/* error log */
        return;
    }
    /* 复制数据头,验证数据？ */
	memcpy((uint8_t *)p_msg, p_buf, p_buf->len);
	
    /* queue copy of cmd */
    GKI_enqueue(&(p_hci_cmd_cb->cmd_cmpl_q), (void *)p_msg);

    if(p_hci_cmd_cb->ticks > 0)
    {
		p_hci_cmd_cb->cmd_cmpl_timer.hci_cb = p_hci_cmd_cb;
		btu_start_timer(&(p_hci_cmd_cb->cmd_cmpl_timer), p_hci_cmd_cb->event , p_hci_cmd_cb->ticks);
    }

}

/*******************************************************************************
**
** Function        btu_hcif_send_cmd 
**
** Description     1 :*hci_cmd_cb 每个设备的命令发送实体  controller_id
**                 2 : if p_buf = NULL  all send 
**
** Returns          void
**
*******************************************************************************/
void btu_hcif_send_cmd(tHCI_CMD_CB *p_cmd, trans_format_t p_buf)
{
    tHCI_CMD_CB *p_hci_cmd_cb = p_cmd;

	p_hci_cmd_cb->cmd_window = 1;

    while( p_hci_cmd_cb->cmd_window )
    {
        if(!p_buf)
        {
        	/* NULL 表示发送当前队列里的所有数据 */
			p_hci_cmd_cb->cmd_window = p_hci_cmd_cb->cmd_xmit_q.count;
			
			p_buf = (trans_format_t)GKI_dequeue(&(p_hci_cmd_cb->cmd_xmit_q));
        }
		
        if(p_buf)
        {
            /* 复制一份副本 */
            btu_hcif_store_cmd(p_hci_cmd_cb, p_buf);
            p_hci_cmd_cb->cmd_window--;
	
            stack_hci_send(p_hci_cmd_cb->controller_id, p_buf, p_buf->len);
            p_buf = NULL;
        }
        else
            break;
    }

    if(p_buf)
        GKI_enqueue(&(p_hci_cmd_cb->cmd_xmit_q), p_buf);

}

/*******************************************************************************
**
** Function        btu_process_timer 
**
** Description      
**
** Returns          void
**
*******************************************************************************/
void btu_process_timer(EventId event)
{

    if (event == EVT_TO_START_TIMER)
    {
        GKI_start_timer(TIMER_0, GKI_MS_TO_TICKS(100),TRUE);
    }
    else if (event == EVT_TO_STOP_TIMER)
    {
        if(btu_cb.timer_queue.p_first == NULL)
        {
            GKI_stop_timer(TIMER_0);
        }
    }
}


/*******************************************************************************
**
** Function        btu_hcif_retry_data_timeout
**
** Description    retry count++, if count > 3, need remove
** Returns          
**
*******************************************************************************/
void btu_hcif_retry_data_timeout(TIMER_LIST_ENT *p_tle, uint8_t count)
{
	tHCI_CMD_CB *p_hci_cmd_cb = (tHCI_CMD_CB *)(p_tle->hci_cb);
	
	trans_format_t p_cmd;

	
	while((p_cmd = (trans_format_t) GKI_dequeue (&(p_hci_cmd_cb->cmd_cmpl_q))))
	{
		
		/* have bug 0/1 */
		p_cmd->layer_specific++;
		if( p_cmd->layer_specific >= count )
		{
			/* remove timeout current cmd_data limit */
			//STACK_LOGE("%s conid: %d , index: %d, name: %s, count limit [%d]!!!", __func__, p_hci_cmd_cb->conid, p_cmd->req_index, p_cmd->name_id, p_cmd->layer_specific);
			/* timer callback */
			p_tle->p_cback(PTLE_DATA_TIMEOUT , p_hci_cmd_cb->controller_id,(void *)p_cmd);

			continue;
		}
		//STACK_LOGD("check layer_specific = [%d] ", p_cmd->layer_specific);
		/* add p_cmd to cmd_xmit_q list */
		GKI_enqueue(&(p_hci_cmd_cb->cmd_xmit_q), (void *)p_cmd);
	}
	
	/* xmit send all data to link */
	btu_hcif_send_cmd ( p_hci_cmd_cb, NULL );
	
	return;
}

/*******************************************************************************
**
** Function        btu_hcif_command_complete_evt
**
** Description    Remove the retransmission queue corresponding to p_msg (if yes)
**			只做摘除操作(kcp command need refresh retry count!!! not remove)
**
**			
** Returns ：
**				
*******************************************************************************/
int btu_hcif_command_complete_evt(tHCI_CMD_CB *hci_cmd_cb, trans_format_t p_msg)
{
	int iRet = -1;
	tHCI_CMD_CB *p_hci_cmd_cb = hci_cmd_cb;
	trans_format_t	 p_cmd;
	UINT16	event = p_msg->event;
	UINT32	seqno = p_msg->seqno;
	
	if (event == ACL_DATA_CMPL_EVT)
	{
		p_cmd = (trans_format_t)GKI_getfirst(&p_hci_cmd_cb->cmd_cmpl_q);
		while(p_cmd)
		{
			/* 1.判断index有效性 */
			
			/* 2.判断主次命令是否一致 */
			if ((p_cmd->event != GET_ANS_REQ(p_msg->event)))
			{
				/* 2.1 不是当前命令，一下个 */
				p_cmd = (trans_format_t)GKI_getnext(p_cmd);
				continue;
			}

			GKI_remove_from_queue(&p_hci_cmd_cb->cmd_cmpl_q, (void *)p_cmd);

			GKI_freebuf((void *)p_cmd);

			iRet = 0;
			break;			
		}
		
		if (BTU_DATA_CMPL_TIMEOUT > 0)
		{
			if (!GKI_queue_is_empty(&(p_hci_cmd_cb->cmd_cmpl_q)))
			{
				/*  */
				btu_start_timer(&(p_hci_cmd_cb->cmd_cmpl_timer), (uint16_t)(BTU_DATA_TTYPE_TIMEOUT_EVT), BTU_DATA_CMPL_TIMEOUT);
			}
			else
			{
				btu_stop_timer(&(p_hci_cmd_cb->cmd_cmpl_timer));
			}
		}
	}
	else
	{
		
		
	}
	
	return iRet;
}
