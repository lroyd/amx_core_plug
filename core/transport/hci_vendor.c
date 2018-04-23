


#include <sys/prctl.h>
#include <signal.h>
#include <assert.h>
#include <pthread.h>

#include "hci_vendor.h"
#include "hci_lib.h"
#include "message.h"

#include "log_file.h"


/******************************************************************************
**  
******************************************************************************/
hc_callbacks_t *hc_cbacks = NULL;



/*****************************************************************************
**
**   HOST/CONTROLLER INTERFACE LIBRARY FUNCTIONS
**
*****************************************************************************/
static int init(const hc_callbacks_t* p_cb)
{

	syslog_wrapper(LOG_INFO, "....int");


    return 0;
}


static void set_power(int state)
{
	syslog_wrapper(LOG_INFO, "....set_power");

}

static void preload(int conid, TRANSAC transac)
{
	syslog_wrapper(LOG_INFO, "....preload");

}

/* 所有的底层要发送的数据包，都是由底层释放 */
static int transmit_buf(int conid, TRANSAC transac, uint16_t len)
{
	
	syslog_wrapper(LOG_INFO, "....transmit_buf conid = %d, len = %d, data = %s",conid, len, transac + TRANS_HDR);
	
	if (transac)
	{
		GKI_freebuf(transac);  
	}
	
    return 0;
}

static int cmd_int(int conid, uint8_t cmd)
{

	syslog_wrapper(LOG_INFO, "....cmd_int");
    return 0;
}


static void cleanup( void )
{

	syslog_wrapper(LOG_INFO, "....cleanup");

}


static const hc_interface_t HCLibInterface = {
    sizeof(hc_interface_t),
    init,
    set_power,
    preload,
    transmit_buf,
    cmd_int,
    cleanup
};


/*******************************************************************************
**
** Function        bt_hc_get_interface
**
** Description     Caller calls this function to get API instance
**
** Returns         API table
**
*******************************************************************************/
const hc_interface_t *hc_get_interface(void)
{
    return &HCLibInterface;
}
