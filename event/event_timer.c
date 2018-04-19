/*******************************************************************************
	> File Name: event_timer.c
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 2014.6.24
 *******************************************************************************/
#include "event_timer.h"
#include "event.h"

VarLock  amx_event_timer_mutex;

amx_thread_volatile amx_rbtree_t  amx_event_timer_rbtree;
static amx_rbtree_node_t          amx_event_timer_sentinel;


amx_int_t amx_event_timer_init(void)
{
    amx_rbtree_init(&amx_event_timer_rbtree, &amx_event_timer_sentinel, amx_rbtree_insert_timer_value);
    InitVarLock(amx_event_timer_mutex);
    return 0;
}


amx_msec_t amx_event_find_timer(void)
{
    amx_msec_int_t      timer;
    amx_rbtree_node_t  *node, *root, *sentinel;

    if (amx_event_timer_rbtree.root == &amx_event_timer_sentinel) 
	{
        return AMX_TIMER_INFINITE;
    }

    LockVarLock(amx_event_timer_mutex);

    root = amx_event_timer_rbtree.root;
    sentinel = amx_event_timer_rbtree.sentinel;

    node = amx_rbtree_min(root, sentinel);

    UnLockVarLock(amx_event_timer_mutex);

    timer = (amx_msec_int_t) (node->key - amx_get_sys_timer());

    return (amx_msec_t) (timer > 0 ? timer : 0);
}


void amx_event_expire_timers(void)
{
    T_EventInfo *ev;
    amx_rbtree_node_t  *node, *root, *sentinel;

    sentinel = amx_event_timer_rbtree.sentinel;

    for ( ;; )
	{

        LockVarLock(amx_event_timer_mutex);

        root = amx_event_timer_rbtree.root;

        if (root == sentinel) 
		{
            return;
        }

        node = amx_rbtree_min(root, sentinel);

        if ((amx_msec_int_t) (node->key - amx_get_sys_timer()) <= 0) 
		{
            ev = (T_EventInfo *) node;
            amx_rbtree_delete(&amx_event_timer_rbtree, &ev->timer); 
			
            UnLockVarLock(amx_event_timer_mutex);
            
			ev->timer.left = NULL;
            ev->timer.right = NULL;
            ev->timer.parent = NULL;
            ev->m_iTimerSet = 0;
            if(ev->m_Handle)
            {
                ev->m_Handle(ev);
            }
            continue;
        }

        break;
    }

    UnLockVarLock(amx_event_timer_mutex);
}
