#ifndef __MESSAGE_H__
#define __MESSAGE_H__

/* 给用户使用的 */

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "gki.h"
#include "btu_declare.h"


/* pro用户层 通用事件 */
enum{
	PRO_EVT_DATA_TIMEOUT = 0,
	PRO_EVT_RECV_DATA,
	PRO_EVT_NEW_DATA,
};



/* 16进制 注意字头 */
typedef struct 
{
    UINT16          event;
    UINT16          len;					//要发送数据的总长度
	UINT32			seqno;
	
	UINT16			task_id;				//task id
	UINT16			ans_table;				//
	
    UINT16          offset;					
    UINT16          layer_specific;			//暂用超时重传
	//void			*msg;	//8个字节
}trans_format, *trans_format_t;

#define TRANS_HDR 	sizeof(trans_format)


#define TSK_SEND_MSG(m1, m2, m3)	TaskSendMessage(m1, m2, m3)







#ifdef __cplusplus
}
#endif

#endif
