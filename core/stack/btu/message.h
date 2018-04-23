#ifndef __MESSAGE_H__
#define __MESSAGE_H__

/* ���û�ʹ�õ� */

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "gki.h"
#include "btu_declare.h"


/* pro�û��� ͨ���¼� */
enum{
	PRO_EVT_DATA_TIMEOUT = 0,
	PRO_EVT_RECV_DATA,
	PRO_EVT_NEW_DATA,
};



/* 16���� ע����ͷ */
typedef struct 
{
    UINT16          event;
    UINT16          len;					//Ҫ�������ݵ��ܳ���
	UINT32			seqno;
	
	UINT16			task_id;				//task id
	UINT16			ans_table;				//
	
    UINT16          offset;					
    UINT16          layer_specific;			//���ó�ʱ�ش�
	//void			*msg;	//8���ֽ�
}trans_format, *trans_format_t;

#define TRANS_HDR 	sizeof(trans_format)


#define TSK_SEND_MSG(m1, m2, m3)	TaskSendMessage(m1, m2, m3)







#ifdef __cplusplus
}
#endif

#endif
