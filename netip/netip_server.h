/*******************************************************************************
	> File Name: netip_server.h
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#ifndef _NET_TCP_SERVER_H_
#define _NET_TCP_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "event.h"

#define RECV_BUF_SIZE_MAX (1500)

#define NETIP_ADDR_SIZE_MAX (16)


typedef enum
{
	NETIP_TCP	= 0,
	NETIP_UDP,
	NETIP_MAX,
}NETIP_TYPE;





typedef enum
{
	NETIP_RET_SOCKET_CLOSE	= 0x0001,	//目前只有TCP有
	
}NETIP_RET_CODE;


/* 连接的客户端信息 */
typedef struct _tagClientNode
{
	SOCKET				m_tSocket;	
	UINT8				m_ucClientIp[NETIP_ADDR_SIZE_MAX];
	UINT16				m_usClientPort;   
	void				*pThis;	//用户不可用
}T_PeerNodeInfo, *PT_PeerNodeInfo;	

typedef INT32 (*NET_PROCESSRECVDATA)(PT_PeerNodeInfo, UINT8 *, INT32);
typedef INT32 (*NET_PROCESSREERROR)(PT_PeerNodeInfo, NETIP_RET_CODE);
typedef INT32 (*_HANDLE)(void *);

typedef struct _tagNetEntity
{
    T_EventInfo			in_tEvent;
    SOCKET				m_tSocket;
    UINT16				m_usPort;
    void 				*m_pManger;
    NET_PROCESSRECVDATA	pUserRcvCbk;
    NET_PROCESSREERROR	pUserErrCbk;
}T_NetEntity,*PT_NetEntity;


typedef struct _tagNetServer
{
	T_NetEntity		in_tEntity;
}T_NetServer, *PT_NetServer;


/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
PT_NetServer NetServerCreate(void *_pManger);
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 NetServerInit(NETIP_TYPE _emNetType, PT_NetServer _pServer, UINT16 _usPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun, UINT8 *_cIP);
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpPeerDisconnect(PT_PeerNodeInfo _pClient);

INT32 GetTcpServerNodeNum(INT32 _iSocket);
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpServerSnd(PT_PeerNodeInfo _pConnect, UINT8 *_aucSndBuf, INT32 _iLen);
/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 UdpServerSnd(PT_PeerNodeInfo _pClient, UINT8 *_aucSndBuf, INT32 _iLen);


#ifdef __cplusplus
}
#endif

#endif



