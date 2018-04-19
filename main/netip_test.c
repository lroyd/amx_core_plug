#include "BaseType.h"

#include "log_file.h"
#include "netip.h"



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
			syslog_wrapper(LOG_TRACE,"TCP socket = %d, ip = [%s] [%d] len = %d, data = %s", pClient->m_tSocket, pClient->m_ucClientIp, pClient->m_usClientPort, iLen, aucRcvData);
			TcpServerSnd(pClient, "fuck you", 8);
			//TcpPeerDisconnect(pClient);
		}
		else
		{
			syslog_wrapper(LOG_ERROR,"TCP user rcv data full error");
		}
	}
	
	return 0;
}

INT32 TcpConnectErrorProcess(PT_PeerNodeInfo _pThis, NETIP_RET_CODE _emErrCode)
{
	PT_PeerNodeInfo pClient = _pThis;
	syslog_wrapper(LOG_TRACE,"m_tSocket = %d , code = 0x%04x", pClient->m_tSocket, _emErrCode);
	
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
			syslog_wrapper(LOG_TRACE,"UDP socket = %d, ip = [%s] [%d] len = %d, data = %s", pClient->m_tSocket, pClient->m_ucClientIp, pClient->m_usClientPort, iLen, aucRcvData);
			UdpServerSnd(pClient, "gun...", 6);
		}
		else
		{
			syslog_wrapper(LOG_ERROR,"UDP user rcv data full error");
		}
	}	
	
}



#endif

/*******************************************************************************
*		网络客户端接口测试
********************************************************************************/
#define USER_TEST_NETIP_CLIENT

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
			syslog_wrapper(LOG_TRACE,"UDP rrrrrrr = %d, ip = [%s] [%d] len = %d, data = %s", pClient->m_tSocket, pClient->m_ucClientIp, pClient->m_usClientPort, iLen, aucRcvData);
			
			/* 客户端关闭 */
			//TcpClientDestroy(1);
			//UdpServerSnd(pClient, "gun...", 6);
		}
		else
		{
			syslog_wrapper(LOG_ERROR,"UDP user rcv data full error");
		}
	}	
}





#endif
/*******************************************************************************
*		
********************************************************************************/
INT32 main(INT32 argc, char **argv) 
{

/********************************************************************************/	
#ifdef USER_TEST_NETIP_SERVER
	/* 测试：TCP服务器 */
	printf("user test[tcp server]...\r\n");

	
#endif	
/********************************************************************************/		
#ifdef USER_TEST_NETIP_CLIENT

	printf("user test[udp client]...\r\n");
	INT32 ifd1 = 0, ifd2 = 0;
#if 1	
	ifd1 = UdpClientCreate("192.168.1.164", 60008, UdpConnectRecvProcess);

	ifd2 = UdpClientCreate("192.168.1.164", 60009, UdpConnectRecvProcess);
	
	sleep(2);
	
	UdpClientSendData(ifd1,"fuck you",8);
	UdpClientDestroy(ifd1);
	
	UdpClientSendData(ifd2,"fuck you",8);
	UdpClientDestroy(ifd1);
	
	//ifd1 = UdpClientCreate(UdpConnectRecvProcess);
	//UdpClientSendData(ifd1, "192.168.1.164", 60008, "fuck you",8);

	//sleep(2);
	//UdpClientSendData(ifd2, "192.168.1.164", 60009, "caocao",6);
#else
	ifd1 = TcpClientCreate("192.168.1.164", 60001, UdpConnectRecvProcess);
	ifd2 = TcpClientCreate("192.168.1.164", 60001, UdpConnectRecvProcess);
	
	printf("tcp fd1 = %d, %d\r\n",ifd1, ifd2);
	
	TcpClientSendData(ifd1, "woyaofuchou", 11);
	TcpClientSendData(ifd2, "fuck you", 8);
	
	sleep(5);
	TcpClientDestroy(ifd1);
	TcpClientDestroy(ifd2);
	//ifd1 = TcpClientCreate("192.168.1.164", 60001, UdpConnectRecvProcess);
#endif
	
#endif	
/********************************************************************************/	


	while(1)
	{
		sleep(1);
	}
 
    return 0;
}














