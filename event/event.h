/*******************************************************************************
	> File Name: 
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#ifndef _AMX_EVENT_H_
#define _AMX_EVENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "amx_rbtree.h"


typedef enum
{
	MANGER_ROLE_MASTER = 0x00,	//具有fd监听和定时器功能
	MANGER_ROLE_SLAVE,   
	MANGER_ROLE_LISTEN,			//只具有fd监听功能
	MANGER_ROLE_INLT,
}MANGER_ROLE;			//注意每一个管理者有自己独立的线程

typedef enum
{
	EVENT_TIMER = 0x00, 	//创建定时器，只有master可以使用
	EVENT_INPUT,      
	EVENT_INLT,
	EVENT_OUTPUT,		//not use
	EVENT_OOB,			//not use
}EVENT_TYPE;

typedef struct _tagEvent
{    
	amx_rbtree_node_t 	timer;    
	volatile INT16 		m_iTimerSet;
	EVENT_TYPE			m_emType;
	INT32				m_iEventFD;  
	INT32 (*m_Handle)(void *);
	void 				*m_pManger;
}T_EventInfo, *PT_EventInfo;


typedef struct _tagEventManger
{   
	INT32 				m_iEp;
	pthread_t			m_pthreadID;
	struct epoll_event	*m_pEvent;
	INT32				m_iEventNum;
	T_EventInfo			in_tMainTimer;
	UINT32				m_iType;        
}T_EvtMangerInfo, *PT_EvtMangerInfo;

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
PT_EvtMangerInfo CreatEventManger(MANGER_ROLE _emRole);
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 EventMangerInit(PT_EvtMangerInfo _pThis, INT32 (*_pUserHandle)(void *), INT32 _iUserMSec);
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 EventRegister(void *_pThis, void *_pManger);
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 EventCancel(void *_pThis);


#ifdef __cplusplus
}
#endif

#endif


