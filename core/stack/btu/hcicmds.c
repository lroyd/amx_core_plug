#include"hcicmds.h"
 

/*******************************************************************************
**
** Function        
**
** Description    
**
** Parameters:     
**					conid 
**					p_buf 
**					
*******************************************************************************/
uint8_t btsnd_hcic_data(int conid, trans_format_t p_buf, uint16_t time_event, uint32_t time_out, uint8_t flags)
{

	if(flags)
	{
		btu_hcif_send_cmd(conid, p_buf);		
	}
	else
		stack_hci_send(conid, p_buf);

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

