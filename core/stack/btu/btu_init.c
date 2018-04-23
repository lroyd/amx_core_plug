#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <time.h>


#include "gki.h"
#include "btu_declare.h"


#include "hci_vendor.h"
#include "hci_lib.h"
#include "log_file.h"

static hc_interface_t *hc_if = NULL;

static const hc_callbacks_t hc_callbacks;



static void stack_load_conf(void)
{

		

}

static void stack_in_hw_init(void)
{
    if ( (hc_if = (hc_interface_t *) hc_get_interface()) == NULL)
    {

    }

}

static void stack_hci_enable(void)
{
    if (hc_if)
    {
        int result = hc_if->init(&hc_callbacks);


	
        hc_if->preload(0, NULL);
    }
}

static void stack_hci_disable(void)
{

    if (hc_if)
    {
        hc_if->cleanup();
		

    }
}
/******************************************************************************
**
** Function         stack_init
**
** Description      Internal helper function for chip hardware init
**
** Returns          None
**
******************************************************************************/
void stack_init(void)
{
	stack_load_conf();

    stack_in_hw_init();
    

    stack_hci_enable();

}

void stack_deinit(void)
{
    stack_hci_disable();
}

void stack_hci_send (int conid, uint8_t *p_msg, uint16_t len)
{

	if (hc_if)
	{
        hc_if->transmit_buf(conid, p_msg, len);  
	}
    else
	{

	}		
}


void stack_hci_send_cmd(int conid, uint8_t p_cmd)
{
	if (hc_if)
	{

        hc_if->cmd_int(conid, p_cmd);   //TODO!! 
	}
    else
	{

	}	
}



/*****************************************************************************
**
**   libbt-hci Callback Functions
**
*****************************************************************************/
static void preload_cb(TRANSAC transac, hc_preload_result_t result)
{

    
}

static char *alloc(int size)
{
    void *p_hdr = NULL;



    return ((char *) p_hdr);
}

static int dealloc(TRANSAC transac, char *p_buf)
{



    return 0;
}

static int data_ind(TRANSAC transac, uint32_t len, uint16_t dst, uint16_t event)
{


  
	
    return 0;
}


static int tx_result(TRANSAC transac, char *p_buf, hc_transmit_result_t result)
{



    return 0;
}

/*****************************************************************************
**   The libbt-hci Callback Functions Table
*****************************************************************************/
static const hc_callbacks_t hc_callbacks = 
{
    sizeof(hc_callbacks_t),
    preload_cb,
    alloc,
    dealloc,
    data_ind,
    tx_result,
};


