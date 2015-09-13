/*
 * socket.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef COMMUNICATION_SOCKET_H_
#define COMMUNICATION_SOCKET_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include "../thread/thread.h"
#include "../basic/queue.h"
#include "../log/logmanage.h"
#include "../config/configmanage.h"

typedef struct SocketNode
{
	struct sockaddr_in sock_addr;
	int nClientSocket;
	time_t tmAccDateTime;
} SocketNode;

typedef struct Socket
{
	List socketList;
	int nAccThreadLoopSpace;
	int nAccOutTime;
	int nAccOutTimeThreadLoopSpace;
	int nServerSocket;
	Thread *pAccThread;
	Thread *pOutTimeThread;
	int nMaxAcceptSocketNumber;
} Socket;

static Socket serverSocket;

/*接口*/
int Create(int nDomain, int nType, int nProtocol, int nPort, const char *pIp);
int Listen(int nSocket, int nQueSize);
int Accept(int nSocket);
int Close(int nSocket);
void ReleaseSocketNode(SocketNode *pNode);

/*私有*/
void *AcceptSocket(void *pData);
void *FreeOutTimeSocket(void *pData);

#endif /* COMMUNICATION_SOCKET_H_ */
