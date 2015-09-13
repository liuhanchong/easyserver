/*
 * servermanage.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef SERVER_SERVERMANAGE_H_
#define SERVER_SERVERMANAGE_H_

#include <sys/shm.h>
#include "../log/logmanage.h"
#include "../communication/socket.h"
#include "../communication/aiox.h"
#include "../data/data.h"
#include "../config/configmanage.h"
#include "../log/log.h"

#define KEY 0x19900510
#define MAXROWLENGTH 500

typedef struct ShareMemory
{
	pid_t proId;
	int nRun;
} ShareMemory;

typedef struct Server
{
	ShareMemory *pShareMemory;
	int nServerSocket;
    int nShareMemoryId;
    AioX aioX;
} Server;

Server server;

/*接口*/
int StartServer();
int StopServer();
int RestartServer();

/*私有*/
int CreateShareMemory();
int ReleaseShareMemory();
void StopServerSignal(int nSignalType);
int AddSocketEvent(int nClientSocket, SocketNode *pSocketNode);
int RecvData(SocketNode *pSocketNode, void *pData, int nDataSize);

#endif /* SERVER_SERVERMANAGE_H_ */
