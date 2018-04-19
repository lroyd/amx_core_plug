/*******************************************************************************
	> File Name: netip_client.c
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#include "netip_client.h"
#include "log_file.h"
#include "netip_server.h"


#define TAG_NET_CLIENT ("NET Client")


static INT32 tcp_recv_handle(void *);
static INT32 udp_recv_handle(void *);

const _HANDLE g_tClientHandle[NETIP_MAX] = 
{
	tcp_recv_handle,
	udp_recv_handle,
};


/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static INT32 set_blocking(INT32 _iSocket)
{
    return fcntl(_iSocket, F_SETFL, fcntl(_iSocket, F_GETFD, 0)&~O_NONBLOCK);
}

static INT32 set_nonblocking(INT32 _iSocket)
{
    return fcntl(_iSocket, F_SETFL, fcntl(_iSocket, F_GETFD, 0)|O_NONBLOCK);
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static INT32 tcp_read(SOCKET _tSocket, UINT8 *_pData, UINT32 _uiLen)
{
    INT32 iRet = 0;
    if (_tSocket == INVALID_SOCKET) 
    {
		DLOGE(TAG_NET_CLIENT,"{%s:%d} INVALID_SOCKET",__FUNCTION__, __LINE__);
		return -1;
	}
	if (_pData == NULL) 
	{
		DLOGE(TAG_NET_CLIENT,"{%s:%d} data is null",__FUNCTION__, __LINE__);
		return -1;
	}		

	iRet = recv(_tSocket, _pData, _uiLen, 0);
    if (iRet <= 0) 
    {
		if (iRet == 0 || errno != EAGAIN) 
		{
			/* �п����ǹر��߼� */
			//DLOGE(TAG_NET_CLIENT,"{%s:%d} _recv error",__FUNCTION__, __LINE__);
	        return -1;
		} 
		else 
		{
			/* recv ʧ�� -1 */
			DLOGE(TAG_NET_CLIENT,"{%s:%d}",__FUNCTION__, __LINE__);
			return 0;
		}
    }
    return iRet;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static INT32 tcp_recv_handle(void *_pThis)
{
	PT_NetEntity pThis = _pThis;	
	T_PeerNodeInfo pPeerInfo = {0};  //ʹ����ʧЧ,ֻ��peer��Ϣ
    UINT8 aucRcvBuf[RECV_BUF_SIZE_MAX] = {0}; //ʹ����ʧЧ
    INT32 iRcvLen = 0, iRet = -1, iContinue = 0;	
    struct sockaddr_in tPeerAddr;   
	socklen_t nAddrLen = sizeof(struct sockaddr_in); 

	do
	{
		if(iRcvLen == RECV_BUF_SIZE_MAX)
		{
			do
			{
				iRet = tcp_read(pThis->m_tSocket, aucRcvBuf, RECV_BUF_SIZE_MAX);			
			} while (iRet == RECV_BUF_SIZE_MAX);
			
			DLOGE(TAG_NET_CLIENT,"{%s:%d} receive full buff error",__FUNCTION__, __LINE__);
			iRcvLen = 0;
			goto LAB_EXIT;
		}
		iRet = tcp_read(pThis->m_tSocket, aucRcvBuf + iRcvLen, RECV_BUF_SIZE_MAX - iRcvLen);
		if (iRet <= 0) 
		{
			//DLOGE(TAG_NET_CLIENT,"{%s:%d} _tcp_read error",__FUNCTION__, __LINE__);
			goto LAB_EXIT;
		}
		iRcvLen += iRet;
		iContinue = (iRcvLen == RECV_BUF_SIZE_MAX) ? 1 : 0;
	
		if (pThis->pUserRcvCbk)
		{
			pPeerInfo.m_tSocket = pThis->m_tSocket;
			pThis->pUserRcvCbk(&pPeerInfo, aucRcvBuf, iRcvLen);
		}
	} while (iContinue);

LAB_EXIT:
	if(iRet < 0)
	{
		/* SERV�ر��߼����������, �����Ͽ�������� */
		DLOGE(TAG_NET_CLIENT,"xxxxxxxxxxxxxx");
		close(pThis->m_tSocket);
		/* �ͷ�tcpʵ�� */
		
	}
	
	return 0;	
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static INT32 udp_recv_handle(void *_pThis)
{
	PT_NetEntity pThis = _pThis;	
	T_PeerNodeInfo pPeerInfo = {0};  //ʹ����ʧЧ,ֻ��peer��Ϣ
    UINT8 aucRcvBuf[RECV_BUF_SIZE_MAX] = {0}; //ʹ����ʧЧ
    INT32 iRcvLen = 0;	
    struct sockaddr_in tPeerAddr;   
	socklen_t nAddrLen = sizeof(struct sockaddr_in); 

    do  
    {
        iRcvLen = recvfrom(pThis->m_tSocket, aucRcvBuf, 1500,0,(struct sockaddr *) &tPeerAddr, &nAddrLen); //2s����ʱ��

    	if (iRcvLen > 0)
    	{
			pPeerInfo.m_tSocket = pThis->m_tSocket;
			sprintf(pPeerInfo.m_ucClientIp,"%s", inet_ntoa(tPeerAddr.sin_addr));
			pPeerInfo.m_usClientPort = ntohs(tPeerAddr.sin_port);
			//DLOGD(TAG_NET_SERVER_UDP,"peer IP :[%s] [%d] ", pPeerInfo.m_ucClientIp, pPeerInfo.m_usClientPort);
			
            if (pThis->pUserRcvCbk)
            {
                pThis->pUserRcvCbk(&pPeerInfo, aucRcvBuf, iRcvLen);
            }
    	}
	}while(iRcvLen > 0);

	return 0;	
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
PT_NetClient NetClientCreate(void *_pManger, NETIP_CLI_RCV_MODE _emNetRcvMode)
{    
	PT_NetClient pNetClient = malloc(sizeof(T_NetClient));
	if(pNetClient)
	{
		memset(pNetClient,0,sizeof(T_NetClient));
		pNetClient->in_tEntity.m_pManger	= _pManger;
		pNetClient->m_iCliRcvMode			= _emNetRcvMode;
	}
	return pNetClient;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 NetClientDestroy(PT_NetClient _pClient)
{
	PT_NetClient pClient = _pClient;
	if(pClient)
	{
		if (CLI_RCV_ASYNC == pClient->m_iCliRcvMode)
		{
			/* ɾ���¼� */
			EventCancel((void *)&pClient->in_tEntity);
		}
		if (pClient->in_tEntity.m_tSocket)
		{
			close(pClient->in_tEntity.m_tSocket);
			pClient->in_tEntity.m_tSocket = -1;
		}
		/* ���� */
		free(pClient);		
	}
	return 0;
}
/*******************************************************************************
* Name: Ŀǰֻ��UDP
* Descriptions: _pClient �а���_pManger
* Parameter:	
* Return:	INT32:fd
* *****************************************************************************/
INT32 NetClientInit(NETIP_TYPE _emNetType, PT_NetClient _pClient, UINT8 *_pConnIp, UINT16 _usConnPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun)
{
	INT32 iRet = -1; 
	
	PT_NetClient pClient = _pClient;
	
	if(NETIP_TCP == _emNetType)
	{
		pClient->in_tEntity.m_tSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else
	{
		pClient->in_tEntity.m_tSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	
	if(pClient->in_tEntity.m_tSocket < 0)   
	{   
		DLOGE(TAG_NET_CLIENT,"{%s:%d} _socket error",__FUNCTION__,__LINE__);
		return -1;
	} 		
	
	/* ���ÿͻ��˽��ճ�ʱ2s */
	struct timeval timeOut;
    timeOut.tv_sec = 2;                
    timeOut.tv_usec = 0;
    setsockopt(pClient->in_tEntity.m_tSocket, SOL_SOCKET, SO_RCVTIMEO, &timeOut, sizeof(timeOut));	

	memcpy(&pClient->m_aucServIp[0], _pConnIp, strlen(_pConnIp));
	pClient->m_usServPort 			= _usConnPort;	
	pClient->in_tEntity.pUserRcvCbk	= _pUserRecvFun; 
	pClient->in_tEntity.pUserErrCbk	= _pUserErrFun;
		
	if(NETIP_TCP == _emNetType)
	{
		/* tcp connect */
		struct sockaddr_in servaddr;  
		memset(&servaddr, 0, sizeof(servaddr));  
		servaddr.sin_family = AF_INET;  
		servaddr.sin_port = htons(_usConnPort);  			//�������˿�  
		servaddr.sin_addr.s_addr = inet_addr(_pConnIp); 	//������ip  
		  
		//���ӷ��������ɹ�����0�����󷵻�-1  
		if (connect(pClient->in_tEntity.m_tSocket, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)  
		{  
			DLOGE(TAG_NET_CLIENT,"{%s:%d} _connect error",__FUNCTION__,__LINE__);
			return -1;
		} 		
	}	
	
	if(CLI_RCV_ASYNC == pClient->m_iCliRcvMode)
	{
		/* �첽���� */
		//set_nonblocking(pClient->in_tEntity.m_tSocket);  //���÷�����,ֱ�ӷ���
		pClient->in_tEntity.in_tEvent.m_emType		= EVENT_INLT;
		pClient->in_tEntity.in_tEvent.m_iEventFD	= pClient->in_tEntity.m_tSocket;
		pClient->in_tEntity.in_tEvent.m_Handle		= g_tClientHandle[_emNetType];
		EventRegister((void *)&(pClient->in_tEntity), (void *)pClient->in_tEntity.m_pManger);
	}
	else
	{
		set_blocking(pClient->in_tEntity.m_tSocket);  //��������,��ʱ����
	}
	
	return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:�������������٣�ʵ����Ȼ����
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 NetClientClose(PT_NetClient _pClient)
{
	close(_pClient->in_tEntity.m_tSocket);
    return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpClientSnd(PT_NetClient _pClient, UINT8 *_aucSndBuf, INT32 _iLen)
{
    INT32 iRet = -1;
    PT_NetClient pClient =_pClient;

    if(pClient && _aucSndBuf && pClient->in_tEntity.m_tSocket != INVALID_SOCKET)
    {
        iRet = write2(pClient->in_tEntity.m_tSocket, _aucSndBuf, _iLen);
    }
    return iRet;
}
/*******************************************************************************
* Name: 
* Descriptions:����socket ���Ͳ�ͬ��server ip
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 UdpClientSndByIp(PT_NetClient _pClient, UINT8 *_pConnIp, UINT16 _usConnPort, UINT8 *_aucSndBuf, INT32 _iLen)
{
	PT_NetClient pClient = _pClient;	
  	struct sockaddr_in tServInfo;
	
	/* ������˵����� */
	bzero(&tServInfo,sizeof(struct sockaddr_in));   
	tServInfo.sin_family = AF_INET;   
	tServInfo.sin_addr.s_addr = inet_addr(_pConnIp); 
	tServInfo.sin_port = htons(_usConnPort);  

	sendto(pClient->in_tEntity.m_tSocket, _aucSndBuf, _iLen, 0, (struct sockaddr *)&tServInfo, sizeof(struct sockaddr));   
	
	if (CLI_RCV_SYNC == pClient->m_iCliRcvMode)
	{
		/* ͬ�������ȴ����� */
		udp_recv_handle(&pClient->in_tEntity);
	}
	
    return 0;
}


