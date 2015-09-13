/*
 * threadpool.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "threadpool.h"

int CreateThreadPool(ThreadPool *pThreadQueue, int *pTaskQueueLength)
{
	if (!pThreadQueue)
	{
		ErrorInfor("CreateThreadPool", ERROR_ARGNULL);
		return 0;
	}

	pThreadQueue->nMaxThreadNumber = serverConfig.nMaxThreadNumber;
	pThreadQueue->nCoreThreadNumber = serverConfig.nCoreThreadNumber;

	pThreadQueue->nAccOverTime = serverConfig.nAccThreadOverTime;
	pThreadQueue->nAccThreadLoopSpace = serverConfig.nAccThreadLoopSpace;

	pThreadQueue->nAddThreadNumber = serverConfig.nAddThreadNumber;
	pThreadQueue->nAddThreadLoopSpace = serverConfig.nAddThreadLoopSpace;

	pThreadQueue->nExeThreadOverTime = serverConfig.nExeThreadOverTime;
	pThreadQueue->nExeThreadLoopSpace = serverConfig.nExeThreadLoopSpace;

	pThreadQueue->pTaskQueueLength = pTaskQueueLength;
	pThreadQueue->nAddThreadTaskNumber = serverConfig.nAddThreadTaskNumber;

	if (InitQueue(&pThreadQueue->threadList, pThreadQueue->nMaxThreadNumber, 0) == 0)
	{
		ErrorInfor("CreateThreadPool", ERROR_INITQUEUE);
		return 0;
	}

	pThreadQueue->pDynAddThread = CreateLoopThread(AddThreadDynamic, pThreadQueue, pThreadQueue->nAddThreadLoopSpace);
	if (!pThreadQueue->pDynAddThread)
	{
		ReleaseQueue(&pThreadQueue->threadList);
		ErrorInfor("CreateThreadPool-1", ERROR_CRETHREAD);
		return 0;
	}

	pThreadQueue->pFreeOvertimeThread = CreateLoopThread(FreeThreadAccess, pThreadQueue, pThreadQueue->nAccThreadLoopSpace);
	if (!pThreadQueue->pFreeOvertimeThread)
	{
		ReleaseThread(pThreadQueue->pDynAddThread);
		ReleaseQueue(&pThreadQueue->threadList);
		ErrorInfor("CreateThreadPool-2", ERROR_CRETHREAD);
		return 0;
	}

	pThreadQueue->pExecuteOvertimeThread = CreateLoopThread(FreeThreadExecute, pThreadQueue, pThreadQueue->nExeThreadLoopSpace);
	if (!pThreadQueue->pExecuteOvertimeThread)
	{
		ReleaseThread(pThreadQueue->pDynAddThread);
		ReleaseThread(pThreadQueue->pFreeOvertimeThread);
		ReleaseQueue(&pThreadQueue->threadList);
		ErrorInfor("CreateThreadPool-3", ERROR_CRETHREAD);
		return 0;
	}

	if (CreateMulThread(pThreadQueue, pThreadQueue->nCoreThreadNumber) == 0)
	{
		ReleaseThread(pThreadQueue->pDynAddThread);
		ReleaseThread(pThreadQueue->pFreeOvertimeThread);
		ReleaseThread(pThreadQueue->pExecuteOvertimeThread);
		ReleaseQueue(&pThreadQueue->threadList);
		ErrorInfor("CreateThreadPool", ERROR_CREPOOL);
		return 0;
	}

	return 1;
}

int ReleaseThreadPool(ThreadPool *pThreadQueue)
{
	if (!pThreadQueue)
	{
		ErrorInfor("ReleaseThreadPool", ERROR_ARGNULL);
		return 0;
	}

	if (pThreadQueue->pDynAddThread)
	{
		ReleaseThread(pThreadQueue->pDynAddThread);
	}

	if (pThreadQueue->pFreeOvertimeThread)
	{
		ReleaseThread(pThreadQueue->pFreeOvertimeThread);
	}

	if (pThreadQueue->pExecuteOvertimeThread)
	{
		ReleaseThread(pThreadQueue->pExecuteOvertimeThread);
	}

	/*遍历队列列表*/
	BeginTraveData(&pThreadQueue->threadList);
		ReleaseThreadNode((ThreadNode *)pData);
	EndTraveData();

	if (!ReleaseQueue(&pThreadQueue->threadList))
	{
		ErrorInfor("ReleaseThreadPool", ERROR_RELQUEUE);
	}

	return 1;
}

int GetFreeThreadNumber(ThreadPool *pThreadQueue)
{
	if (!pThreadQueue)
	{
		ErrorInfor("GetFreeThreadNumber", ERROR_ARGNULL);
		return 0;
	}

	int nCount = 0;
	BeginTraveData(&pThreadQueue->threadList);
		if (IsResume(((ThreadNode *)pData)->pThread) == 0)
		{
			nCount++;
		}
	EndTraveData();

	return nCount;
}

ThreadNode *GetFreeThread(ThreadPool *pThreadQueue)
{
	if (!pThreadQueue)
	{
		ErrorInfor("GetFreeThread", ERROR_ARGNULL);
		return 0;
	}

	ThreadNode *pThreadNode = NULL;
	BeginTraveData(&pThreadQueue->threadList);
		pThreadNode = (ThreadNode *)pData;
		if (IsResume(pThreadNode->pThread) == 0)
		{
			break;
		}
		pThreadNode = NULL;
	EndTraveData();

	return pThreadNode;
}

void ReleaseThreadNode(ThreadNode *pThreadNode)
{
	if (pThreadNode)
	{
		if (pThreadNode->pThread)
		{
			ReleaseThread(pThreadNode->pThread);
		}

		free(pThreadNode);
		pThreadNode = NULL;
	}
}

int ExecuteTask(ThreadPool *pThreadQueue, void *(*Fun)(void *), void *pData)
{
	if (!pThreadQueue)
	{
		ErrorInfor("ExecuteTask", ERROR_ARGNULL);
		return 0;
	}

	int nReturn = 0;
	LockQueue(&pThreadQueue->threadList);

	ThreadNode *pThreadNode = GetFreeThread(pThreadQueue);
	if (pThreadNode)
	{
		/*设置线程执行的函数和所需要的数据*/
		SetThreadExecute(pThreadNode->pThread, Fun, pData);

		if (ResumeThread(pThreadNode->pThread))
		{
			pThreadNode->tmAccessTime =  time(NULL);
			pThreadNode->tmExeTime = time(NULL);
			nReturn = 1;
		}
	}

	UnlockQueue(&pThreadQueue->threadList);

	return nReturn;
}

void SetTaskQueueLength(ThreadPool *pThreadQueue, int *pTaskQueueLength)
{
	pThreadQueue->pTaskQueueLength = pTaskQueueLength;
}

void *AddThreadDynamic(void *pData)
{
	ThreadPool *pThreadQueue = (ThreadPool *)pData;
	if (!pThreadQueue)
	{
		ErrorInfor("AddThreadDynamic", ERROR_TRANTYPE);
		return NULL;
	}

	LockQueue(&pThreadQueue->threadList);

	if (GetFreeThreadNumber(pThreadQueue) == 0)
	{
		if (pThreadQueue->pTaskQueueLength)
		{
			if (*pThreadQueue->pTaskQueueLength > pThreadQueue->nAddThreadTaskNumber)
			{
				CreateMulThread(pThreadQueue, pThreadQueue->nAddThreadNumber);
			}
		}
	}

	UnlockQueue(&pThreadQueue->threadList);

	return NULL;
}

void *FreeThreadAccess(void *pData)
{
	ThreadPool *pThreadQueue = (ThreadPool *)pData;
	if (!pThreadQueue)
	{
		ErrorInfor("FreeThreadAccess", ERROR_TRANTYPE);
		return NULL;
	}

	/*遍历队列列表*/
	LockQueue(&pThreadQueue->threadList);

	/*当前线程超过核心线程数删除*/
	if (GetCurQueueLen(&pThreadQueue->threadList) > pThreadQueue->nCoreThreadNumber)
	{
		BeginTraveData(&pThreadQueue->threadList);
			time_t tmCurTime = time(NULL);
			ThreadNode *pThreadNode = (ThreadNode *)pData;
			if (IsResume(pThreadNode->pThread) == 0)
			{
				if ((tmCurTime - pThreadNode->tmAccessTime) >= pThreadQueue->nAccOverTime)
				{
					ReleaseThreadNode(pThreadNode);
					DeleteForNode(&pThreadQueue->threadList, pQueueNode);
				}
			}
		EndTraveData();
	}

	UnlockQueue(&pThreadQueue->threadList);

	return NULL;
}

void *FreeThreadExecute(void *pData)
{
	ThreadPool *pThreadQueue = (ThreadPool *)pData;
	if (!pThreadQueue)
	{
		ErrorInfor("FreeThreadExecute", ERROR_TRANTYPE);
		return NULL;
	}

	/*遍历队列列表*/
	LockQueue(&pThreadQueue->threadList);

	BeginTraveData(&pThreadQueue->threadList);
		time_t tmCurTime = time(NULL);
		ThreadNode *pThreadNode = (ThreadNode *)pData;
		if (IsResume(pThreadNode->pThread) == 0)
		{
			if ((tmCurTime - pThreadNode->tmExeTime) >= pThreadQueue->nExeThreadOverTime)
			{
				ReleaseThreadNode(pThreadNode);
				DeleteForNode(&pThreadQueue->threadList, pQueueNode);
				InsertThread(pThreadQueue);
			}
		}
	EndTraveData();

	UnlockQueue(&pThreadQueue->threadList);

	return NULL;
}

int CreateMulThread(ThreadPool *pThreadQueue, int nNumber)
{
	if (!pThreadQueue)
	{
		ErrorInfor("CreateMulThread", ERROR_ARGNULL);
		return 0;
	}

	while ((nNumber--) > 0)
	{
		if (InsertThread(pThreadQueue) == 0)
		{
			ErrorInfor("CreateMulThread", ERROR_CRETHREAD);
		}
	}

	return 1;
}

int InsertThread(ThreadPool *pThreadQueue)
{
	if (!pThreadQueue)
	{
		ErrorInfor("InsertThread", ERROR_ARGNULL);
		return 0;
	}

	if (Full(&pThreadQueue->threadList))
	{
		ErrorInfor("InsertThread", ERROR_OUTQUEUE);
		return 0;
	}

	ThreadNode *pThreadNode = (ThreadNode *)malloc(sizeof(ThreadNode));
	if (!pThreadNode)
	{
		SystemErrorInfor("InsertThread", errno);
		return 0;
	}

	pThreadNode->pThread = CreateSingleThread(DefaultThreadFun, pThreadNode);
	if (!pThreadNode->pThread)
	{
		free(pThreadNode);
		pThreadNode = NULL;

		ErrorInfor("InsertThread", ERROR_CRETHREAD);
		return 0;
	}

	pThreadNode->tmAccessTime = time(NULL);
	pThreadNode->tmExeTime = time(NULL);

	if (!Insert(&pThreadQueue->threadList, (void *)pThreadNode, 0))
	{
		ReleaseThreadNode(pThreadNode);

		ErrorInfor("InsertThread", ERROR_INSEQUEUE);
		return 0;
	}

	return 1;
}

void *DefaultThreadFun(void *pData)
{
	return NULL;
}
