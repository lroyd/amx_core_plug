#include "BaseType.h"
#include "event.h"
#include "netip_server.h"

#include "log_file.h"
#include "netip.h"


#define TAG_MAIN   ("User")


#define TIMER_MAIN   (60*1000)	// 1000ms单位

INT32 MainTimerFun(void *_pThis)
{
	PT_EventInfo pThis = _pThis;
	pThis->m_emType		= EVENT_TIMER;
	pThis->m_iEventFD	= TIMER_MAIN;
	
	EventRegister(pThis, pThis->m_pManger);  
	
	DLOGD(TAG_MAIN,"MainTimerFun...");
   	return 0;
}

/*******************************************************************************
*		用户定时器测试
********************************************************************************/
//#define USER_TEST_EVENT_TIMER

#ifdef USER_TEST_EVENT_TIMER
#define TAG_SUB	   ("Sub")
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
	if(pThis->m_iCnt == 10)
	{
		EventCancel(_pThis);
	}
	else
	{
		pThis->in_tEvent.m_emType	= EVENT_TIMER;
		pThis->in_tEvent.m_iEventFD	= 1000;	//1s
		
		EventRegister(pThis, pThis->in_tEvent.m_pManger); 		
	}
 
	DLOGD(TAG_SUB,"cnt = %d", iCnt);
   	return 0;
}
#endif
/*******************************************************************************
*		网络服务器接口测试
********************************************************************************/
//#define USER_TEST_NETIP_SERVER		//netip 服务器 测试开关

#ifdef USER_TEST_NETIP_SERVER

#define USER_RCV_DATA_SIZE_MAX   (127)
static UINT8 g_TcpServerIP[MAX_IP_STR_LEN + 1] = {0};

INT32 TcpConnectMsgProcess(PT_PeerNodeInfo _pThis, UINT8 *_pData, INT32 _iLen)
{
	PT_PeerNodeInfo pClient = _pThis;
	INT32 iLen = _iLen;
	UINT8 aucRcvData[USER_RCV_DATA_SIZE_MAX + 1] = {0}; //字符型的都要有 + 1
	if (iLen)
	{
		/* 有数据，具体的协议用户自己解析 */
		if(iLen < USER_RCV_DATA_SIZE_MAX)
		{
			memcpy(aucRcvData, _pData, iLen);
			aucRcvData[iLen] = 0;
			DLOGD(TAG_MAIN,"TCP socket = %d, ip = [%s] [%d] len = %d, data = %s", pClient->m_tSocket, pClient->m_ucClientIp, pClient->m_usClientPort, iLen, aucRcvData);
			TcpServerSnd(pClient, "fuck you", 8);
			//TcpPeerDisconnect(pClient);
		}
		else
		{
			DLOGE(TAG_MAIN,"TCP user rcv data full error");
		}
	}
	
	return 0;
}

INT32 TcpConnectErrorProcess(PT_PeerNodeInfo _pThis, NETIP_RET_CODE _emErrCode)
{
	PT_PeerNodeInfo pClient = _pThis;
	DLOGD(TAG_MAIN,"m_tSocket = %d , code = 0x%04x", pClient->m_tSocket, _emErrCode);
	
    return 0;
}

static UINT8 g_UdpServerIP[MAX_IP_STR_LEN + 1] = {0};
INT32 UdpConnectMsgProcess(PT_PeerNodeInfo _pThis, UINT8 *_pData, INT32 _iLen)
{
	PT_PeerNodeInfo pClient = _pThis;
	INT32 iLen = _iLen;
	UINT8 aucRcvData[USER_RCV_DATA_SIZE_MAX + 1] = {0}; //字符型的都要有 + 1
	if (iLen)
	{
		if(iLen < USER_RCV_DATA_SIZE_MAX)
		{
			memcpy(aucRcvData, _pData, iLen);
			aucRcvData[iLen] = 0;
			DLOGD(TAG_MAIN,"UDP socket = %d, ip = [%s] [%d] len = %d, data = %s", pClient->m_tSocket, pClient->m_ucClientIp, pClient->m_usClientPort, iLen, aucRcvData);
			UdpServerSnd(pClient, "gun...", 6);
		}
		else
		{
			DLOGE(TAG_MAIN,"UDP user rcv data full error");
		}
	}	
	
}



#endif

/*******************************************************************************
*		网络客户端接口测试
********************************************************************************/
//#define USER_TEST_NETIP_CLIENT

#ifdef USER_TEST_NETIP_CLIENT

#define USER_RCV_DATA_SIZE_MAX   (127)

INT32 UdpConnectRecvProcess(PT_PeerNodeInfo _pThis, UINT8 *_pData, INT32 _iLen)
{
	PT_PeerNodeInfo pClient = _pThis;
	INT32 iLen = _iLen;
	UINT8 aucRcvData[USER_RCV_DATA_SIZE_MAX + 1] = {0}; //字符型的都要有 + 1
	if (iLen)
	{
		if(iLen < USER_RCV_DATA_SIZE_MAX)
		{
			memcpy(aucRcvData, _pData, iLen);
			aucRcvData[iLen] = 0;
			DLOGD(TAG_MAIN,"UDP rrrrrrr = %d, ip = [%s] [%d] len = %d, data = %s", pClient->m_tSocket, pClient->m_ucClientIp, pClient->m_usClientPort, iLen, aucRcvData);
			
			/* 客户端关闭 */
			//TcpClientDestroy(1);
			//UdpServerSnd(pClient, "gun...", 6);
		}
		else
		{
			DLOGE(TAG_MAIN,"UDP user rcv data full error");
		}
	}	
}





#endif
/*******************************************************************************
*		
********************************************************************************/
INT32 main(INT32 argc, char **argv) 
{
	PT_EvtMangerInfo pMainEventManger = NULL;
    pMainEventManger = CreatEventManger(MANGER_ROLE_MASTER);
	
    if(NULL == pMainEventManger)
    {
        DLOGE(TAG_MAIN,"_CreatEventManger error");
        return 0;
    }
	
	EventMangerInit(pMainEventManger, NULL, 0); //MainTimerFun, TIMER_MAIN
/********************************************************************************/	
#ifdef USER_TEST_EVENT_TIMER	
	/* 测试：注册用户定时器 */
	printf("user test[event timer]...\r\n");
	g_pUserEvt.in_tEvent.m_emType		= EVENT_TIMER;
	g_pUserEvt.in_tEvent.m_iEventFD		= 1000;	//1s
	g_pUserEvt.in_tEvent.m_Handle		= SubTimerFun;
	g_pUserEvt.m_iCnt = 0;
	EventRegister(&g_pUserEvt, pMainEventManger);
#endif
/********************************************************************************/	
#ifdef USER_TEST_NETIP_SERVER
	/* 测试：TCP服务器 */
	printf("user test[tcp server]...\r\n");
	
	PT_NetEntity pTCPServer = NULL;
	pTCPServer = NetServerCreate(pMainEventManger);
	if(pTCPServer)
	{
		NetServerInit(NETIP_TCP, pTCPServer, 60009, TcpConnectMsgProcess, TcpConnectErrorProcess, g_TcpServerIP);
	}
	else
	{
		DLOGE(TAG_MAIN,"_TcpServerCreate error");
	}
	
	/* 测试：UDP服务器 */
	printf("user test[udp server]...\r\n");
	PT_NetEntity pUDPServer = NULL;
	pUDPServer = NetServerCreate(pMainEventManger); 
	if(pUDPServer)
	{
		NetServerInit(NETIP_UDP, pUDPServer, 60008, UdpConnectMsgProcess, NULL, g_UdpServerIP);
	}
	else
	{
		DLOGE(TAG_MAIN,"_UdpServerCreate error");
	}	
	
#endif	
/********************************************************************************/		
#ifdef USER_TEST_NETIP_CLIENT

	printf("user test[udp client]...\r\n");
	INT32 ifd1 = 0, ifd2 = 0;
#if 0	
	ifd1 = UdpClientCreate(UdpConnectRecvProcess);

	ifd2 = UdpClientCreate(UdpConnectRecvProcess);
	printf("333333 %d %d\r\n",ifd1, ifd2);
	UdpClientSendData(ifd1, "192.168.1.164", 60008, "fuck you",8);
	UdpClientDestroy(ifd1);
	UdpClientSendData(ifd1, "192.168.1.164", 60008, "fuck you",8);
	
	ifd1 = UdpClientCreate(UdpConnectRecvProcess);
	//UdpClientSendData(ifd1, "192.168.1.164", 60008, "fuck you",8);
	printf("444444444 %d %d\r\n",ifd1, ifd2);
	sleep(2);
	UdpClientSendData(ifd2, "192.168.1.164", 60009, "caocao",6);
#endif
	ifd1 = TcpClientCreate("192.168.1.164", 60001, UdpConnectRecvProcess);

	printf("tcp fd1 = %d, %d\r\n",ifd1, ifd2);
	TcpClientSendData(ifd1, "woyaofuchou", 11);
	
	
	sleep(5);
	TcpClientDestroy(ifd1);
	
	//ifd1 = TcpClientCreate("192.168.1.164", 60001, UdpConnectRecvProcess);
#endif	
/********************************************************************************/	


	while(1)
	{
		sleep(1);
	}
 
    return 0;
}














