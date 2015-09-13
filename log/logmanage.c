/*
 * logmanage.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "logmanage.h"
#include "log.h"

void DebugInfor(char *pDebug)
{
#ifdef DEBUG_SERVER
	if (pDebug)
	{
		int nSize = strlen("Debug: !\n") + strlen(pDebug) + 1;
		char *pLog = (char *)malloc(nSize);
		if (pLog)
		{
			sprintf(pLog, "Debug: %s!\n", pDebug);
			pLog[nSize - 1] = '\0';
			WriteLog(pLog, 1);
		}
	}
#endif
}

void LogInfor(char *pLog)
{
	if (pLog)
	{
		int nSize = strlen("Log: !\n") + strlen(pLog) + 1;
		char *pLogs = (char *)malloc(nSize);
		if (pLogs)
		{
			sprintf(pLogs, "Log: %s!\n", pLog);
			pLogs[nSize - 1] = '\0';
			WriteLog(pLogs, 2);
		}
	}
}

void ErrorInfor(char *pFunctionName, char *pError)
{
	if (pFunctionName && pError)
	{
		int nSize = strlen("Error: -!\n") + strlen(pFunctionName) + strlen(pError) + 1;
		char *pLog = (char *)malloc(nSize);
		if (pLog)
		{
			sprintf(pLog, "Error: %s-%s!\n", pFunctionName, pError);
			pLog[nSize - 1] = '\0';
			WriteLog(pLog, 3);
		}
	}
}

void SystemErrorInfor(char *pFunctionName, int nErrorNo)
{
	if (pFunctionName)
	{
		char *pError = strerror(nErrorNo);
		int nSize = strlen("SystemError: -!\n") + strlen(pFunctionName) + strlen(pError) + 1;
		char *pLog = (char *)malloc(nSize);
		if (pLog)
		{
			sprintf(pLog, "SystemError: %s-%s!\n", pFunctionName, pError);
			pLog[nSize - 1] = '\0';
			WriteLog(pLog, 4);
		}
	}
}
