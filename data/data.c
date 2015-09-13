/*
 * data.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "data.h"

int InitData()
{
	data.nMaxRecvListLen = serverConfig.nMaxDataRecvListLen;
	data.nMaxSendListLen = serverConfig.nMaxDataSendListLen;
	data.nProRecvDataLoopSpace = serverConfig.nProRecvDataLoopSpace;
	data.nProSendDataLoopSpace = serverConfig.nProSendDataLoopSpace;

	if (InitQueue(&data.recvDataList, data.nMaxRecvListLen, 0) == 0)
	{
		ErrorInfor("InitData-1", ERROR_INITQUEUE);
		return 0;
	}

	if (InitQueue(&data.sendDataList, data.nMaxSendListLen, 0) == 0)
	{
		ReleaseQueue(&data.recvDataList);
		ErrorInfor("InitData-2", ERROR_INITQUEUE);
		return 0;
	}

	if (CreateThreadPool(&data.recvThreadPool, &data.recvDataList.nCurQueueLen) == 0)
	{
		ReleaseQueue(&data.recvDataList);
		ReleaseQueue(&data.sendDataList);
		ErrorInfor("InitData-1", ERROR_CREPOOL);
		return 0;
	}

	if (CreateThreadPool(&data.sendThreadPool, &data.sendDataList.nCurQueueLen) == 0)
	{
		ReleaseThreadPool(&data.recvThreadPool);
		ReleaseQueue(&data.recvDataList);
		ReleaseQueue(&data.sendDataList);
		ErrorInfor("InitData-2", ERROR_CREPOOL);
		return 0;
	}

	if (CreateDBConnPool(&data.dbConnPool) == 0)
	{
		ReleaseThreadPool(&data.sendThreadPool);
		ReleaseThreadPool(&data.recvThreadPool);
		ReleaseQueue(&data.recvDataList);
		ReleaseQueue(&data.sendDataList);
		ErrorInfor("InitData-3", ERROR_CREPOOL);
		return 0;
	}

	data.pProRecvThread = CreateLoopThread(ProcessRecvData, NULL, data.nProRecvDataLoopSpace);
	if (!data.pProRecvThread)
	{
		ReleaseThreadPool(&data.sendThreadPool);
		ReleaseThreadPool(&data.recvThreadPool);
		ReleaseDBConnPool(&data.dbConnPool);
		ReleaseQueue(&data.recvDataList);
		ReleaseQueue(&data.sendDataList);
		ErrorInfor("InitData-1", ERROR_CRETHREAD);
		return 0;
	}

	data.pProSendThread = CreateLoopThread(ProcessSendData, NULL, data.nProSendDataLoopSpace);
	if (!data.pProSendThread)
	{
		ReleaseThread(data.pProRecvThread);
		ReleaseThreadPool(&data.sendThreadPool);
		ReleaseThreadPool(&data.recvThreadPool);
		ReleaseDBConnPool(&data.dbConnPool);
		ReleaseQueue(&data.recvDataList);
		ReleaseQueue(&data.sendDataList);
		ErrorInfor("InitData-2", ERROR_CRETHREAD);
		return 0;
	}

	return 1;
}

int ReleaseData()
{
	DebugInfor("2-1");
	if (data.pProRecvThread)
	{
		ReleaseThread(data.pProRecvThread);
	}

	DebugInfor("2-2");
	if (data.pProSendThread)
	{
		ReleaseThread(data.pProSendThread);
	}

	DebugInfor("2-3");
	if (ReleaseThreadPool(&data.recvThreadPool) == 0)
	{
		ErrorInfor("ReleaseData-1", ERROR_RELPOOL);
	}

	DebugInfor("2-4");
	if (ReleaseThreadPool(&data.sendThreadPool) == 0)
	{
		ErrorInfor("ReleaseData-2", ERROR_RELPOOL);
	}

	DebugInfor("2-5");
	if (ReleaseDBConnPool(&data.dbConnPool) == 0)
	{
		ErrorInfor("ReleaseData-3", ERROR_RELPOOL);
	}

	DebugInfor("2-6");
	BeginTraveData(&data.recvDataList);
		ReleaseDataNode((DataNode *)pData);
	EndTraveData();

	DebugInfor("2-7");
	printf("2-7-%d\n", GetCurQueueLen(&data.recvDataList));
	if (ReleaseQueue(&data.recvDataList) == 0)
	{
		ErrorInfor("ReleaseData-1", ERROR_RELQUEUE);
	}

	printf("2-8\n");
	DebugInfor("2-8");
	BeginTraveData(&data.sendDataList);
		ReleaseDataNode((DataNode *)pData);
	EndTraveData();

	DebugInfor("2-9");
	if (ReleaseQueue(&data.sendDataList) == 0)
	{
		ErrorInfor("ReleaseData-2", ERROR_RELQUEUE);
	}

	return 1;
}

void ReleaseDataNode(DataNode *pNode)
{
	if (pNode)
	{
		if (pNode->pData)
		{
			free(pNode->pData);
			pNode->pData = NULL;
		}

		free(pNode);
		pNode = NULL;
	}
}

int InsertRecvDataNode(int nSocket, void *pData, int nDataSize)
{
	return InsertDataNode(nSocket, pData, nDataSize, 1);
}

int InsertSendDataNode(int nSocket, void *pData, int nDataSize)
{
	return InsertDataNode(nSocket, pData, nDataSize, 2);
}

void *ProcessRecvData(void *pData)
{
	LockQueue(&data.recvDataList);

	QueueNode *pQueueNode = (QueueNode *)GetNodeForIndex(&data.recvDataList, 0);
	if (pQueueNode)
	{
		/*此处分配的datanode内存空间需要执行的线程函数进行销毁*/
		if (ExecuteTask(&data.recvThreadPool, TestData, pQueueNode->pData) == 1)
		{
			DeleteForNode(&data.recvDataList, pQueueNode);
		}
	}

	UnlockQueue(&data.recvDataList);

	return NULL;
}

void *ProcessSendData(void *pData)
{
	LockQueue(&data.sendDataList);

	QueueNode *pQueueNode = (QueueNode *)GetNodeForIndex(&data.sendDataList, 0);
	if (pQueueNode)
	{
		/*此处分配的datanode内存空间需要执行的线程函数进行销毁*/
		if (ExecuteTask(&data.sendThreadPool, TestData, pQueueNode->pData) == 1)
		{
			DeleteForNode(&data.sendDataList, pQueueNode);
		}
	}

	UnlockQueue(&data.sendDataList);

	return NULL;
}

int InsertDataNode(int nSocket, void *pData, int nDataSize, int nType)
{
	DataNode *pDataNode = (DataNode *)malloc(sizeof(DataNode));
	if (!pDataNode)
	{
		SystemErrorInfor("InsertDataNode", errno);
		return 0;
	}

	pDataNode->pData = pData;
	pDataNode->nSocket = nSocket;
	pDataNode->nDataSize = nDataSize;

	/*插入数据处理队列*/
	int nReturn = 0;
	if (nType == 1)
	{
		LockQueue(&data.recvDataList);
		nReturn = Insert(&data.recvDataList, pDataNode, 0);
		UnlockQueue(&data.recvDataList);
	}
	else
	{
		LockQueue(&data.sendDataList);
		nReturn = Insert(&data.sendDataList, pDataNode, 0);
		UnlockQueue(&data.sendDataList);
	}

	return nReturn;
}

void *TestData(void *pData)
{
	DataNode *pDataNode = (DataNode *)pData;
	if (pDataNode)
	{
		if(pDataNode->pData)
		{
//			char *pData = (char *)pDataNode->pData;
//			pData[pDataNode->nDataSize - 1] = '\0';
//			printf("INFOR-socket:%d data:%s\n", pDataNode->nSocket, pData);
			DBConnNode *pDBConnNode = GetFreeDBConn(&data.dbConnPool);
			if (pDBConnNode)
			{
				if (ExecuteModify(pDBConnNode->pMySql, "insert into test.message(id, message) values(1, '123')") == 0)
				{
//					printf("1\n");
					ReleaseAccessDBConn(pDBConnNode);
					exit(0);
				}
				else
				{
//					printf("2\n");
				}
				ReleaseAccessDBConn(pDBConnNode);
			}
			else
			{
				DebugInfor("没有空闲的连接");
			}
		}

		ReleaseDataNode(pDataNode);
		pDataNode = NULL;
	}

	return NULL;
}
