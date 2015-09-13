/*
 * thread.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef THREAD_THREAD_H_
#define THREAD_THREAD_H_

#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include <stdlib.h>
#include "../log/logmanage.h"

typedef struct Thread
{
	pthread_t thId;
	pthread_mutex_t thMutex;
	pthread_cond_t thCondition;
	int nExecute;/*线程是否正在执行*/
	void *pData;/*传递的执行函数的参数*/
	void *(*Fun)(void *);/*线程执行函数*/
	int nCancelMode;//取消线程的模式
	int nLoopSecond;//线程循环一次间隔的时间
	int nExecuteMode;//执行模式 循环执行 单任务执行
	pthread_attr_t thAttribute;//线程属性
} Thread;

#define RESUMETHREAD(pThread) \
		{ \
			while (1) \
			{ \
				/*当线程异常退出时候，需要执行的清理函数*/ \
				pthread_testcancel(); \
				pthread_cleanup_push(ReleaseResource, (&(pThread->thMutex))); \
				pthread_testcancel(); \
				\
				/*等待线程可以执行*/ \
				pthread_mutex_lock((&(pThread->thMutex))); \
				while (pThread->nExecute == 0) \
				{ \
					pthread_testcancel(); \
					pthread_cond_wait((&(pThread->thCondition)), (&(pThread->thMutex))); \
					pthread_testcancel(); \
				}

#define SUSPENDTHREAD(pThread) \
				/*解锁资源 并将线程挂起*/ \
		 		pthread_mutex_unlock((&(pThread->thMutex))); \
		 		pthread_testcancel(); \
		 		PauseThread(pThread); \
		 		pthread_cleanup_pop(0); \
			} \
		}

#define BEGINTHREAD()  \
		{ \
			while (1) \
			{ \
				pthread_testcancel();

#define ENDTHREAD(SEC)  \
				pthread_testcancel(); \
				if (SEC > 0) \
				{ \
					pthread_testcancel(); \
					sleep(SEC); \
					pthread_testcancel(); \
				} \
				pthread_testcancel(); \
			} \
		}

/*接口*/
Thread *CreateSingleThread(void *(*Fun)(void *), void *pData);
Thread *CreateLoopThread(void *(*Fun)(void *), void *pData, int nLoopSecond);
int ReleaseThread(Thread *pThread);
int PauseThread(Thread *pThread);
int ResumeThread(Thread *pThread);
int IsResume(Thread *pThread);
void SetThreadDetach(Thread *pThread, int nDetach);
void SetThreadExecute(Thread *pThread, void *(*Fun)(void *), void *pData);

/*私有*/
void *DefaultExecuteMode(void *pData);
int SetCancelMode(int nMode);
Thread *CreateThread(void *(*Fun)(void *), void *pData, int nExecuteMode, int nLoopSecond);
void ReleaseResource(void *pData);

#endif /* THREAD_THREAD_H_ */
