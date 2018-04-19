/*******************************************************************************
	> File Name: netip.h
	> Author: lroyd
	> Mail: htzhangxmu@163.com
	> Created Time: 
 *******************************************************************************/
#ifndef _NET_USER_API_H_
#define _NET_USER_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "BaseType.h"
#include "netip_server.h"
#include "netip_client.h"


/*******************************************************************************
* Name: 
* Descriptions:
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 TcpServerCreate(UINT16 _usLocalPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun);
INT32 UdpServerCreate(UINT16 _usLocalPort, NET_PROCESSRECVDATA _pUserRecvFun, NET_PROCESSREERROR _pUserErrFun);




/*******************************************************************************
* Name: 
* Descriptions:映射还没解决
* Parameter:	
* Return:	
* *****************************************************************************/
INT32 UdpClientCreate(UINT8 *_pConnIp, UINT16 _usConnPort, NET_PROCESSRECVDATA _pUserRecvFun);
INT32 TcpClientCreate(UINT8 *_pConnIp, UINT16 _usConnPort, NET_PROCESSRECVDATA _pUserRecvFun);

INT32 TcpClientDestroy(INT32 _iUserFd);
INT32 UdpClientDestroy(INT32 _iUserFd);

INT32 TcpClientSendData(INT32 _iUserFd, UINT8 *_aucSndBuf, INT32 _iLen);

INT32 UdpClientSendData(INT32 _iUserFd, UINT8 *_aucSndBuf, INT32 _iLen);
INT32 UdpClientSendDataByIP(INT32 _iUserFd, UINT8 *_pConnIp, UINT16 _usConnPort, UINT8 *_aucSndBuf, INT32 _iLen);

INT32 GetUdpClientNum(void);
INT32 GetTcpClientNum(void);

#ifdef __cplusplus
}
#endif

#endif



