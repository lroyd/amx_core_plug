/*************************************************************************
	> File Name: stack_declare.h
	> Author: lroyd
	> Mail: 
	> Created Time: 2016年01月27日 星期三 11时41分11秒
 ************************************************************************/
#ifndef _STACK_DECLARE_H
#define _STACK_DECLARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "gki.h"
#include "log_file.h"


typedef struct
{
	int test;
	
	
} tBTU_CB;

extern  tBTU_CB   btu_cb; 
/**************************************************************************
 *                        函数声明
 **************************************************************************/
void btu_task_init(void** data);
void btu_task(void** data, EventId event, void* msg);
void btu_task_deinit(void** data);

#define STACK_TASK_INIT      btu_task_init
#define STACK_TASK_HANDLE    btu_task
#define STACK_TASK_DEINIT    btu_task_deinit



#ifdef __cplusplus
}
#endif
#endif
