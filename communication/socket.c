/*
 * socket.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "socket.h"
#include "../server/servermanage.h"

int Create(int nDomain, int nType, int nProt, int nPort, const char *pIp)
{
	/*必须是TCP/IP协议的套接字创建*/
	if (!pIp || nDomain != AF_INET || nType != SOCK_STREAM)
	{
		ErrorInfor("Create", ERROR_TRANTYPE);
		return -1;
	}

	serverSocket.nAccOutTime = serverConfig.nAccSocketOutTime;
	serverSocket.nAccOutTimeThreadLoopSpace = serverConfig.nAccOutTimeSocketLoopSpace;
	serverSocket.nMaxAcceptSocketNumber = serverConfig.nMaxAcceptSocketNumber;
	serverSocket.nAccThreadLoopSpace = serverConfig.nAccSocketThreadLoopSpace;

	if (InitQueue(&serverSocket.socketList, serverSocket.nMaxAcceptSocketNumber, 0) == 0)
	{
		ErrorInfor("Create", ERROR_INITQUEUE);
		return 0;
	}

	/*创建套接字*/
	int nSocket = socket(nDomain, nType, nProt);
	if (nSocket == -1)
	{
		SystemErrorInfor("Create-1", errno);
		ReleaseQueue(&serverSocket.socketList);
		return -1;
	}

	/*设置套接字选项*/
	int nReuse = 1;
	if (setsockopt(nSocket, SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof(nReuse)) == -1)
	{
		SystemErrorInfor("Create-2", errno);
	}

	/*绑定套接字*/
	serverSocket.nServerSocket  = nSocket;
	serverSocket.pAccThread     = NULL;
	serverSocket.pOutTimeThread = NULL;

	struct sockaddr_in sock_addr;
	sock_addr.sin_family 	  = nDomain;
	sock_addr.sin_port 		  = htons(nPort);
	sock_addr.sin_addr.s_addr = inet_addr(pIp);
	if (bind(nSocket, (struct sockaddr *)&sock_addr, sizeof(struct sockaddr_in)) == -1)
	{
		SystemErrorInfor("Create-3", errno);
		ReleaseQueue(&serverSocket.socketList);
		close(nSocket);
		return -1;
	}

	return nSocket;
}

int Listen(int nSocket, int nQueSize)
{
	if (nSocket < 0 || nQueSize <= 0)
	{
		ErrorInfor("Listen", ERROR_TRANTYPE);
		return 0;
	}

	if (listen(nSocket, nQueSize) != 0)
	{
		SystemErrorInfor("Listen", errno);
		return 0;
	}

	return 1;
}

int Accept(int nSocket)
{
	if (nSocket < 0)
	{
		ErrorInfor("Accept", ERROR_TRANTYPE);
		return 0;
	}

	serverSocket.pAccThread = CreateLoopThread(AcceptSocket, NULL, serverSocket.nAccThreadLoopSpace);
	if (!serverSocket.pAccThread)
	{
		ErrorInfor("Accept-1", ERROR_CRETHREAD);
		return 0;
	}

	serverSocket.pOutTimeThread = CreateLoopThread(FreeOutTimeSocket, NULL, serverSocket.nAccOutTimeThreadLoopSpace);
	if (!serverSocket.pOutTimeThread)
	{
		ErrorInfor("Accept-2", ERROR_CRETHREAD);
		return 0;
	}

	return 1;
}

int Close(int nSocket)
{
	if (nSocket < 0)
	{
		ErrorInfor("Close", ERROR_ARGNULL);
		return 0;
	}

	if (serverSocket.pAccThread)
	{
		ReleaseThread(serverSocket.pAccThread);
	}

	if (serverSocket.pOutTimeThread)
	{
		ReleaseThread(serverSocket.pOutTimeThread);
	}

	BeginTraveData(&serverSocket.socketList);
		ReleaseSocketNode((SocketNode *)pData);
	EndTraveData();

	ReleaseQueue(&serverSocket.socketList);

	if (close(nSocket) != 0)
	{
		SystemErrorInfor("Close", errno);
		return 0;
	}

	return 1;
}

void ReleaseSocketNode(SocketNode *pNode)
{
	if (pNode)
	{
		if (close(pNode->nClientSocket) != 0)
		{
			ErrorInfor("ReleaseSocketNode", ERROR_CLOSOCKET);
		}

		free(pNode);
		pNode = NULL;
	}
}

void *AcceptSocket(void *pData)
{
	//接收异步执行
	int nClientSocket = 0;
	int nSerSocket 	  = 0;
	int nLen 		  = 0;
	struct sockaddr_in sock_addr;
	nSerSocket = serverSocket.nServerSocket;
	nLen = sizeof(struct sockaddr_in);
	nClientSocket = accept(nSerSocket, (struct sockaddr *)&sock_addr, (socklen_t *)&nLen);

	/*异步读取数据*/
	if (nClientSocket < 0)
	{
		SystemErrorInfor("AcceptSocket-1", errno);
		return NULL;
	}

	//将客户端socket设置为non_blocked
	if (fcntl(nClientSocket, F_SETFL, O_NONBLOCK) == -1)
	{
		SystemErrorInfor("AcceptSocket-2", errno);
	}

	SocketNode *pSocketNode = (SocketNode *)malloc(sizeof(SocketNode));
	if (!pSocketNode)
	{
		SystemErrorInfor("AcceptSocket-3", errno);
		return NULL;
	}

	pSocketNode->sock_addr 	   = sock_addr;
	pSocketNode->nClientSocket = nClientSocket;
	pSocketNode->tmAccDateTime = time(NULL);

	//为socket增加监听事件
	if (AddSocketEvent(nClientSocket, pSocketNode) == 0)
	{
		ErrorInfor("AcceptSocket", ERROR_ADDEVENT);
		return NULL;
	}

	/*插入队列*/
	LockQueue(&serverSocket.socketList);
	if (!Insert(&serverSocket.socketList, (void *)pSocketNode, 0))
	{
		ReleaseSocketNode(pSocketNode);
		ErrorInfor("AcceptSocket", ERROR_INSOCKET);
	}
	UnlockQueue(&serverSocket.socketList);

	return NULL;
}

void *FreeOutTimeSocket(void *pData)
{
	/*遍历队列列表*/
	LockQueue(&serverSocket.socketList);

	BeginTraveData(&serverSocket.socketList);
		time_t tmCurTime = time(NULL);
		SocketNode *pSocketNode = (SocketNode *)pData;
		if (((tmCurTime - pSocketNode->tmAccDateTime) >= serverSocket.nAccOutTime))
		{
			ReleaseSocketNode(pSocketNode);
			DeleteForNode(&serverSocket.socketList, pQueueNode);
		}
	EndTraveData();

	UnlockQueue(&serverSocket.socketList);

	return NULL;
}

