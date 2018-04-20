#ifndef HCIDEFS_H
#define HCIDEFS_H


#include "btu_declare.h"
#include "btu_init.h"




extern uint8_t btsnd_hcic_data(tHCI_CMD_CB *p_ent, int conid, trans_format_t p_buf, uint16_t time_event, uint32_t time_out, uint8_t flags);

extern uint8_t btsnd_hcic_cmd( int conid, uint8_t p_cmd);



#endif

