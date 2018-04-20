#ifndef __TASK_DECLARE_H__
#define __TASK_DECLARE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include <task_private.h>

#include "btu_declare.h"



#define INVALID_TASK_ID        (TASK_MAX)

enum
{
    STACK_TASK = 0x00,	
    TASK_MAX
};
extern taskEntry LocalTasks[TASK_MAX];
#ifdef __cplusplus
}
#endif
#endif
