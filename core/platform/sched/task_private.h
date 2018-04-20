#ifndef __TASK_PRIVATE_H__
#define __TASK_PRIVATE_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"

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
#ifdef __cplusplus
}
#endif

#endif
