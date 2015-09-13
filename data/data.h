/*
 * data.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef DATA_DATA_H_
#define DATA_DATA_H_

#include "../thread/threadpool.h"
#include "../db/dbconnpool.h"

typedef struct DataNode
{
	int nSocket;
	void *pData;
	int nDataSize;
} DataNode;

typedef struct Data
{
	List recvDataList;
	List sendDataList;

	ThreadPool recvThreadPool;
	ThreadPool sendThreadPool;

	DBConnPool dbConnPool;

	int nMaxRecvListLen;
	int nMaxSendListLen;

	Thread *pProRecvThread;
	int nProRecvDataLoopSpace;

	Thread *pProSendThread;
	int nProSendDataLoopSpace;
} Data;

static Data data;

/*接口*/
int InitData();
int ReleaseData();
void ReleaseDataNode(DataNode *pNode);
int InsertRecvDataNode(int nSocket, void *pData, int nDataSize);
int InsertSendDataNode(int nSocket, void *pData, int nDataSize);

/*私有*/
void *ProcessRecvData(void *pData);
void *ProcessSendData(void *pData);
int InsertDataNode(int nSocket, void *pData, int nDataSize, int nType);

/*测试*/
void *TestData(void *pData);

#endif /* DATA_DATA_H_ */
