/*
 * aiox.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "aiox.h"
#include "../server/servermanage.h"

int CreateAio(AioX *pAio, int nMaxAioQueueLength, int nLoopSpace, int nMaxBufferLength)
{
	if (!pAio || nMaxAioQueueLength < 1 || nMaxAioQueueLength < 1)
	{
		ErrorInfor("CreateAio", ERROR_ARGNULL);
		return 0;
	}

	pAio->nAioId = kqueue();
	if (pAio->nAioId == -1)
	{
		SystemErrorInfor("CreateAio", errno);
		return 0;
	}

	pAio->nMaxBufferLength   = nMaxBufferLength;
	pAio->nMaxAioQueueLength = nMaxAioQueueLength;
	pAio->pEvnetQueue 	     = malloc(nMaxAioQueueLength * sizeof(struct kevent));
	if (!pAio->pEvnetQueue)
	{
		SystemErrorInfor("CreateAio", errno);
		close(pAio->nAioId);
		return 0;
	}

	pAio->pProAioThread = CreateLoopThread(ProcessAio, pAio, nLoopSpace);
	if (!pAio->pProAioThread)
	{
		free(pAio->pEvnetQueue);
		close(pAio->nAioId);
		ErrorInfor("CreateAio", ERROR_CRETHREAD);
		return 0;
	}

	return 1;
}

int ControlAio(int nQueueId, struct kevent *event)
{
	if (kevent(nQueueId, event, 1, NULL, 0, NULL) == -1)
	{
		SystemErrorInfor("ControlAio", errno);
		return 0;
	}
	return 1;
}

int RemoveEvent(int nQueueId, int nFd, int nFilter)
{
	struct kevent event = GetEvent(nFd, nFilter, EV_DELETE, NULL);
	return ControlAio(nQueueId, &event);
}

int AdditionEvent(int nQueueId, int nFd, int nFilter, void *pData)
{
	struct kevent event = GetEvent(nFd, nFilter, EV_ADD, pData);
	return ControlAio(nQueueId, &event);
}

int ModifyEvent(int nQueueId, int nFd, int nFilter, void *pData)
{
	return AdditionEvent(nQueueId, nFd, nFilter, pData);
}

struct kevent GetEvent(int fd, int nFilter, int nFlags, void *pData)
{
	struct kevent event;
	EV_SET(&event, fd, nFilter, nFlags, 0, 0, pData);
	return event;
}

int ReleaseAio(AioX *pAio)
{
	if (!pAio)
	{
		ErrorInfor("ReleaseAio", ERROR_ARGNULL);
		return 0;
	}

	if (pAio->pProAioThread)
	{
		ReleaseThread(pAio->pProAioThread);
	}

	if (pAio->pEvnetQueue)
	{
		free(pAio->pEvnetQueue);
		pAio->pEvnetQueue = NULL;
	}

	if (close(pAio->nAioId) != 0)
	{
		SystemErrorInfor("ReleaseAio", errno);
		return 0;
	}
	return 1;
}

void *ProcessAio(void *pData)
{
	AioX *pAioX = (AioX *)pData;
	if (!pAioX || !pAioX->pEvnetQueue || pAioX->nMaxAioQueueLength < 1)
	{
		ErrorInfor("ProcessAio", ERROR_ARGNULL);
		return NULL;
	}

	/*五秒超时*/
	struct timespec time = {.tv_sec = 5, .tv_nsec = 0};
	int nQueLen = kevent(pAioX->nAioId, NULL, 0, pAioX->pEvnetQueue, pAioX->nMaxAioQueueLength, &time);
	if (nQueLen == -1)
	{
		SystemErrorInfor("ProcessAio", errno);
	}

	struct kevent event;
	for (int i = 0; i < nQueLen; i++)
	{
		event = pAioX->pEvnetQueue[i];

		//读取数据
		if (event.flags & EVFILT_READ)
		{
			Read(pAioX->nAioId, pAioX->nMaxBufferLength, &event);
		}
		//写数据
		else if (event.flags & EVFILT_WRITE)
		{
			Write(&event);
		}
		//错误数据
		else if (event.flags & EV_ERROR)
		{
			ErrorInfor("ProcessAio", (char *)event.data);

			if (RemoveEvent(pAioX->nAioId, event.ident, EV_ERROR) == 0)
			{
				ErrorInfor("ProcessAio", ERROR_REMEVENT);
			}
		}
		else
		{
		}
	}

	return NULL;
}

int Read(int nAioId, int nMaxBufferLength, struct kevent *event)
{
	char *pData = (char *)malloc(nMaxBufferLength);
	if (!pData)
	{
		SystemErrorInfor("Read", errno);
		return 0;
	}

	/*接收信息*/
	int nDataSize = recv(event->ident, pData, nMaxBufferLength, 0);

	//读取信息错误 或对方关闭了套接字
	if (nDataSize == -1 || nDataSize == 0)
	{
		SystemErrorInfor("Read-1", errno);

		if (RemoveEvent(nAioId, event->ident, EVFILT_READ) == 0)
		{
			ErrorInfor("Read", ERROR_REMEVENT);
		}
		return 0;
	}

	//插入数据
	RecvData(event->udata, pData, nDataSize);

	return 1;
}

int Write(struct kevent *event)
{
	return 1;
}
