#ifndef __MESSAGE_H__
#define __MESSAGE_H__

/* ���û�ʹ�õ� */

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "gki.h"
#include "btu_declare.h"



/* �û��¼� */
#define SEND_DATA_TIMEOUT		(1)   //



typedef struct 
{
    UINT16          event;
    UINT16          len;
    UINT16          offset;
    UINT16          layer_specific;			//���ó�ʱ�ش�
	void			*msg;
}trans_format, *trans_format_t;




#define TSK_SEND_MSG(m1, m2, m3)	TaskSendMessage(m1, m2, m3)







#ifdef __cplusplus
}
#endif

#endif
