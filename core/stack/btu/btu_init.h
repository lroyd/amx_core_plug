#ifndef __STACK_INIT_H__
#define __STACK_INIT_H__

#ifdef __cplusplus
extern "C" {
#endif




void stack_init(void);
void stack_deinit(void);

void stack_hci_send (int conid, uint8_t *p_msg, uint16_t len);
void stack_hci_send_cmd(int conid, uint8_t p_cmd);


#ifdef __cplusplus
}
#endif


#endif
