/*
 * aiox.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef COMMUNICATION_AIOX_H_
#define COMMUNICATION_AIOX_H_

#include <sys/types.h>
#include <sys/event.h>
#include <sys/socket.h>
#include "../log/logmanage.h"
#include "../thread/thread.h"
#include "../basic/queue.h"

typedef struct AioX
{
	struct kevent *pEvnetQueue;
	int nMaxAioQueueLength;
	int nAioId;
	Thread *pProAioThread;
	int nMaxBufferLength;
} AioX;

/*接口*/
int CreateAio(AioX *pAio, int nMaxAioQueueLength, int nLoopSpace, int nMaxBufferLength);
int ControlAio(int nQueueId, struct kevent *event);
int RemoveEvent(int nQueueId, int nFd, int nFilter);
int AdditionEvent(int nQueueId, int nFd, int nFilter, void *pData);
int ModifyEvent(int nQueueId, int nFd, int nFilter, void *pData);
struct kevent GetEvent(int fd, int nFilter, int nFlags, void *pData);
int ReleaseAio(AioX *pAio);

/*私有*/
void *ProcessAio(void *pData);
int Read(int nAioId, int nMaxBufferLength, struct kevent *event);
int Write(struct kevent *event);

#endif /* COMMUNICATION_AIOX_H_ */
