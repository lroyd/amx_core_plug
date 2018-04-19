#include "BaseType.h"

#include "log_file.h"
#include "netip.h"



#define TIMER_MAIN   (2*1000)	// 1000ms单位

INT32 MainTimerFun(void *_pThis)
{
	PT_EventInfo pThis = _pThis;
	pThis->m_emType		= EVENT_TIMER;
	pThis->m_iEventFD	= TIMER_MAIN;
	
	EventRegister(pThis, pThis->m_pManger);  
	
	syslog_wrapper(LOG_TRACE,"MainTimerFun...");
   	return 0;
}

/*******************************************************************************
*		用户定时器测试
********************************************************************************/
typedef struct _tagUserInfo
{    
	T_EventInfo	in_tEvent;
	INT32		m_iCnt;
}T_UserInfo;

T_UserInfo g_pUserEvt;

INT32 SubTimerFun(void *_pThis)
{
	T_UserInfo *pThis = _pThis;
	INT32 iCnt = pThis->m_iCnt++;
	if(pThis->m_iCnt == 5)
	{
		EventCancel(_pThis);
	}
	else
	{
		pThis->in_tEvent.m_emType	= EVENT_TIMER;
		pThis->in_tEvent.m_iEventFD	= 1000;	//1s
		
		EventRegister(pThis, pThis->in_tEvent.m_pManger); 		
	}
 
	syslog_wrapper(LOG_TRACE,"cnt = %d", iCnt);
   	return 0;
}


/*******************************************************************************
*		
********************************************************************************/
INT32 main(INT32 argc, char **argv) 
{
	PT_EvtMangerInfo pMainEventManger = NULL;
    pMainEventManger = CreatEventManger(MANGER_ROLE_MASTER);
	
    if(NULL == pMainEventManger)
    {
        syslog_wrapper(LOG_ERROR,"_CreatEventManger error");
        return 0;
    }
	
	EventMangerInit(pMainEventManger, MainTimerFun, TIMER_MAIN); //MainTimerFun, TIMER_MAIN
/********************************************************************************/	
	/* 测试：注册用户定时器 */

	g_pUserEvt.in_tEvent.m_emType		= EVENT_TIMER;
	g_pUserEvt.in_tEvent.m_iEventFD		= 1000;	//1s
	g_pUserEvt.in_tEvent.m_Handle		= SubTimerFun;
	g_pUserEvt.m_iCnt = 0;
	EventRegister(&g_pUserEvt, pMainEventManger);

/********************************************************************************/	


	while(1)
	{
		sleep(1);
	}
 
    return 0;
}














