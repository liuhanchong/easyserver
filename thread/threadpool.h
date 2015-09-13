/*
 * threadpool.h
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#ifndef THREAD_THREADPOOL_H_
#define THREAD_THREADPOOL_H_

#include "thread.h"
#include "../basic/queue.h"
#include "../log/logmanage.h"
#include "../config/configmanage.h"

typedef struct ThreadNode
{
	Thread *pThread;
	time_t tmAccessTime;
	time_t tmExeTime;
} ThreadNode;

typedef struct ThreadPool
{
	List threadList;

	Thread *pDynAddThread;
	Thread *pFreeOvertimeThread;
	Thread *pExecuteOvertimeThread;

	int *pTaskQueueLength;
	int nAddThreadTaskNumber;/*当任务队列超过多少 自动增加线程*/

	int nMaxThreadNumber;
	int nCoreThreadNumber;

	int nAccOverTime;/*线程未使用时间超时*/
	int nAccThreadLoopSpace;/*线程未使用的判断间隔*/

	int nAddThreadNumber;/*增加线程时候增加的个数*/
	int nAddThreadLoopSpace;/*增加线程时候的判断间隔*/

	int nExeThreadOverTime;/*执行线程的时间超时*/
	int nExeThreadLoopSpace;/*执行线程的判断间隔*/
} ThreadPool;

/*接口*/
int CreateThreadPool(ThreadPool *pThreadQueue, int *pTaskQueueLength);
int ReleaseThreadPool(ThreadPool *pThreadQueue);
int GetFreeThreadNumber(ThreadPool *pThreadQueue);/*获取空闲线程个数*/
ThreadNode *GetFreeThread(ThreadPool *pThreadQueue);/*获取一个空闲线程*/
void ReleaseThreadNode(ThreadNode *pThreadNode);/*释放线程节点*/
int ExecuteTask(ThreadPool *pThreadQueue, void *(*Fun)(void *), void *pData);/*执行线程*/
void SetTaskQueueLength(ThreadPool *pThreadQueue, int *pTaskQueueLength);/*设置任务队列的长度，其是动态变化的*/

/*私有*/
void *AddThreadDynamic(void *pData);/*动态的添加线程*/
void *FreeThreadAccess(void *pData);/*未访问超时线程*/
void *FreeThreadExecute(void *pData);/*执行超时线程*/
int CreateMulThread(ThreadPool *pThreadQueue, int nNumber);
int InsertThread(ThreadPool *pThreadQueue);
void *DefaultThreadFun(void *pData);

#endif /* THREAD_THREADPOOL_H_ */
