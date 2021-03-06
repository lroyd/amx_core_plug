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
#include "message.h"
#include "log_file.h"
#include "os_declare.h"

typedef struct
{
    uint16_t    event;
	uint16_t	len;				           
	uint16_t    layer_specific;
	void*		msg;
}BTU_HDR;

#define BTU_HDR_SIZE     (sizeof(BTU_HDR))




#define PTLE_DATA_TIMEOUT (PRO_EVT_DATA_TIMEOUT)




/* 超时事件 */
#define BTU_DATA_TTYPE_TIMEOUT_EVT		(0) 

/* 超时时间 */
#define BTU_DATA_CMPL_TIMEOUT           (10)    //10*100 = 1s

/* 超时重传次数 */
#define BTU_DATA_RETRY_TIMEOUT_MAX  	(5)

/*btu task time event  0x3000 - 0x3001 */
#define EVT_TO_TIMER_MSK            (0x3000)
#define EVT_TO_START_TIMER          (0x3000)
#define EVT_TO_STOP_TIMER           (0x3001)


/* 正常接收有应答的数据时 */
#define BTU_RECV_DATA_EVT           (0x4000)  //data_ind 发送过来的


typedef struct
{      
	uint16_t		request;
	uint16_t		respone;
	uint16_t		timeout;
	uint16_t		ticks;
}tBTU_ANS_EVT;

#define BTU_ANS_EVT_REQ_MSK		(0x00FF)
#define GET_ANS_REQ(x)			(x|BTU_ANS_EVT_REQ_MSK)

#define BTU_ANS_EVT_RSP_MSK		(0xFF00)
#define MAKE_ANS_RSP(x)			(x|BTU_ANS_EVT_RSP_MSK)

#define BTU_ANS_EVT_XMIT_REQ	(0x0000)	//透传数据
#define BTU_ANS_EVT_DATA_REQ	(0x0001)


enum{
	ACL_DATA_NONE_EVT = 0,  //透传无应答
	ACL_DATA_CMPL_EVT,
	ANS_MAX,
};


extern  tBTU_ANS_EVT   btu_ans_table[ANS_MAX]; 

 /* profile层向往HCI层发（需要定时重传） 必须定义 */
typedef struct
{      
	uint8_t				controller_id;
	BUFFER_Q 			cmd_xmit_q;
	BUFFER_Q 			cmd_cmpl_q;
	TIMER_LIST_ENT 		cmd_cmpl_timer;   //user kcp
	uint16_t		    cmd_window;
	/* 定时器 */
	uint16_t 			event;				
	uint32_t 			ticks;			
}tHCI_CMD_CB;



#define BTU_SEND_MODE_XMIT	(0)	//透传模式，不带头
#define BTU_SEND_MODE_ANS	(1)	//应答模式，带头

typedef struct
{
	uint8_t			sand_mode;
    TIMER_LIST_Q	quick_timer_queue;
    TIMER_LIST_Q	timer_queue;
    tHCI_CMD_CB		hci_cmd_cb[INVALID_TASK_ID];	
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
