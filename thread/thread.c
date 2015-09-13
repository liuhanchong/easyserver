/*
 * thread.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "thread.h"

Thread *CreateSingleThread(void *(*Fun)(void *), void *pData)
{
	Thread *pThread = CreateThread(Fun, pData, 2, 0);
	if (!pThread)
	{
		ErrorInfor("CreateSingleThread", ERROR_CRETHREAD);
		return NULL;
	}

	return pThread;
}

Thread *CreateLoopThread(void *(*Fun)(void *), void *pData, int nLoopSecond)
{
	Thread *pThread = CreateThread(Fun, pData, 1, nLoopSecond);
	if (!pThread)
	{
		ErrorInfor("CreateLoopThread", ERROR_CRETHREAD);
		return NULL;
	}

	return pThread;
}

int ReleaseThread(Thread *pThread)
{
	if (!pThread)
	{
		ErrorInfor("ReleaseThread", ERROR_ARGNULL);
		return 0;
	}

	if (pthread_cancel(pThread->thId) == 0 &&
		pthread_join(pThread->thId, NULL) == 0 &&
		pthread_attr_destroy(&pThread->thAttribute) == 0 &&
		pthread_cond_destroy(&pThread->thCondition) == 0 &&
		pthread_mutex_destroy(&pThread->thMutex) == 0)
	{
		free(pThread);
		pThread = NULL;
		return 1;
	}

	SystemErrorInfor("ReleaseThread", errno);

	free(pThread);
	pThread = NULL;

	return 0;
}

int PauseThread(Thread *pThread)
{
	if (!pThread)
	{
		ErrorInfor("PauseThread", ERROR_ARGNULL);
		return 0;
	}

	pthread_mutex_lock(&pThread->thMutex);
	pThread->nExecute = 0;
	pthread_mutex_unlock(&pThread->thMutex);
	return 1;
}

int ResumeThread(Thread *pThread)
{
	if (!pThread)
	{
		ErrorInfor("ResumeThread", ERROR_ARGNULL);
		return 0;
	}

	pthread_mutex_lock(&pThread->thMutex);
	pThread->nExecute = 1;
	pthread_cond_signal(&pThread->thCondition);
	pthread_mutex_unlock(&pThread->thMutex);
	return 1;
}

int IsResume(Thread *pThread)
{
	if (!pThread)
	{
		ErrorInfor("IsResume", ERROR_ARGNULL);
		return 0;
	}

	return pThread->nExecute;
}

void SetThreadDetach(Thread *pThread, int nDetach)
{
	if (!pThread)
	{
		ErrorInfor("SetThreadDetach", ERROR_ARGNULL);
		return;
	}

  	int nFlag =  (nDetach == 1) ? PTHREAD_CREATE_DETACHED : PTHREAD_CREATE_JOINABLE;
	if (pthread_attr_setdetachstate(&pThread->thAttribute, nFlag) != 0)
	{
		SystemErrorInfor("SetThreadDetach", errno);
	}
}

void SetThreadExecute(Thread *pThread, void *(*Fun)(void *), void *pData)
{
	if (pThread)
	{
		pThread->Fun = Fun;
		pThread->pData = pData;
	}
}

void *DefaultExecuteMode(void *pData)
{
	pthread_testcancel();
	Thread *pThread = (Thread *)pData;
	if (!pThread)
	{
		ErrorInfor("DefaultExecuteMode", ERROR_TRANTYPE);
		return NULL;
	}

	pthread_testcancel();
	if (SetCancelMode(pThread->nCancelMode) == 0)
	{
		ErrorInfor("DefaultExecuteMode", ERROR_SETMODE);
		return NULL;
	}

	pthread_testcancel();
	if (!pThread->Fun)
	{
		ErrorInfor("DefaultExecuteMode", ERROR_PROFUNNULL);
		return NULL;
	}

	if (pThread->nExecuteMode == 1)
	{
		pthread_testcancel();

		BEGINTHREAD();
		pThread->Fun(pThread->pData);
		ENDTHREAD(pThread->nLoopSecond);
	}
	else if (pThread->nExecuteMode == 2)
	{
		pthread_testcancel();

		RESUMETHREAD(pThread);
		pThread->Fun(pThread->pData);
		SUSPENDTHREAD(pThread);
	}

	return NULL;
}

int SetCancelMode(int nMode)
{
	if (nMode)
	{
		if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0 &&
			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL) == 0)
		{
			return 1;
		}

		SystemErrorInfor("SetCancelMode-1", errno);
	}
	else
	{
		if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) == 0)
		{
			return 1;
		}

		SystemErrorInfor("SetCancelMode-2", errno);
	}
	return 0;
}

Thread *CreateThread(void *(*Fun)(void *), void *pData, int nExecuteMode, int nLoopSecond)
{
	if (!Fun)
	{
		ErrorInfor("CreateThread", ERROR_ARGNULL);
		return NULL;
	}

	Thread *pThread = (Thread *)malloc(sizeof(Thread));
	if (!pThread)
	{
		SystemErrorInfor("CreateThread-1", errno);
		return NULL;
	}

	memset(pThread, 0, sizeof(Thread));
	if (pthread_mutex_init(&pThread->thMutex, NULL) == 0 &&
		pthread_cond_init(&pThread->thCondition, NULL) == 0 &&
		pthread_attr_init(&pThread->thAttribute) == 0)
	{
		pThread->nExecute     = 0;
		pThread->Fun 		  = Fun;
		pThread->pData 		  = pData;
		pThread->nCancelMode  = 1;
		pThread->nExecuteMode = nExecuteMode;
		pThread->nLoopSecond  = nLoopSecond;
		if (pthread_create(&pThread->thId, &pThread->thAttribute, DefaultExecuteMode, (void *)pThread) == 0)
		{
			return pThread;
		}

		SystemErrorInfor("CreateThread-2", errno);
	}
	else
	{
		SystemErrorInfor("CreateThread-3", errno);
	}

	free(pThread);
	pThread = NULL;

	return NULL;
}

void ReleaseResource(void *pData)
{
	pthread_mutex_t *pMutex = (pthread_mutex_t *)pData;
	if (!pMutex)
	{
		ErrorInfor("ReleaseResource", ERROR_TRANTYPE);
		return;
	}

	if (pthread_mutex_unlock(pMutex) != 0)
	{
		SystemErrorInfor("ReleaseResource", errno);
	}
}

