/*******************************************************************************
	> File Name: 
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#include <stdarg.h>
#include <sys/epoll.h>

#include "event_timer.h"

#include "event.h"
#include "log_file.h"

/*******************************************************************************
*		define macro variable
********************************************************************************/

#define TAG_EPOLL_EVENT ("Event")

#define TIMER_MAIN_DEFAULT		(60*1000)		//1s     
#define EPOLL_EVENT_DEFAULT		(4*1024)



/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static void epoll_process(T_EvtMangerInfo *_pManger)
{
	INT32 i, iFds = -1;
	PT_EventInfo pThis = NULL;	
	
	INT32 iMinTime = _pManger->m_iType == MANGER_ROLE_MASTER ? amx_event_find_timer(): -1;
	iMinTime = iMinTime == 0 ? -1 :iMinTime;
	
	iFds = epoll_wait(_pManger->m_iEp, _pManger->m_pEvent, _pManger->m_iEventNum, iMinTime);
	if(iFds <= 0)
	{
		goto LAB_EXIT;
	}	

	for(i = 0; i < iFds; i++)
	{	
		pThis = NULL;
		if(_pManger->m_pEvent[i].events & EPOLLIN)
		{
			pThis = _pManger->m_pEvent[i].data.ptr;
			if(pThis == NULL)
				continue;
			if(pThis->m_Handle)
				pThis->m_Handle(pThis);
		}
		else
		{
			continue;
		}
	}

LAB_EXIT:
	if(MANGER_ROLE_MASTER == _pManger->m_iType)
	{
		amx_event_expire_timers();
	}
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static void *epoll_thread(void *_pArg)
{
	PT_EvtMangerInfo pManger = _pArg;

	//DLOGD(TAG_EPOLL_EVENT,"{%s:%d} start",__FUNCTION__,__LINE__);
	while( pManger->m_iEp != -1 )
	{
		epoll_process(pManger);
	}	    
	DLOGE(TAG_EPOLL_EVENT,"{%s:%d} exit",__FUNCTION__,__LINE__);
	return NULL;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 EventRegister(void *_pThis, void *_pManger)
{
	PT_EventInfo pThis			= _pThis;
	PT_EvtMangerInfo pManger	= _pManger;

	struct epoll_event ee;

	if ((EVENT_TIMER == pThis->m_emType) && (MANGER_ROLE_MASTER == pManger->m_iType))
	{
		/* in case : _iSetFd = time out */
		pThis->m_pManger	= pManger;
		amx_add_timer(_pThis, pThis->m_iEventFD);  
		pThis->m_iTimerSet	= 1;
		pThis->m_emType		= EVENT_TIMER;
		return 0;
	}
	else if (EVENT_INPUT == pThis->m_emType)
	{
		ee.events = EPOLLIN;
	}
	else if (EVENT_OUTPUT == pThis->m_emType)
	{
		ee.events = EPOLLOUT;
	}		
	else//注意其他事件 EVENT_OUTPUT/EVENT_OOB
	{
		ee.events = EPOLLIN|EPOLLET;
	}
	//pThis->m_iEventFD	= _iSetFd;

	ee.data.ptr			= (void *)pThis;
	pThis->m_pManger	= pManger;
	if (epoll_ctl(pManger->m_iEp, EPOLL_CTL_ADD, pThis->m_iEventFD, &ee) == -1) 
	{
		pThis->m_pManger = NULL;
		DLOGE(TAG_EPOLL_EVENT,"{%s:%d} _epoll_ctl error",__FUNCTION__,__LINE__);
		return -1;
	}
	return 0;
}

INT32 EventCancel(void *_pThis)
{
	INT32 iTpye;
	struct epoll_event ee;	
	PT_EventInfo pThis = _pThis;	
	T_EvtMangerInfo *pManger = pThis->m_pManger;

    if ((EVENT_TIMER == pThis->m_emType) && (1 == pThis->m_iTimerSet))
    {
        amx_event_del_timer(pThis);
    }
    else
    {
		iTpye = EPOLL_CTL_DEL;
		ee.events	= 0;
		ee.data.ptr	= NULL;
		if (epoll_ctl(pManger->m_iEp, iTpye, pThis->m_iEventFD, &ee) == -1) 
		{
			return -1;
		}
		pThis->m_pManger = NULL;
	}
	return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 MainTimerDefaultFun(void *_pThis)
{
	PT_EventInfo pThis = _pThis;
	pThis->m_emType		= EVENT_TIMER;
	pThis->m_iEventFD	= TIMER_MAIN_DEFAULT;
	
	EventRegister(pThis, pThis->m_pManger);  
	
	DLOGD(TAG_EPOLL_EVENT,"{%s:%d}",__FUNCTION__,__LINE__);
   	return 0;
}

INT32 EventMangerInit(PT_EvtMangerInfo _pThis, INT32 (*_pUserHandle)(void *), INT32 _iUserMSec)
{
	INT32 iRet = 0, iMainTime = 0;
	PT_EvtMangerInfo pThis = _pThis;
	
	if (pThis->m_iType == MANGER_ROLE_MASTER)	
	{
		amx_event_timer_init();
	}
	
	pThis->m_iEventNum	= EPOLL_EVENT_DEFAULT;
	pThis->m_pEvent		= malloc(sizeof(struct epoll_event) * pThis->m_iEventNum);
	if (NULL==pThis->m_pEvent)
	{    
		DLOGE(TAG_EPOLL_EVENT,"{%s:%d} malloc error",__FUNCTION__,__LINE__);
		return -1;
	}
	pThis->m_iEp = epoll_create(pThis->m_iEventNum);
	if (pThis->m_iEp == -1) 
	{
		free(pThis->m_pEvent);
		pThis->m_pEvent = NULL;        
		DLOGE(TAG_EPOLL_EVENT,"{%s:%d} epoll_create error",__FUNCTION__,__LINE__);
		return -1;
	}

	iRet = pthread_create(&pThis->m_pthreadID, NULL, epoll_thread, pThis); 
	if (iRet != 0)
	{
		close(pThis->m_iEp);
		free(pThis->m_pEvent);
		pThis->m_pEvent = NULL;
		DLOGE(TAG_EPOLL_EVENT,"{%s:%d} create epoll_thread error",__FUNCTION__,__LINE__);
		return iRet;
	}
	pthread_detach(pThis->m_pthreadID);
	if (pThis->m_iType == MANGER_ROLE_MASTER)
	{
		if (_pUserHandle)
		{
			pThis->in_tMainTimer.m_Handle = _pUserHandle;
			iMainTime = _iUserMSec;
		}
		else
		{
			pThis->in_tMainTimer.m_Handle = MainTimerDefaultFun;	
			iMainTime = TIMER_MAIN_DEFAULT;			
		}
		pThis->in_tMainTimer.m_emType		= EVENT_TIMER;
		pThis->in_tMainTimer.m_iEventFD		= iMainTime;
		EventRegister(&pThis->in_tMainTimer, pThis);
	}
	return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
PT_EvtMangerInfo CreatEventManger(MANGER_ROLE _emRole)
{
    PT_EvtMangerInfo pManger = malloc(sizeof(T_EvtMangerInfo));
    if(pManger)
    {
       memset(pManger,0,sizeof(T_EvtMangerInfo)); 
       pManger->m_iType =_emRole; 
    }
    return pManger;
}


