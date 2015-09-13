/*
 * servermanage.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "servermanage.h"

int StartServer()
{
	if (!InitServerConfig(MAXROWLENGTH))
	{
		printf("%s-初始化配置失败!\n", "StartServer");
		return 0;
	}

	if (!InitLog())
	{
		ErrorInfor("StartServer", ERROR_INITLOG);
		return 0;
	}

	server.nServerSocket = Create(AF_INET, SOCK_STREAM, 0, serverConfig.nServerPort, serverConfig.pServerIp);
	if (server.nServerSocket == -1)
	{
		ErrorInfor("StartServer", ERROR_CRESOCKET);
		return 0;
	}

	if (Listen(server.nServerSocket, serverConfig.nServerListenNumber) != 1)
	{
		Close(server.nServerSocket);
		ErrorInfor("StartServer", ERROR_CRELISTEN);
		return 0;
	}

	if (Accept(server.nServerSocket) == 0)
	{
		Close(server.nServerSocket);
		ErrorInfor("StartServer", ERROR_CREACCEPT);
		return 0;
	}

	/*创建数据处理*/
	if (InitData() == 0)
	{
		ReleaseAio(&server.aioX);
		Close(server.nServerSocket);
		ErrorInfor("StartServer", ERROR_INITDATA);
		return 0;
	}

	/*创建异步IO*/
	if (CreateAio(&server.aioX, serverConfig.nMaxAioQueueLength,
			serverConfig.nAioLoopSpace, serverConfig.nMaxAioBufferLength) == 0)
	{
		Close(server.nServerSocket);
		ErrorInfor("StartServer", ERROR_CREAIOX);
		return 0;
	}

	server.pShareMemory->proId = getpid();
	server.pShareMemory->nRun = 1;
	if (signal(SIGTERM, StopServerSignal) == (void *)-1)//激活终止信号
	{
		ErrorInfor("StartServer", ERROR_CRESIGNAL);
		StopServer();
		return 0;
	}

	return 1;
}

int StopServer()
{
	DebugInfor("1-1");
	if (ReleaseAio(&server.aioX) == 0)
	{
		ErrorInfor("StopServer", ERROR_RELAIOX);
	}

	DebugInfor("1-2");
	if (ReleaseData() == 0)
	{
		ErrorInfor("StopServer", ERROR_RELEDATA);
	}

	DebugInfor("1-3");
	if (Close(server.nServerSocket) != 1)
	{
		ErrorInfor("StopServer", ERROR_STOP);
	}

	DebugInfor("1-4");
	if (!ReleaseServerConfig())
	{
		ErrorInfor("StopServer", ERROR_RELINI);
	}

	DebugInfor("1-5");
	sleep(1);
	if (!ReleaseLog())
	{
		printf("%s-释放日志失败!\n", "StopServer");
	}

	return 1;
}

int RestartServer()
{
	return (StopServer() && StartServer());
}

int CreateShareMemory()
{
	key_t key = KEY;
	size_t size = sizeof(Server);
	int nFlag = 0644 | IPC_CREAT;

	server.nShareMemoryId = shmget(key, size, nFlag);
	if (server.nShareMemoryId == -1)
	{
		return 0;
	}

	if ((server.pShareMemory = (ShareMemory *)shmat(server.nShareMemoryId, NULL, 0)) == (void *)-1)
	{
		return 0;
	}

	return 1;
}

int ReleaseShareMemory()
{
	if (shmdt(server.pShareMemory) != 0)
	{
		return 0;
	}

	if (shmctl(server.nShareMemoryId, IPC_RMID, 0) != 0)
	{
		return 0;
	}

	return 1;
}

void StopServerSignal(int nSignalType)
{
	StopServer();
	exit(0);
}

int AddSocketEvent(int nClientSocket, SocketNode *pSocketNode)
{
	if (AdditionEvent(server.aioX.nAioId, nClientSocket, EVFILT_READ, pSocketNode) == 0)
	{
		ErrorInfor("AddSocketEvent", ERROR_ADDEVENT);
		return 0;
	}
	return 1;
}

int RecvData(SocketNode *pSocketNode, void *pData, int nDataSize)
{
	if (!pSocketNode)
	{
		ErrorInfor("RecvData", ERROR_ARGNULL);
		return 0;
	}

	pSocketNode->tmAccDateTime = time(NULL);
	InsertRecvDataNode(pSocketNode->nClientSocket, pData, nDataSize);
	return 1;
}
