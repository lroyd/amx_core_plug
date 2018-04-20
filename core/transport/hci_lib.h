#ifndef _HCI_LIB_H
#define _HCI_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>



typedef void* TRANSAC;

typedef enum {
    HC_PRELOAD_SUCCESS  = 0x00,    //鍚姩鎴愬姛  0x00
    HC_PRELOAD_CB_ERROR,           //鍥炶皟鏈寕杞?
    HC_PRELOAD_UART_ERROR = 0x10,  //涓插彛澶辫触
    HC_PRELOAD_FULL_ERROR          //涓插彛澶辫触锛屽洖璋冨け璐?
}hc_preload_result_t;

typedef enum {
    HC_TX_SUCCESS,
    HC_TX_FAIL,
    HC_TX_FRAGMENT
}hc_transmit_result_t;




typedef void (*preload_result_cb)(TRANSAC transac, hc_preload_result_t result);

typedef char* (*alloc_mem_cb)(int size);

typedef int (*dealloc_mem_cb)(TRANSAC transac, char *p_buf);

typedef int (*tx_result_cb)(TRANSAC transac, char *p_buf, hc_transmit_result_t result);

typedef int (*data_ind_cb)(TRANSAC transac, uint32_t len, uint16_t dst, uint16_t event);

typedef struct 
{
    size_t         size;
	
    preload_result_cb  preload_cb;

    alloc_mem_cb   alloc;

    dealloc_mem_cb dealloc;

    data_ind_cb data_ind;

    tx_result_cb  tx_result;
} hc_callbacks_t;



typedef struct 
{
    size_t          size;

    int   (*init)(const hc_callbacks_t* p_cb);

    void (*set_power)(int state);

    void (*preload)(int conid, TRANSAC transac);

    int (*transmit_buf)(int conid, TRANSAC transac);

    int (*cmd_int)(int conid, uint8_t cmd);

    void  (*cleanup)( void );
} hc_interface_t;



extern const hc_interface_t* hc_get_interface(void);

#ifdef __cplusplus
}
#endif
#endif
