#include "BaseType.h"
#include "task_declare.h"


#define DECLARE_TASK(id, init, handle, deinit) {id, init, handle, deinit, NULL}

/* 所有的任务 handle中的msg 都需要手动释放 */

taskEntry LocalTasks[] = {

/******************************************************************************************
 *      FROM HERE DECLARE YOUR TASKS!
 ******************************************************************************************/

    DECLARE_TASK(STACK_TASK, STACK_TASK_INIT, STACK_TASK_HANDLE, STACK_TASK_DEINIT),
/******************************************************************************************
 *      ABOVE HERE DECLARE YOUR TASKS!
 ******************************************************************************************/

};
