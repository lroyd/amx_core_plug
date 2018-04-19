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

	syslog_wrapper(LOG_TRACE,"event thread start...");
	while( pManger->m_iEp != -1 )
	{
		epoll_process(pManger);
	}	    
	syslog_wrapper(LOG_ERROR,"event thread exit!!!");
	return NULL;
}
/*******************************************************************************
* Name: EventRegister
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
		//syslog_wrapper(LOG_DEBUG,"add timer %d ms", pThis->m_iEventFD);
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

	//syslog_wrapper(LOG_DEBUG,"add event fd = %d", pThis->m_iEventFD);
	ee.data.ptr			= (void *)pThis;
	pThis->m_pManger	= pManger;
	if (epoll_ctl(pManger->m_iEp, EPOLL_CTL_ADD, pThis->m_iEventFD, &ee) == -1) 
	{
		pThis->m_pManger = NULL;
		syslog_wrapper(LOG_FATAL,"_epoll_ctl error");
		return -1;
	}
	return 0;
}
/*******************************************************************************
* Name: EventCancel
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 EventCancel(void *_pThis)
{
	struct epoll_event ee;	
	PT_EventInfo pThis = _pThis;	
	T_EvtMangerInfo *pManger = pThis->m_pManger;

	//syslog_wrapper(LOG_DEBUG,"emType = %d, m_iTimerSet = %d", pThis->m_emType, pThis->m_iTimerSet);
    if (EVENT_TIMER == pThis->m_emType)
    {
		if(1 == pThis->m_iTimerSet)
		{
			syslog_wrapper(LOG_WARNING,"m_iTimerSet = 1 ? !!!!");
			amx_event_del_timer(pThis);
		}
    }
    else
    {
		ee.events	= 0;
		ee.data.ptr	= NULL;
		if (epoll_ctl(pManger->m_iEp, EPOLL_CTL_DEL, pThis->m_iEventFD, &ee) == -1) 
		{
			syslog_wrapper(LOG_FATAL,"_epoll_ctl delete error");
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
	INT32 iRet = -1;
	PT_EventInfo pThis = _pThis;
	pThis->m_emType		= EVENT_TIMER;
	pThis->m_iEventFD	= TIMER_MAIN_DEFAULT;
	
	iRet = EventRegister(pThis, pThis->m_pManger);  
	
	syslog_wrapper(LOG_DEBUG,"MainTimerDefaultFun %d", iRet);
   	return 0;
}
/*******************************************************************************
* Name: EventMangerInit
* Descriptions:
* Parameter:	
* Return: -1/0
* *****************************************************************************/
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
	if (NULL == pThis->m_pEvent)
	{    
		syslog_wrapper(LOG_FATAL,"event manager _malloc error");
		return -1;
	}
	pThis->m_iEp = epoll_create(pThis->m_iEventNum);
	if (pThis->m_iEp == -1) 
	{
		free(pThis->m_pEvent);
		pThis->m_pEvent = NULL;       
		syslog_wrapper(LOG_FATAL,"event manager _epoll_create error");
		return -1;
	}

	iRet = pthread_create(&pThis->m_pthreadID, NULL, epoll_thread, pThis); 
	if (iRet != 0)
	{
		close(pThis->m_iEp);
		free(pThis->m_pEvent);
		pThis->m_pEvent = NULL;
		syslog_wrapper(LOG_FATAL,"event manager create _thread error");
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
		if (!EventRegister(&pThis->in_tMainTimer, pThis))
		{
			syslog_wrapper(LOG_INFO,"event manager init ok");
		}
	}
	
	return 0;
}

/*******************************************************************************
* Name: CreatEventManger
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
	else
	{
		syslog_wrapper(LOG_FATAL,"event manager create _malloc error!!!");
	}
    return pManger;
}


