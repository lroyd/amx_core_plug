#ifndef __TASK_DECLARE_H__
#define __TASK_DECLARE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"

//#include "btu_declare.h"



#define INVALID_TASK_ID        (TASK_MAX)


typedef void (*taskInit)(void** data);
typedef void (*taskHandle)(void** data, EventId event, void* msg);
typedef void (*taskDeinit)(void** data);

typedef struct
{
    TaskId      id;
    taskInit    init;
    taskHandle  handle;
    taskDeinit  deinit;

    void*       data;
}taskEntry;


enum
{
    STACK_TASK = 0x00,	
    TASK_MAX,
};
extern taskEntry LocalTasks[TASK_MAX];

#ifdef __cplusplus
}
#endif
#endif
