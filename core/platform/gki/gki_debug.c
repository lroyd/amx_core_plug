#include "gki_init.h"
#include "log_file.h"

#if (GKI_DEBUG == TRUE)

const INT8 * const OSTaskStates[] =
{
    (INT8 *)"DEAD",  /* 0 */
    (INT8 *)"REDY",  /* 1 */
    (INT8 *)"WAIT",  /* 2 */
    (INT8 *)"",
    (INT8 *)"DELY",  /* 4 */
    (INT8 *)"",
    (INT8 *)"",
    (INT8 *)"",
    (INT8 *)"SUSP",  /* 8 */
};


/*******************************************************************************
**
** Function         GKI_PrintBufferUsage
**
** Description      Displays Current Buffer Pool summary
**
** Returns          void
**
*******************************************************************************/
void GKI_PrintBufferUsage(UINT8 *p_num_pools, UINT16 *p_cur_used)
{
    int i;
    FREE_QUEUE_T    *p;
    UINT8   num = gki_cb.com.curr_total_no_of_pools;
    UINT16   cur[GKI_NUM_TOTAL_BUF_POOLS];

    syslog_wrapper(LOG_DEBUG, "");
    syslog_wrapper(LOG_DEBUG, "--- GKI Buffer Pool Summary (R - restricted, P - public) ---");

    syslog_wrapper(LOG_DEBUG, "POOL     SIZE  USED  MAXU  TOTAL");
    syslog_wrapper(LOG_DEBUG, "------------------------------");
    for (i = 0; i < gki_cb.com.curr_total_no_of_pools; i++)
    {
        p = &gki_cb.com.freeq[i];
        if ((1 << i) & gki_cb.com.pool_access_mask)
        {
            syslog_wrapper(LOG_DEBUG, "%02d: (R), %4d, %3d, %3d, %3d",
                        i, p->size, p->cur_cnt, p->max_cnt, p->total);
        }
        else
        {
            syslog_wrapper(LOG_DEBUG, "%02d: (P), %4d, %3d, %3d, %3d",
                        i, p->size, p->cur_cnt, p->max_cnt, p->total);
        }
        cur[i] = p->cur_cnt;
    }
    if (p_num_pools)
        *p_num_pools = num;
    if (p_cur_used)
        memcpy(p_cur_used, cur, num*2);
}

/*******************************************************************************
**
** Function         GKI_PrintBuffer
**
** Description      Called internally by OSS to print the buffer pools
**
** Returns          void
**
*******************************************************************************/
void GKI_PrintBuffer(void)
{
    UINT16 i;
    for(i=0; i<GKI_NUM_TOTAL_BUF_POOLS; i++)
	{
		syslog_wrapper(LOG_DEBUG, "pool:%4u free %4u cur %3u max %3u  total%3u", i, gki_cb.com.freeq[i].size,
                    gki_cb.com.freeq[i].cur_cnt, gki_cb.com.freeq[i].max_cnt, gki_cb.com.freeq[i].total);
    }
}

/*******************************************************************************
**
** Function         gki_calc_stack
**
** Description      This function tries to calculate the amount of
**                  stack used by looking non magic num. Magic num is consider
**                  the first byte in the stack.
**
** Returns          the number of unused byte on the stack. 4 in case of stack overrun
**
*******************************************************************************/
static UINT16 gki_calc_stack (UINT8 task)
{
    int    j, stacksize;
    UINT32 MagicNum;
    UINT32 *p;

    stacksize = (int) gki_cb.com.OSStackSize[task];
    p = (UINT32 *)gki_cb.com.OSStack[task]; /* assume stack is aligned, */
    MagicNum = *p;

    for(j = 0; j < stacksize; j++)
    {
        if(*p++ != MagicNum) break;
    }

    return (j * sizeof(UINT32));
}

/*******************************************************************************
**
** Function         GKI_print_task
**
** Description      Print task stack usage.
**
** Returns          void
**
*******************************************************************************/
void GKI_print_task(void)
{
    UINT8 TaskId;

	syslog_wrapper(LOG_DEBUG, "TID TASKNAME STATE FREE_STACK  STACK");
	for(TaskId=0; TaskId < GKI_MAX_TASKS; TaskId++)
	{
		if (gki_cb.com.OSRdyTbl[TaskId] != TASK_DEAD)
		{
			syslog_wrapper(LOG_DEBUG, "%2u   %-8s %-5s  0x%04X     0x%04X Bytes",
				(UINT16)TaskId,  gki_cb.com.OSTName[TaskId],
				OSTaskStates[gki_cb.com.OSRdyTbl[TaskId]],
                gki_calc_stack(TaskId), gki_cb.com.OSStackSize[TaskId]);
        }
    }
}


/*******************************************************************************
**
** Function         gki_print_buffer_statistics
**
** Description      Called internally by OSS to print the buffer pools statistics
**
** Returns          void
**
*******************************************************************************/
void gki_print_buffer_statistics(FP_PRINT print, INT16 pool)
{
    UINT16           i;
    BUFFER_HDR_T    *hdr;
    UINT16           size,act_size,maxbuffs;
    UINT32           *magic;

    if (pool > GKI_NUM_TOTAL_BUF_POOLS || pool < 0)
    {
        print("Not a valid Buffer pool\n");
        return;
    }

    size = gki_cb.com.freeq[pool].size;
    maxbuffs = gki_cb.com.freeq[pool].total;
    act_size = size + BUFFER_PADDING_SIZE;
    syslog_wrapper(LOG_DEBUG, "Buffer Pool[%u] size=%u cur_cnt=%u max_cnt=%u  total=%u\n",
        pool, gki_cb.com.freeq[pool].size,
        gki_cb.com.freeq[pool].cur_cnt, gki_cb.com.freeq[pool].max_cnt, gki_cb.com.freeq[pool].total);

    syslog_wrapper(LOG_DEBUG, "      Owner  State    Sanity\n");
    syslog_wrapper(LOG_DEBUG, "----------------------------\n");
    hdr = (BUFFER_HDR_T *)(gki_cb.com.pool_start[pool]);
    for(i=0; i<maxbuffs; i++)
    {
        magic = (UINT32 *)((UINT8 *)hdr + BUFFER_HDR_SIZE + size);
        syslog_wrapper(LOG_DEBUG, "%3d: 0x%02x %4d %10s\n", i, hdr->task_id, hdr->status, (*magic == MAGIC_NO)?"OK":"CORRUPTED");
        hdr          = (BUFFER_HDR_T *)((UINT8 *)hdr + act_size);
    }
    return;
}


/*******************************************************************************
**
** Function         gki_print_used_bufs
**
** Description      GKI_map_taskname 需要添加
**
*******************************************************************************/
GKI_API void gki_print_used_bufs (FP_PRINT print, UINT8 pool_id)
{
    UINT8        *p_start;
    UINT16       buf_size;
    UINT16       num_bufs;
    BUFFER_HDR_T *p_hdr;
    UINT16       i;
    UINT32         *magic;
    UINT16       *p;


    if ((pool_id >= GKI_NUM_TOTAL_BUF_POOLS) || (gki_cb.com.pool_start[pool_id] != 0))
    {
        print("Not a valid Buffer pool\n");
        return;
    }

    p_start = gki_cb.com.pool_start[pool_id];
    buf_size = gki_cb.com.freeq[pool_id].size + BUFFER_PADDING_SIZE;
    num_bufs = gki_cb.com.freeq[pool_id].total;

    for (i = 0; i < num_bufs; i++, p_start += buf_size)
    {
        p_hdr = (BUFFER_HDR_T *)p_start;
        magic = (UINT32 *)((UINT8 *)p_hdr + buf_size - sizeof(UINT32));
        p     = (UINT16 *) p_hdr;

        if (p_hdr->status != BUF_STATUS_FREE)
        {
            print ("%d:0x%x (Q:%d,Task:%s,Stat:%d,%s) %04x %04x %04x %04x %04x %04x %04x %04x\n",
                i, p_hdr,
                p_hdr->q_id,
                1,
                p_hdr->status,
                (*magic == MAGIC_NO)? "OK" : "CORRUPTED",
                p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
        }
    }
}


/*******************************************************************************
**
** Function         gki_print_task
**
** Description      This function prints the task states.
**
** Returns          void
**
*******************************************************************************/
void gki_print_task (FP_PRINT print)
{

}


/*******************************************************************************
**
** Function         gki_print_exception
**
** Description      This function prints the exception information.
**
** Returns          void
**
*******************************************************************************/
void gki_print_exception(FP_PRINT print)
{

}


/*****************************************************************************/
void gki_dump (UINT8 *s, UINT16 len, FP_PRINT print)
{
    UINT16 i, j;

    for(i=0, j=0; i<len; i++)
    {
        if(j == 0)
            print("\n%lX: %02X, ", &s[i], s[i]);
        else if(j == 7)
            print("%02X,  ", s[i]);
        else
            print("%02X, ", s[i]);
        if(++j == 16)
            j=0;
    }
    print("\n");
}

void gki_dump2 (UINT16 *s, UINT16 len, FP_PRINT print)
{
    UINT16 i, j;

    for(i=0, j=0; i<len; i++)
    {
        if(j == 0)
            print("\n%lX: %04X, ", &s[i], s[i]);
        else
            print("%04X, ", s[i]);
        if(++j == 8)
            j=0;
    }
    print("\n");
}

void gki_dump4 (UINT32 *s, UINT16 len, FP_PRINT print)
{
    UINT16 i, j;

    for(i=0, j=0; i<len; i++)
    {
        if(j == 0)
            print("\n%lX: %08lX, ", &s[i], s[i]);
        else
            print("%08lX, ", s[i]);
        if(++j == 4)
            j=0;
    }
    print("\n");
}


#endif
