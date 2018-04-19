/*******************************************************************************
	> File Name: netip_client.h
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#ifndef _NET_TCP_CLIENT_H_
#define _NET_TCP_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "event.h"
#include "netip_server.h"

typedef enum
{
	CLI_RCV_SYNC	= 0,	//�ͻ���ͬ������ģʽ
	CLI_RCV_ASYNC,			//�ͻ����첽����ģʽ
}NETIP_CLI_RCV_MODE;

/* ����clientʵ�� */
typedef struct _tagNetClient
{
	T_NetEntity		in_tEntity;
	INT32			m_iCliRcvMode;	//�ͻ��˽���ģʽ
	
	/* server info */
	UINT8			m_aucServIp[NETIP_ADDR_SIZE_MAX];
	UINT16			m_usServPort;
}T_NetClient, *PT_NetClient;


/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
PT_NetClient NetClientCreate(void *_pManger, NETIP_CLI_RCV_MODE _emNetRcvMode);
INT32 NetClientDestroy(PT_NetClient _pClient);

INT32 NetClientInit(NETIP_TYPE _emNetType, PT_NetClient _pClient, UINT8 *_pConnIp, UINT16 _usConnPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun);
INT32 NetClientClose(PT_NetClient _pClient);

INT32 TcpClientSnd(PT_NetClient _pClient, UINT8 *_aucSndBuf, INT32 _iLen);
INT32 UdpClientSnd(PT_NetClient _pClient, UINT8 *_aucSndBuf, INT32 _iLen);
INT32 UdpClientSndByIp(PT_NetClient _pClient, UINT8 *_pConnIp, UINT16 _usConnPort, UINT8 *_aucSndBuf, INT32 _iLen);

#ifdef __cplusplus
}
#endif

#endif



