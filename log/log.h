/*
 * log.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef LOG_LOG_H_
#define LOG_LOG_H_

#include <sys/syslimits.h>
#include <stdlib.h>
#include "logmanage.h"
#include "../basic/queue.h"
#include "../config/configmanage.h"
#include "../thread/threadpool.h"


/*错误级别
 * 1 debug		  调试信息
 * 2 log 		  输出的日志
 * 3 error 		  错误信息
 * 4 system error 系统信息
 * */

#define LOG_LEVEL 4

typedef struct LogNode
{
	char *pLog;
	int nType;
	int nDataSize;
} LogNode;

typedef struct Log
{
	List logList;
	ThreadPool logThreadPool;
	int nMaxLogListLength;

	Thread *pProLogThread;
	int nProLogLoopSpace;

	FILE *pFileArray[LOG_LEVEL];
	char *pLevelName[LOG_LEVEL];
} Log;

static Log sysLog;

int InitLog();
int ReleaseLog();
int WriteLog(char *pText, int nType);
int ReleaseLogNode(LogNode *pLogNode);

void *ProcessLog(void *pData);
void *WriteFile(void *pData);
FILE *GetFile(int nType);

#endif /* LOG_LOG_H_ */
