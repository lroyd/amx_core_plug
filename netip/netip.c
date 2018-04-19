/*******************************************************************************
	> File Name: netip.c
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#include "netip.h"
#include "log_file.h"
#include "event.h"


typedef struct _tagUDPRemmapCli 
{
    INT32			m_tSocket;	//客户端真实fd
	PT_NetClient	pClient;
}T_UDPRemmapCli, *PT_UDPRemmapCli;


#define UDP_CLI_REMMAP_MAX (5)
static UINT32 g_uiUdpRemmapCnt = 1;
static T_UDPRemmapCli	g_tUdpRemmapTable[UDP_CLI_REMMAP_MAX] = {0}; //[0] not use


PT_EvtMangerInfo	g_pNetipServerManger = NULL;	//只创建一次
PT_EvtMangerInfo	g_pNetipClientManger = NULL;	//只创建一次


/* NETIP 守护定时器(默认关闭) */

#define NETIP_TIMER_MAIN   (60*1000)	// 默认1min-1000ms单位
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 NetipTimerFun(void *_pThis)
{
	INT32 iRet = -1;
	PT_EventInfo pThis	= _pThis;
	pThis->m_emType		= EVENT_TIMER;
	pThis->m_iEventFD	= NETIP_TIMER_MAIN;
	
	iRet = EventRegister(pThis, pThis->m_pManger);  
	
	syslog_wrapper(LOG_DEBUG, "Netip TimerFun %d", iRet);
   	return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static PT_EvtMangerInfo NetipServerMangerCreate(void)
{
	if(g_pNetipServerManger == NULL)
	{
		g_pNetipServerManger = CreatEventManger(MANGER_ROLE_MASTER);	//具有定时器功能
		EventMangerInit(g_pNetipServerManger, NULL, NETIP_TIMER_MAIN); //默认关闭
	}

	return g_pNetipServerManger;
}
/*******************************************************************************
* Name: TcpServerCreate
* Descriptions:创建服务器，不提供IP(本地IP)，只有PORT
* Parameter:	
* Return:	-1错误/socket fd
* *****************************************************************************/
INT32 TcpServerCreate(UINT16 _usLocalPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun)
{
	INT32 iRet = -1; 
	
	PT_EvtMangerInfo pServerManger = NetipServerMangerCreate(); //manger只有一个
    if(NULL == pServerManger)
    {
		syslog_wrapper(LOG_ERROR, "tcp _NetipServerMangerCreate error");
        return -1;
    }
	
	PT_NetServer pTCPServer = NetServerCreate(pServerManger);
	
	iRet = NetServerInit(NETIP_TCP, pTCPServer, _usLocalPort, _pUserRecvFun, _pUserErrFun, NULL);
	syslog_wrapper(LOG_TRACE, "tcp server create fd = %d", iRet);
	return iRet;
}

INT32 UdpServerCreate(UINT16 _usLocalPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun)
{
	INT32 iRet = -1; 
	
	PT_EvtMangerInfo pServerManger = NetipServerMangerCreate(); //manger只有一个
    if(NULL == pServerManger)
    {
		syslog_wrapper(LOG_ERROR, "udp _NetipServerMangerCreate error");
        return -1;
    }	
	
	PT_NetServer pUDPServer = NetServerCreate(pServerManger);
	
	iRet = NetServerInit(NETIP_UDP, pUDPServer, _usLocalPort, _pUserRecvFun, _pUserErrFun, NULL);

	syslog_wrapper(LOG_TRACE, "udp server create fd = %d", iRet);
	return 0;
}


















/* 所有的client实体都使用MANGER_ROLE_LISTEN监听功能 */
static PT_EvtMangerInfo NetipClientMangerCreate(void)
{
	if(g_pNetipClientManger == NULL)
	{
		g_pNetipClientManger = CreatEventManger(MANGER_ROLE_LISTEN);	
		EventMangerInit(g_pNetipClientManger, NULL, 0);
	}
		
	return g_pNetipClientManger;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 UdpClientCreate(UINT8 *_pConnIp, UINT16 _usConnPort, NET_PROCESSRECVDATA _pUserRecvFun)
{
	INT32 iSocket = -1, iRet = -1; 
	/* 1.初始化管理结构体 */
	PT_EvtMangerInfo pClientManger = NetipClientMangerCreate(); //manger只有一个
    if(NULL == pClientManger)
    {
		syslog_wrapper(LOG_ERROR, "udp _NetipClientMangerCreate error");
        return -1;
    }
	
	/* 2.创建SOCKET实体 */
	PT_NetClient pUdpClient = NetClientCreate(pClientManger, CLI_RCV_ASYNC); //client实体 可以有多个 CLI_RCV_ASYNC
	
	iRet = NetClientInit(NETIP_UDP, pUdpClient, _pConnIp, _usConnPort, _pUserRecvFun, NULL);
	if(iRet == 0)
	{
		/* 初始化成功 fd 与  PT_NetClient 映射 */
		g_tUdpRemmapTable[g_uiUdpRemmapCnt].pClient = pUdpClient;
		iSocket = g_uiUdpRemmapCnt++;			
	}

	return iSocket;
}

INT32 UdpClientDestroy(INT32 _iUserFd)
{
	PT_NetClient pClient = NULL;
	/* 解映射 */
	pClient = g_tUdpRemmapTable[_iUserFd].pClient;
	
	if(pClient)
	{
		NetClientDestroy(pClient);
		g_tUdpRemmapTable[_iUserFd].pClient = NULL;
	}
	else
	{
		/* 无效fd */
		syslog_wrapper(LOG_ERROR, "invalid udp fd = %d",_iUserFd);
	}	
	return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpClientCreate(UINT8 *_pConnIp, UINT16 _usConnPort, NET_PROCESSRECVDATA _pUserRecvFun)
{
	INT32 iSocket = -1, iRet = -1; 
	/* 1.初始化管理结构体 */
	PT_EvtMangerInfo pClientManger = NetipClientMangerCreate(); //manger只有一个
    if(NULL == pClientManger)
    {
		syslog_wrapper(LOG_ERROR, "tcp _NetipClientMangerCreate error");
        return -1;
    }
	
	/* 2.创建SOCKET实体 */
	PT_NetClient pTcpClient = NetClientCreate(pClientManger, CLI_RCV_ASYNC); //client实体 可以有多个 CLI_RCV_ASYNC
	
	iRet = NetClientInit(NETIP_TCP, pTcpClient, _pConnIp, _usConnPort, _pUserRecvFun, NULL);
	if(iRet == 0)
	{
		/* 初始化成功 fd 与  PT_NetClient 映射 */
		g_tUdpRemmapTable[g_uiUdpRemmapCnt].pClient = pTcpClient; //不应该共用UDPRemmap
		iSocket = g_uiUdpRemmapCnt++;			
	}
	
	return iSocket;
}

INT32 TcpClientDestroy(INT32 _iUserFd)
{
	PT_NetClient pClient = NULL;
	/* 解映射 */
	pClient = g_tUdpRemmapTable[_iUserFd].pClient;
	
	if(pClient)
	{
		NetClientDestroy(pClient);
		g_tUdpRemmapTable[_iUserFd].pClient = NULL;
	}
	else
	{
		/* 无效fd */
		syslog_wrapper(LOG_ERROR, "invalid tcp fd = %d",_iUserFd);
	}	
	return 0;
}



/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpClientSendData(INT32 _iUserFd, UINT8 *_aucSndBuf, INT32 _iLen)
{
	PT_NetClient pClient = NULL;	
	/* 解映射 */
	pClient = g_tUdpRemmapTable[_iUserFd].pClient;	
	if(pClient)
	{
		TcpClientSnd(pClient, _aucSndBuf, _iLen);
	}
	else
	{
		/* 无效fd */
		syslog_wrapper(LOG_ERROR, "invalid tcp fd = %d",_iUserFd);
	}

	return 0;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 UdpClientSendData(INT32 _iUserFd, UINT8 *_aucSndBuf, INT32 _iLen)
{
	PT_NetClient pClient = NULL;	
	/* 解映射 */
	pClient = g_tUdpRemmapTable[_iUserFd].pClient;	
	if(pClient)
	{
		UdpClientSndByIp(pClient, pClient->m_aucServIp, pClient->m_usServPort, _aucSndBuf, _iLen);
	}
	else
	{
		/* 无效fd */
		syslog_wrapper(LOG_ERROR, "invalid udp fd = %d",_iUserFd);
	}

	return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 UdpClientSendDataByIP(INT32 _iUserFd, UINT8 *_pConnIp, UINT16 _usConnPort, UINT8 *_aucSndBuf, INT32 _iLen)
{
	PT_NetClient pClient = NULL;	
	/* 解映射 */
	pClient = g_tUdpRemmapTable[_iUserFd].pClient;	
	if(pClient)
	{
		UdpClientSndByIp(pClient, _pConnIp, _usConnPort, _aucSndBuf, _iLen);
	}
	else
	{
		/* 无效fd */
		syslog_wrapper(LOG_ERROR, "invalid udp fd = %d",_iUserFd);
	}

	return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:查看本地创建了多少UDPclient
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 GetUdpClientNum(void)
{
	
	return 0;
}


/*******************************************************************************
* Name: 
* Descriptions:查看本地创建了多少TCPclient
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 GetTcpClientNum(void)
{
	
	return 0;
}



