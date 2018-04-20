#include <os_sched.h>
#include <task_declare.h>

#include "gki.h"



extern taskEntry LocalTasks[TASK_MAX];

static uint8_t SchedLoopRunning = TRUE;

static void SchedTaskInit(void)
{
    uint8_t i = 0;

    for (i = 0; i < TASK_MAX; i++)
    {
        if (LocalTasks[i].init != NULL)
        {
            LocalTasks[i].init(&LocalTasks[i].data);
        }
    }
}

static void SchedTaskDeinit(void)
{
    uint8_t i = 0;

    for (; i < TASK_MAX; i++)
    {
        if (LocalTasks[i].deinit != NULL)
        {
            LocalTasks[i].deinit(&LocalTasks[i].data);
        }
    }
}

void SchedInit(void)
{
    SchedLoopRunning = TRUE;

    SchedTaskInit();

}

void SchedDeinit(void)
{
     SchedLoopRunning = FALSE;
	 
     SchedTaskDeinit();

}



void SchedLoop(uint32_t parameter)
{
    uint16_t event;
    uint8_t  handled;
    //GKI_set_taskid(0);


    while ( SchedLoopRunning )
    {
        event = GKI_wait(0xFFFF, 0);
        
        if (event & TASK_MBOX_0_EVT_MASK)
        {
            char *p_msg = NULL;
            
            while ((p_msg = GKI_read_mbox (TASK_MBOX_0)) != NULL)
            {
                syslog_wrapper(LOG_DEBUG, "M0  %s", p_msg);
				LocalTasks[0].handle(&LocalTasks[0].data, 4, p_msg);
				GKI_freebuf(p_msg);
            }			

        }       
		

        if (event & TASK_MBOX_1_EVT_MASK)
        {
            char *p_msg2 = NULL;
            
            while ((p_msg2 = GKI_read_mbox (TASK_MBOX_1)) != NULL)
            {
                syslog_wrapper(LOG_DEBUG, "M1  %s", p_msg2);
				GKI_freebuf(p_msg2);
            }	
        }     

        if (event & TASK_MBOX_2_EVT_MASK)
        {

	
        }


        if(event & TIMER_0_EVT_MASK)
        {

        }
        
        if(event & TIMER_1_EVT_MASK)
        {

        }
        
    }
    
    return;
}


void SchedSendMessage(uint16_t taskId, uint16_t opcode, void* msg)
{


}

void SchedSendBtaMessage(uint16_t taskId, uint16_t opcode, void *msg)
{

}



