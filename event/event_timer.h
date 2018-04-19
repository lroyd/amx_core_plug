/*******************************************************************************
	> File Name: event_timer.h
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 2014.6.24
 *******************************************************************************/
#ifndef _AMX_EVENT_TIMER_H_
#define _AMX_EVENT_TIMER_H_

#include "BaseType.h"
#include "amx_rbtree.h"
#include "event.h"

typedef amx_rbtree_key_t      amx_msec_t;
typedef amx_rbtree_key_int_t  amx_msec_int_t;

#define AMX_TIMER_INFINITE  (amx_msec_t) -1

#define AMX_TIMER_LAZY_DELAY  300

#define amx_add_timer        amx_event_add_timer
#define amx_del_timer        amx_event_del_timer

amx_int_t amx_event_timer_init(void);
amx_msec_t amx_event_find_timer(void);
void amx_event_expire_timers(void);

extern VarLock  amx_event_timer_mutex;
extern amx_thread_volatile amx_rbtree_t amx_event_timer_rbtree;

static amx_inline amx_msec_t amx_get_sys_timer(void)
{	
  	struct 	timeval now_tv; 
    gettimeofday(&now_tv, NULL);
  	return (now_tv.tv_sec*1000 + now_tv.tv_usec/1000);
}

static amx_inline void amx_event_del_timer(T_EventInfo *ev)
{

    LockVarLock(amx_event_timer_mutex);

    amx_rbtree_delete(&amx_event_timer_rbtree, &ev->timer);

    UnLockVarLock(amx_event_timer_mutex);

    ev->timer.left = NULL;
    ev->timer.right = NULL;
    ev->timer.parent = NULL;
    ev->m_iTimerSet = 0;
}

static amx_inline void amx_event_add_timer(T_EventInfo *ev, amx_msec_t timer)
{
    amx_msec_t      key;
    amx_msec_int_t  diff;

    key = amx_get_sys_timer() + timer;

    if (ev->m_iTimerSet) 
    {
        diff = (amx_msec_int_t) (key - ev->timer.key);
        if (amx_abs(diff) < AMX_TIMER_LAZY_DELAY) 
        {
           return;
        }
        amx_del_timer(ev);
    }
	
    ev->timer.key = key;
	
    LockVarLock(amx_event_timer_mutex);
	
    amx_rbtree_insert(&amx_event_timer_rbtree, &ev->timer);
	
    UnLockVarLock(amx_event_timer_mutex);
	
    ev->m_iTimerSet = 1;
}


#endif 
