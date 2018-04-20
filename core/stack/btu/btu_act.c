/*************************************************************************
	> File Name: stack_act.c
	> Author: lroyd
	> Mail: 
	> Created Time: 2016年01月27日 星期三 10时19分42秒
 ************************************************************************/
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
    BTU_HDR *p_msg;
    GKI_disable();

    if (btu_cb.timer_queue.p_first == NULL)
    {
        /* 需要btu开启定时器，如果当前不是btu，则给btu发一个开启定时事件 */
        if (GKI_get_taskid() != BTU_TASK)
        {
            /* post event to start timer in BTU task */
            if ((p_msg = (BTU_HDR *)GKI_getbuf(BTU_HDR_SIZE)) != NULL)
            {
                p_msg->event = EVT_TO_START_TIMER;
                GKI_send_msg (BTU_TASK, TASK_MBOX_0, p_msg);
            }
        }
        else
        {
            /* Start free running 1 second timer for list management */
            GKI_start_timer (TIMER_0, GKI_MS_TO_TICKS (100), TRUE);
        }
    }
    /* 定时器列表不为空，需要移除重复的定时器，重新挂载 */
   
    GKI_remove_from_timer_list (&btu_cb.timer_queue, p_tle);

    p_tle->event = type;
    p_tle->ticks = timeout;         /* Save the number of seconds for the timer */

    GKI_add_to_timer_list (&btu_cb.timer_queue, p_tle);
    GKI_enable();
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
    BTU_HDR *p_msg;
    GKI_disable();
    GKI_remove_from_timer_list (&btu_cb.timer_queue, p_tle);

    /* if timer is stopped on other than BTU task */
    if (GKI_get_taskid() != BTU_TASK)
    {

        /* post event to stop timer in BTU task */
        if ((p_msg = (BTU_HDR *)GKI_getbuf(BTU_HDR_SIZE)) != NULL)
        {
            p_msg->event = EVT_TO_STOP_TIMER;
            GKI_send_msg (BTU_TASK, TASK_MBOX_0, p_msg);
        }
    }
    else
    {
        /* if timer list is empty stop periodic GKI timer */
        if (btu_cb.timer_queue.p_first == NULL)
        {
            GKI_stop_timer(TIMER_0);
        }
    }
    GKI_enable();
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
void btu_hcif_store_cmd(int controller_id, trans_format_t p_buf)
{
    tHCI_CMD_CB *p_hci_cmd_cb = &(btu_cb.hci_cmd_cb[controller_id]);
    uint16_t opcode;
    trans_format_t p_cmd;
    uint8_t *p;

    /* allocate buffer, Mount the retransmission queue*/ 
    if((p_cmd = GKI_getbuf( sizeof(trans_format)))==NULL)
    {
		/* error log */
        return;
    }
 
    /* 复制数据头 */
    memcpy(p_cmd , p_buf, sizeof(trans_format));
	/* 分配具体数据 */
	p_cmd->msg = malloc(p_buf->len);
	memcpy(p_cmd->msg, p_buf->msg, p_buf->len);
	
    /* queue copy of cmd */
    GKI_enqueue(&(p_hci_cmd_cb->cmd_cmpl_q), (void *)p_cmd);

    if(BTU_DATA_CMPL_TIMEOUT > 0)
    {
		p_hci_cmd_cb->cmd_cmpl_timer.hci_cb = p_hci_cmd_cb;
		btu_start_timer(&(p_hci_cmd_cb->cmd_cmpl_timer), BTU_TTYPE_DATA_TIMEOUT_EVT , BTU_DATA_CMPL_TIMEOUT);
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
void btu_hcif_send_cmd(int controller_id, trans_format_t p_buf)
{
    tHCI_CMD_CB *p_hci_cmd_cb = &(btu_cb.hci_cmd_cb[controller_id]);
	
	p_hci_cmd_cb->controller_id = controller_id;  //是否可以放在初始化的时候
	
	p_hci_cmd_cb->cmd_window = 1;

    while( p_hci_cmd_cb->cmd_window )
    {
        if(!p_buf)
        {
        	/* NULL 表示发送当前队列里的所有数据 */
			p_hci_cmd_cb->cmd_window = p_hci_cmd_cb->cmd_xmit_q.count--;
					
			p_buf = (trans_format_t)GKI_dequeue(&(p_hci_cmd_cb->cmd_xmit_q));
        }
        
        if(p_buf)
        {
            /* 复制一份副本 */
            btu_hcif_store_cmd(p_hci_cmd_cb->controller_id, p_buf);
            p_hci_cmd_cb->cmd_window--;
	
            stack_hci_send(p_hci_cmd_cb->controller_id, p_buf);
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
	btu_hcif_send_cmd ( p_hci_cmd_cb, p_hci_cmd_cb->conid, NULL );
	
	return;
}


/*******************************************************************************
**
** Function        btu_hcif_chk_kcp_evt
**
** Description    
** Returns          void
**
*******************************************************************************/
void btu_hcif_chk_kcp_evt(void)
{

}

/*******************************************************************************
**
** Function        btu_process_kcp 
**
** Description      ����Ӧ����������ش�����
**
** Returns          void
**
*******************************************************************************/
void btu_process_kcp(EventId event)
{

}

