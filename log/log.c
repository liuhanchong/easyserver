/*
 * log.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "log.h"

int InitLog()
{
	sysLog.nMaxLogListLength = serverConfig.nMaxLogListLength;
	sysLog.nProLogLoopSpace = serverConfig.nProLogLoopSpace;

	if (InitQueue(&sysLog.logList, sysLog.nMaxLogListLength, 0) == 0)
	{
		ErrorInfor("InitLog", ERROR_INITQUEUE);
		return 0;
	}

	if (CreateThreadPool(&sysLog.logThreadPool, &sysLog.logList.nCurQueueLen) == 0)
	{
		ReleaseQueue(&sysLog.logList);
		ErrorInfor("InitLog", ERROR_CREPOOL);
		return 0;
	}

	sysLog.pProLogThread = CreateLoopThread(ProcessLog, NULL, sysLog.nProLogLoopSpace);
	if (!sysLog.pProLogThread)
	{
		ReleaseThreadPool(&sysLog.logThreadPool);
		ReleaseQueue(&sysLog.logList);
		ErrorInfor("InitLog", ERROR_CRETHREAD);
		return 0;
	}

	//开启日志文件
	char chPath[PATH_MAX];
	sysLog.pLevelName[0] = "debug";
	sysLog.pLevelName[1] = "log";
	sysLog.pLevelName[2] = "error";
	sysLog.pLevelName[3] = "syserror";
	for (int i = 0; i < LOG_LEVEL; i++)
	{
		sprintf(chPath, "%s%s.log", "../file/log/", sysLog.pLevelName[i]);
		sysLog.pFileArray[i] = fopen(chPath, "a+");
		if (!sysLog.pFileArray[i])
		{
			SystemErrorInfor("InitLog", errno);
		}
	}

	return 1;
}

int ReleaseLog()
{
	if (sysLog.pProLogThread)
	{
		ReleaseThread(sysLog.pProLogThread);
	}

	if (ReleaseThreadPool(&sysLog.logThreadPool) == 0)
	{
		printf("%s-释放线程池失败", "ReleaseLog");
	}

	BeginTraveData(&sysLog.logList);
		ReleaseLogNode((LogNode *)pData);
	EndTraveData();

	if (ReleaseQueue(&sysLog.logList) == 0)
	{
		printf("%s-释放队列失败", "ReleaseLog");
	}

	//关闭文件
	for (int i = 0; i < LOG_LEVEL; i++)
	{
		if (sysLog.pFileArray[i])
		{
			fclose(sysLog.pFileArray[i]);
		}
	}

	return 1;
}

int WriteLog(char *pText, int nType)
{
	if (!pText || nType < 0)
	{
		ErrorInfor("WriteLog", ERROR_ARGNULL);
		return 0;
	}

	LogNode *pLogNode = (LogNode *)malloc(sizeof(LogNode));
	if (!pLogNode)
	{
		SystemErrorInfor("WriteLog-1", errno);
		return 0;
	}

	pLogNode->nType = nType;
	pLogNode->nDataSize = strlen(pText);
	pLogNode->pLog = (char *)malloc(pLogNode->nDataSize);
	if (!pLogNode->pLog)
	{
		free(pLogNode);
		pLogNode = NULL;
		SystemErrorInfor("WriteLog-2", errno);
		return 0;
	}

	memcpy(pLogNode->pLog, pText, pLogNode->nDataSize);

	LockQueue(&sysLog.logList);
	InsertLog(&sysLog.logList, pLogNode, 0);
	UnlockQueue(&sysLog.logList);

	return 1;
}

int ReleaseLogNode(LogNode *pLogNode)
{
	if (pLogNode)
	{
		if (pLogNode->pLog)
		{
			free(pLogNode->pLog);
			pLogNode->pLog = NULL;
		}

		free(pLogNode);
		pLogNode = NULL;
	}

	return 1;
}

void *ProcessLog(void *pData)
{
	LockQueue(&sysLog.logList);

	QueueNode *pQueueNode = (QueueNode *)GetNodeForIndex(&sysLog.logList, 0);
	if (pQueueNode)
	{
		/*此处分配的node内存空间需要执行的线程函数进行销毁*/
		if (ExecuteTask(&sysLog.logThreadPool, WriteFile, pQueueNode->pData) == 1)
		{
			DeleteForNode(&sysLog.logList, pQueueNode);
		}
	}

	UnlockQueue(&sysLog.logList);

	return NULL;
}

void *WriteFile(void *pData)
{
	LogNode *pLogNode = (LogNode *)pData;
	if (!pLogNode)
	{
		ErrorInfor("WriteFile", ERROR_ARGNULL);
		return NULL;
	}

	FILE *pFile = GetFile(pLogNode->nType);
	if (!pFile)
	{
		ErrorInfor("WriteFile", ERROR_GETFILE);
		return NULL;
	}

	if (pLogNode->pLog)
	{
		fwrite(pLogNode->pLog, pLogNode->nDataSize, 1, pFile);
	}

	ReleaseLogNode(pLogNode);

	return NULL;
}

FILE *GetFile(int nType)
{
	if (nType > 0 && nType < (LOG_LEVEL + 1))
	{
		return sysLog.pFileArray[nType - 1];
	}

	return sysLog.pFileArray[0];
}
