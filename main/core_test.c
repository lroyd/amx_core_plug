#include "BaseType.h"
#include "gki.h"
#include "os_sched.h"



#define BTU_STACK_SIZE (20)
#define BTU_TASK_STR    ((int8_t *)"BTU_SCHED")

int SchedStack[(BTU_STACK_SIZE + 3)/4];





int main(int argc, char **argv) 
{
	int ret;
	/**************************************************************************************/
	GKI_init();
	 
	stack_init();
	 
    ret = GKI_create_task((TASKPTR)SchedLoop, \
                            BTU_TASK, BTU_TASK_STR, \
                            (uint16_t *)((uint8_t *)SchedStack + BTU_STACK_SIZE),\
                            sizeof(SchedStack));

    
    SchedInit();


    GKI_run(0);	 
 
	//sleep(2);
	/**************************************************************************************/
#if 1
	char *prim1 = GKI_getbuf(20);
	char *prim2 = GKI_getbuf(20);
	char *prim3 = GKI_getbuf(100);
	memcpy(prim1,"hello",6);
	memcpy(prim2,"fuck",5);
	

	GKI_PrintBufferUsage(NULL, NULL);
	GKI_PrintBuffer();
	
	GKI_print_task();
	
	GKI_send_msg(BTU_TASK, TASK_MBOX_0, prim1); 
	GKI_send_msg(BTU_TASK, TASK_MBOX_1, prim2); 
	sleep(1);
	GKI_PrintBufferUsage(NULL, NULL);
	prim1 = GKI_getbuf(20);
	GKI_PrintBufferUsage(NULL, NULL);
#endif
	while(1)
	{
		sleep(1);
	}
	
	
    return 0;
}
