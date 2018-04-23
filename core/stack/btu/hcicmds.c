#include"hcicmds.h"
 

 
/* 不带重发，透传模式（无头） + 无应答模式（带头） */
uint8_t btsnd_hcic_data_xmit(int conid, void *p_buf, uint16_t len)
{
	/* 根据配置文件标志 */
	uint8_t send_mode = btu_cb.sand_mode;
	
	if (BTU_SEND_MODE_ANS == send_mode)
	{
		/* 分配数据头 */
		trans_format_t p_msg = GKI_getbuf(TRANS_HDR + len);
		p_msg->event = 0;
		p_msg->len = TRANS_HDR + len;	
		p_msg->seqno = 0;

		memcpy((uint8_t *)p_msg + TRANS_HDR, p_buf, len);	//注意 按字节拷贝，而不是按结构字节
		stack_hci_send(conid, (uint8_t *)p_msg, TRANS_HDR + len);
	}
	else
	{
		uint8_t *p_msg = GKI_getbuf(len);
		memcpy(p_msg, p_buf, len);
		stack_hci_send(conid, p_buf, len);
	}
	
    return (TRUE);
}

/* 应答模式 */
uint8_t btsnd_hcic_data_ans(int conid, void *p_buf, uint16_t len)
{
	tHCI_CMD_CB *p_hci_cmd_cb = &(btu_cb.hci_cmd_cb[conid]);
	p_hci_cmd_cb->controller_id = conid; 

	trans_format_t p_msg = GKI_getbuf(TRANS_HDR + len); //注意释放
	p_msg->event = btu_ans_table[ACL_DATA_CMPL_EVT].request;
	p_msg->len = TRANS_HDR + len;	//***********************************************
	p_msg->seqno = 0;  //帧序号添加，cmpl判断
	
	p_msg->task_id = conid;
	p_msg->ans_table = ACL_DATA_CMPL_EVT;
	
	memcpy((uint8_t *)p_msg + TRANS_HDR, p_buf, len);
	
	p_hci_cmd_cb->event	= btu_ans_table[ACL_DATA_CMPL_EVT].timeout;
	p_hci_cmd_cb->ticks	= btu_ans_table[ACL_DATA_CMPL_EVT].ticks;

	btu_hcif_send_cmd(p_hci_cmd_cb, p_msg);

    return (TRUE);
}
/*******************************************************************************
**
** Function        btsnd_hcic_cmd
**
** Description    
**
** Parameters:      
**					
*******************************************************************************/
uint8_t btsnd_hcic_cmd( int conid, uint8_t p_cmd)
{
	stack_hci_send_cmd(conid, p_cmd);
    return (TRUE);
}

