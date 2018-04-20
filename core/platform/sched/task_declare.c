#include "BaseType.h"
#include "task_private.h"
#include "task_declare.h"


#define DECLARE_TASK(id, init, handle, deinit) {id, init, handle, deinit, NULL}

taskEntry LocalTasks[] = {

/******************************************************************************************
 *      FROM HERE DECLARE YOUR TASKS!
 ******************************************************************************************/

    DECLARE_TASK(STACK_TASK, STACK_TASK_INIT, STACK_TASK_HANDLE, STACK_TASK_DEINIT),
/******************************************************************************************
 *      ABOVE HERE DECLARE YOUR TASKS!
 ******************************************************************************************/

};
