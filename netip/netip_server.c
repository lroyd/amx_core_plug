/*******************************************************************************
	> File Name: netip_server.c
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#include "netip_server.h"
#include "log_file.h"

#define POOL_CONNECT_NODE_MAX  (3000)		//最大连接池上线



#define TAG_NET_SERVER_TCP ("TCP Server")
#define TAG_NET_SERVER_UDP ("UDP Server")

/*为每一个连接建立一个结构,记录连接信息*/
typedef struct _tagConnectNode
{
    T_EventInfo			in_tEvent;
	T_PeerNodeInfo		in_tPeer;
	
    UINT8				m_aucRecvBuf[RECV_BUF_SIZE_MAX];	//数据接收缓冲	
	UINT32				m_uiRecvLen;				//接收到的数据长度
	UINT32				m_uiState;		//

	//void 				*m_pPeer;		//not use
	//void 				*m_pData;		//not use
	NET_PROCESSRECVDATA pConnRcvCbk; 
	NET_PROCESSREERROR  pConnErrCbk;
}T_ConNodeInfo, *PT_ConNodeInfo;	

/* TCP 使用缓冲池需要有较大的空间 3000 * 1500 */
typedef struct _tagConnectPool 
{
    PT_ConNodeInfo	m_ptFreeData[POOL_CONNECT_NODE_MAX];
    UINT32			m_iFreeDataCnt;
	VarLock			m_Lock;
}T_ConnNodePool, *PT_ConnNodePool;

static INT32 tcp_accept_handle(void *);
static INT32 udp_event_handle(void *);


const _HANDLE g_tServerHandle[NETIP_MAX] = {
	tcp_accept_handle,
	udp_event_handle,
};

static T_ConnNodePool g_tConnNodePool; //only tcp

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
static PT_ConNodeInfo connect_node_malloc(void)
{
    void * pNode = NULL;
    LockVarLock(g_tConnNodePool.m_Lock);
    if(g_tConnNodePool.m_iFreeDataCnt > 0)
    {
        pNode = g_tConnNodePool.m_ptFreeData[--g_tConnNodePool.m_iFreeDataCnt];
        UnLockVarLock(g_tConnNodePool.m_Lock);
    }
    else
    {
        UnLockVarLock(g_tConnNodePool.m_Lock);
        pNode = (void *)malloc(sizeof(T_ConNodeInfo));
		if(pNode)
		{
			memset(pNode,0,sizeof(T_ConNodeInfo));
		}
    }
    return pNode;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static void connect_node_free(void * _pNode)
{
    PT_ConNodeInfo pNode =_pNode;
    if(pNode)
    {
        memset(pNode,0,sizeof(T_ConNodeInfo));
        LockVarLock(g_tConnNodePool.m_Lock);
        if(g_tConnNodePool.m_iFreeDataCnt < POOL_CONNECT_NODE_MAX)
        {
            g_tConnNodePool.m_ptFreeData[g_tConnNodePool.m_iFreeDataCnt++]=pNode;
            UnLockVarLock(g_tConnNodePool.m_Lock);
        }
        else
        {
            UnLockVarLock(g_tConnNodePool.m_Lock);
            free(pNode);
        }
    }
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
*******************************************************************************/
static INT32 connect_close(void *_pThis)
{
    PT_ConNodeInfo pThis = _pThis;
    if(pThis->pConnErrCbk)
    {
        pThis->pConnErrCbk((void *)&pThis->in_tPeer, NETIP_RET_SOCKET_CLOSE);
    }
    EventCancel(pThis);
	syslog_wrapper(LOG_DEBUG, "tcp server: peer close fd = %d",pThis->in_tPeer.m_tSocket);
    close2(pThis->in_tPeer.m_tSocket);
    pThis->in_tPeer.m_tSocket	= INVALID_SOCKET;
    pThis->m_uiRecvLen	= 0;
    connect_node_free(pThis);
    return 0;
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
		syslog_wrapper(LOG_WARNING, "tcp server: peer fd is INVALID_SOCKET!!");
		return -1;
	}
	if (_pData == NULL) 
	{
		syslog_wrapper(LOG_WARNING, "tcp server: peer receive data is null!!");
		return -1;
	}		

	iRet = recv(_tSocket, _pData, _uiLen, 0);
    if (iRet <= 0) 
    {
		if (iRet == 0 || errno != EAGAIN) 
		{
			/* 有可能是关闭逻辑 */
	        return -1;
		} 
		else 
		{
			/* recv 失败 -1 */
			syslog_wrapper(LOG_FATAL, "tcp server: peer receive error!!");
			return 0;
		}
    }
    return iRet;
}
/*******************************************************************************
* Name: 
* Descriptions:注意 正常关闭也会进来
* Parameter:	
* Return:	
* *****************************************************************************/
static INT32 connect_event_handle(void * _pThis)
{
    INT32 iRet = 0, iContinue = 0;
    PT_ConNodeInfo pThis = _pThis;
	pThis->m_uiRecvLen = 0;

	do
	{
		if(pThis->m_uiRecvLen == RECV_BUF_SIZE_MAX)
		{
			do 
			{
				iRet = tcp_read(pThis->in_tPeer.m_tSocket, pThis->m_aucRecvBuf, RECV_BUF_SIZE_MAX);			
			} while (iRet == RECV_BUF_SIZE_MAX);
			syslog_wrapper(LOG_WARNING, "tcp server: peer receive full buff error!!");
			pThis->m_uiRecvLen = 0;
			goto LAB_EXIT;
		}
		iRet = tcp_read(pThis->in_tPeer.m_tSocket, pThis->m_aucRecvBuf + pThis->m_uiRecvLen, RECV_BUF_SIZE_MAX - pThis->m_uiRecvLen);
		if (iRet <= 0) 
		{
			goto LAB_EXIT;
		}
		pThis->m_uiRecvLen += iRet;
		iContinue = (pThis->m_uiRecvLen == RECV_BUF_SIZE_MAX) ? 1 : 0;
	
		//if (pThis->pConnRcvCbk)
		{
			pThis->pConnRcvCbk((void *)&pThis->in_tPeer, pThis->m_aucRecvBuf, pThis->m_uiRecvLen);
		}

	} while (iContinue);
	
LAB_EXIT:
	if(iRet < 0)
	{
		/* 关闭逻辑? */
		connect_close(pThis);
	}
    return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static INT32 tcp_accept_handle(void * _pServer)
{
    PT_ConNodeInfo pConnect = NULL;
    PT_NetEntity pServer = _pServer;    
    SOCKET	nSockClient = -1;           
    struct sockaddr_in 	tAddrClient;      
	
	socklen_t nAddrClientLen = sizeof(struct sockaddr_in); 
          	
    struct linger so_linger;	
    int keepAlive		= 1; 	//开启keepalive属性
	int keepIdle		= 5; 	//如该连接在20秒内没有任何数据往来,则进行探测 
	int keepInterval	= 5; 	//探测时发包的时间间隔为5 秒
	int keepCount		= 2; 	//探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.

    nSockClient = accept(pServer->m_tSocket, (struct sockaddr *) &tAddrClient, &nAddrClientLen);
    if(nSockClient)
    {
	    so_linger.l_onoff	= 1;
	    so_linger.l_linger	= 0;
	    setsockopt(nSockClient, SOL_SOCKET,	SO_LINGER,		&so_linger,				sizeof(so_linger));
    	setsockopt(nSockClient, SOL_SOCKET,	SO_KEEPALIVE,	(void *)&keepAlive,		sizeof(keepAlive));
    	setsockopt(nSockClient, SOL_TCP,	TCP_KEEPIDLE,	(void *)&keepIdle,		sizeof(keepIdle));	
    	setsockopt(nSockClient, SOL_TCP,	TCP_KEEPINTVL,	(void *)&keepInterval,	sizeof(keepInterval));
    	setsockopt(nSockClient, SOL_TCP,	TCP_KEEPCNT,	(void *)&keepCount,		sizeof(keepCount));
        set_nonblocking(nSockClient);
        pConnect = connect_node_malloc();
        if(pConnect)
        {			
            pConnect->in_tPeer.m_tSocket		= nSockClient;
			sprintf(pConnect->in_tPeer.m_ucClientIp,"%s", inet_ntoa(tAddrClient.sin_addr));
			pConnect->in_tPeer.m_usClientPort	= ntohs(tAddrClient.sin_port);
			pConnect->in_tPeer.pThis			= pConnect;	//释放的时候使用
			syslog_wrapper(LOG_DEBUG, "tcp server: new peer accept IP =[%s]-[%d]", pConnect->in_tPeer.m_ucClientIp, pConnect->in_tPeer.m_usClientPort);
			pConnect->in_tEvent.m_emType	= EVENT_INLT;	//EVENT_INLT;
			pConnect->in_tEvent.m_iEventFD	= nSockClient;
			pConnect->in_tEvent.m_Handle	= connect_event_handle;		
			
            pConnect->pConnRcvCbk			= pServer->pUserRcvCbk;            
            pConnect->pConnErrCbk			= pServer->pUserErrCbk;
            EventRegister((void *)pConnect, (void *)pServer->m_pManger);    
        }
        else
        {
            close2(nSockClient);
			syslog_wrapper(LOG_ERROR, "tcp server: new peer accept but connect_node_malloc error!!");
        }
    }
    return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
static INT32 udp_event_handle(void *_pThis)
{
	PT_NetEntity pThis = _pThis;	
	T_PeerNodeInfo pPeerInfo = {0};  //使用完失效,只是peer信息
    UINT8 aucRcvBuf[RECV_BUF_SIZE_MAX] = {0}; //使用完失效
    INT32 iRcvLen = 0;	
    struct sockaddr_in tPeerAddr;   
	socklen_t nAddrLen = sizeof(struct sockaddr_in); 

    do  //bug 一直在while 出不去？非阻塞模式下 recvfrom 直接返回
    {
        iRcvLen = recvfrom(pThis->m_tSocket, aucRcvBuf, 1500,0,(struct sockaddr *) &tPeerAddr, &nAddrLen); 
    	if (iRcvLen > 0)
    	{
			pPeerInfo.m_tSocket = pThis->m_tSocket;
			sprintf(pPeerInfo.m_ucClientIp,"%s", inet_ntoa(tPeerAddr.sin_addr));
			pPeerInfo.m_usClientPort = ntohs(tPeerAddr.sin_port);
			syslog_wrapper(LOG_TRACE, "udp server: peer IP =[%s]-[%d] receive data", pPeerInfo.m_ucClientIp, pPeerInfo.m_usClientPort);
			pThis->pUserRcvCbk(&pPeerInfo, aucRcvBuf, iRcvLen);

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
PT_NetServer NetServerCreate(void *_pManger)
{    
	PT_NetServer pNetServer = malloc(sizeof(T_NetServer));
	if(pNetServer)
	{
		memset(pNetServer,0,sizeof(T_NetServer));
		pNetServer->in_tEntity.m_pManger = _pManger;
	}
	return pNetServer;
}
/*******************************************************************************
* Name: 
* Descriptions: 服务器必须是异步接收
* Parameter:	_cIP:返回本地的IP
* Return:	-1错误/socket fd
* *****************************************************************************/
INT32 NetServerInit(NETIP_TYPE _emNetType, PT_NetServer _pServer, UINT16 _usLocalPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun, UINT8 *_cIP)
{    
	INT32	iRet = -1;
	struct sockaddr_in 	tAddrServ;        		
	struct sockaddr_in 	tAddrLocal;        
	T_NetEntity *pServer = (T_NetEntity *)&_pServer->in_tEntity;  //server里面只有一个实体，添加其他成员时需要修改

	socklen_t nAddrLen = sizeof(struct sockaddr_in); 
    
	pServer->m_usPort			= _usLocalPort;
	
	if (_pUserRecvFun == NULL)
	{
		syslog_wrapper(LOG_ERROR, "net %s server init: user call back function can not empty!!", _emNetType?"udp":"tcp");
	}
	pServer->pUserRcvCbk		= _pUserRecvFun;
	pServer->pUserErrCbk		= _pUserErrFun;
	
	tAddrServ.sin_family		= AF_INET;
	tAddrServ.sin_addr.s_addr	= htonl(INADDR_ANY);
	tAddrServ.sin_port			= htons(pServer->m_usPort);

	if(NETIP_TCP == _emNetType)
		pServer->m_tSocket	= socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	else
		pServer->m_tSocket	= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if (pServer->m_tSocket < 0)
	{
		syslog_wrapper(LOG_FATAL, "net %s server init: _socket error", _emNetType?"udp":"tcp");
		return -1;		
	}
	
	INT32 op = 1;
	setsockopt(pServer->m_tSocket, SOL_SOCKET, SO_REUSEADDR, (void *)&op, sizeof (op));
	
	set_nonblocking(pServer->m_tSocket);
	
	iRet = bind(pServer->m_tSocket, (struct sockaddr *)&tAddrServ, sizeof(tAddrServ));
	if (iRet < 0)
	{
		close(pServer->m_tSocket);
		syslog_wrapper(LOG_FATAL, "net %s server init: _bind error", _emNetType?"udp":"tcp");
		return -1;
	}
	
	if (_cIP)
	{
		getsockname(pServer->m_tSocket,(struct sockaddr *)&tAddrLocal, &nAddrLen);
		strcpy(_cIP,inet_ntoa(tAddrLocal.sin_addr));
		syslog_wrapper(LOG_DEBUG, "net %s server init: local IP =[%s]-[%d]", _emNetType?"udp":"tcp", _cIP, pServer->m_usPort);
	}

	pServer->in_tEvent.m_emType		= EVENT_INLT;
	pServer->in_tEvent.m_iEventFD	= pServer->m_tSocket;
	pServer->in_tEvent.m_Handle		= g_tServerHandle[_emNetType];
	EventRegister((void *)pServer, (void *)pServer->m_pManger);  

	if (NETIP_TCP == _emNetType)
	{
		iRet = listen(pServer->m_tSocket,100);
		if (iRet < 0)
		{
			close(pServer->m_tSocket);
			syslog_wrapper(LOG_FATAL, "net %s server init: _listen error", _emNetType?"udp":"tcp");
			return -1;
		}		
	}
	
	return pServer->m_tSocket;
}
/*******************************************************************************
* Name: 
* Descriptions: 
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpPeerDisconnect(PT_PeerNodeInfo _pClient)
{
    INT32 iRet = 0;
    PT_ConNodeInfo pNode = _pClient->pThis; //注意这里可能会出错，节点的有效性
	
	if(pNode)
	{
		connect_close(pNode);
	}
    return iRet;
}
/*******************************************************************************
* Name: 
* Descriptions: 
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 GetTcpServerNodeNum(INT32 _iSocket)
{
	
	
	return 0;
}
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpServerSnd(PT_PeerNodeInfo _pClient, UINT8 *_aucSndBuf, INT32 _iLen)
{
    INT32 iRet = 0;
    PT_PeerNodeInfo pClient =_pClient;

    if(pClient && _aucSndBuf && pClient->m_tSocket != INVALID_SOCKET)
    {
        iRet = write2(pClient->m_tSocket,_aucSndBuf,_iLen);
		syslog_wrapper(LOG_TRACE, "tcp server send data to fd = %d, size = %d", pClient->m_tSocket, iRet);
    }
    return iRet;
}

/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 UdpServerSnd(PT_PeerNodeInfo _pClient, UINT8 *_aucSndBuf, INT32 _iLen)
{
	PT_PeerNodeInfo pClient =_pClient;
    struct sockaddr_in	tAddrSnd;    
    tAddrSnd.sin_family	= AF_INET;
	/* 注意字节序 */
    tAddrSnd.sin_port	= htons(pClient->m_usClientPort);
	inet_aton(pClient->m_ucClientIp, &tAddrSnd.sin_addr);
	
    sendto(pClient->m_tSocket, _aucSndBuf, _iLen, 0, (struct sockaddr *)&tAddrSnd, sizeof(struct sockaddr));   
	syslog_wrapper(LOG_TRACE, "udp server send data to fd = %d, size = %d", pClient->m_tSocket, _iLen);
    return 0;
}




