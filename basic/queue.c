/*
 * queue.c
 *
 *  Created on: 2015年9月8日
 *      Author: liuhanchong
 */

#include "queue.h"

int InitQueue(List *list, int nMaxLen, int nOpenPrio)
{
	if (!list || nMaxLen < 1)
	{
		ErrorInfor("InitQueue", ERROR_ARGNULL);
		return 0;
	}

	if (pthread_mutex_init(&list->thMutex, NULL) != 0)
	{
		SystemErrorInfor("InitQueue", errno);
		return 0;
	}

	list->nCurQueueLen = 0;
	list->pQueueHead = NULL;
	list->nMaxQueueLen = nMaxLen;
	list->nOpenPrio = nOpenPrio;

	return 1;
}

int ReleaseQueue(List *list)
{
	if (!list)
	{
		ErrorInfor("ReleaseQueue", ERROR_ARGNULL);
		return 0;
	}

	while (!Empty(list))
	{
		DeleteForNode(list, list->pQueueHead);
	}

	if (pthread_mutex_destroy(&list->thMutex) != 0)
	{
		SystemErrorInfor("ReleaseQueue", errno);
		return 0;
	}

	return 1;
}

int GetCurQueueLen(List *list)
{
	if (!list)
	{
		ErrorInfor("GetCurQueueLen", ERROR_ARGNULL);
		return 0;
	}
	return list->nCurQueueLen;
}

int GetMaxQueueLen(List *list)
{
	if (!list)
	{
		ErrorInfor("GetMaxQueueLen", ERROR_ARGNULL);
		return 0;
	}
	return list->nMaxQueueLen;
}

void SetMaxQueueLen(List *list, int nMaxLen)
{
	if (!list || nMaxLen <= 0)
	{
		ErrorInfor("SetMaxQueueLen", ERROR_ARGNULL);
		return;
	}
	list->nMaxQueueLen = nMaxLen;
}

int Empty(List *list)
{
	if (!list)
	{
		ErrorInfor("Empty", ERROR_ARGNULL);
		return 1;
	}
	return ((list->nCurQueueLen == 0) ? 1 : 0);
}

int Insert(List *list, void *pData, int nPrio)
{
	if (!list)
	{
		ErrorInfor("Insert", ERROR_ARGNULL);
		return 0;
	}

	if (Full(list))
	{
		ErrorInfor("Insert", ERROR_OUTQUEUE);
		return 0;
	}

	QueueNode *pNode = (QueueNode *)malloc(sizeof(QueueNode));
	if (!pNode)
	{
		SystemErrorInfor("Insert", errno);
		return 0;
	}

	/*填充结构体*/
	pNode->pData = pData;
	pNode->pNext = NULL;
	pNode->pPre = NULL;
	pNode->nPrio = nPrio;

	/*第一次插入元素*/
	if (Empty(list))
	{
		list->pQueueHead = pNode;
		pNode->pNext = pNode;
		pNode->pPre = pNode;
	}
	/*链表存在元素*/
	else
	{
		//找到头指针
		QueueNode *pHead = list->pQueueHead;
		if (!pHead)
		{
			free(pNode);
			pNode = NULL;

			ErrorInfor("Insert", ERROR_INSEQUEUE);
			return 0;
		}

		//优先级队列
		if (list->nOpenPrio == 1)
		{
			while (pHead->pNext != list->pQueueHead)
			{
				if (pNode->nPrio < pHead->nPrio)
				{
					break;
				}

				pHead = pHead->pNext;
			}

			pNode->pPre = pHead;
			pNode->pNext = pHead->pNext;
			pHead->pNext->pPre = pNode;
			pHead->pNext = pNode;
		}
		//非优先级队列
		else
		{
			pNode->pPre = pHead->pPre;
			pHead->pPre->pNext = pNode;
			pHead->pPre = pNode;
			pNode->pNext = pHead;
		}
	}

	list->nCurQueueLen++;

	return 1;
}

int DeleteForNode(List *list, QueueNode *pData)
{
	if (!list || !pData || Empty(list))
	{
		ErrorInfor("DeleteForNode", ERROR_ARGNULL);
		return 0;
	}

	if (pData->pNext && pData->pPre)
	{
		pData->pNext->pPre = pData->pPre;
		pData->pPre->pNext = pData->pNext;

		//删除第一个元素改变头指针
		if (pData == list->pQueueHead)
		{
			list->pQueueHead = pData->pNext;
		}

		free(pData);
		pData = NULL;

		list->nCurQueueLen--;

		//链表为空
		if (list->nCurQueueLen == 0)
		{
			list->pQueueHead = NULL;
		}

		return 1;
	}

	ErrorInfor("DeleteForNode", ERROR_DELQUEUE);
	return 0;
}

int DeleteForIndex(List *list, int nIndex)
{
	if (!list || nIndex < 0)
	{
		ErrorInfor("DeleteForIndex", ERROR_ARGNULL);
		return 0;
	}

	QueueNode *pData = GetNodeForIndex(list, nIndex);
	if (DeleteForNode(list, pData))
	{
		return 1;
	}

	ErrorInfor("DeleteForIndex", ERROR_DELQUEUE);
	return 0;
}

QueueNode *GetHead(List *list)
{
	if (!list)
	{
		ErrorInfor("GetHead", ERROR_ARGNULL);
		return NULL;
	}

	return GetNodeForIndex(list, 0);
}

void *GetDataForIndex(List *list, int nIndex)
{
	if (!list || nIndex < 0)
	{
		ErrorInfor("GetDataForIndex", ERROR_ARGNULL);
		return NULL;
	}

	QueueNode *pTmp = GetNodeForIndex(list, nIndex);
	if (!pTmp)
	{
		return NULL;
	}

	return pTmp->pData;
}

void *GetDataForNode(QueueNode *pNode)
{
	if (!pNode)
	{
		ErrorInfor("GetDataForNode", ERROR_ARGNULL);
		return NULL;
	}

	return pNode->pData;
}

QueueNode *GetNodeForIndex(List *list, int nIndex)
{
	if (!list || nIndex < 0)
	{
		ErrorInfor("GetNodeForIndex", ERROR_ARGNULL);
		return NULL;
	}

	if (Empty(list))
	{
		return NULL;
	}

	QueueNode *pData = list->pQueueHead;
	while (nIndex > 0)
	{
		if (pData->pNext != list->pQueueHead)
		{
			pData = pData->pNext;
		}
		nIndex--;
	}

	return (nIndex == 0) ? pData : NULL;
}

int FindNodeIndex(List *list, const QueueNode *pData)
{
	if (!list || !pData || Empty(list))
	{
		ErrorInfor("FindNode", ERROR_ARGNULL);
		return -1;
	}

	int nIndex = 0;
	QueueNode *pTmp = list->pQueueHead;
	do
	{
		if (pTmp == pData)
		{
			return nIndex;
		}

		pTmp = pTmp->pNext;
		nIndex++;
	} while (pTmp != list->pQueueHead);

	return -1;
}

int FindDataIndex(List *list, const void *pData)
{
	if (!list || !pData || Empty(list))
	{
		ErrorInfor("FindData", ERROR_ARGNULL);
		return -1;
	}

	int nIndex = 0;
	QueueNode *pTmp = list->pQueueHead;
	do
	{
		if (pTmp->pData == pData)
		{
			return nIndex;
		}

		pTmp = pTmp->pNext;
		nIndex++;
	} while (pTmp != list->pQueueHead);

	return -1;
}

int ModifyData(List *list, int nIndex, void *pData)
{
	if (!list || nIndex < 0)
	{
		ErrorInfor("ModifyData", ERROR_ARGNULL);
		return 0;
	}

	QueueNode *pTmp = GetNodeForIndex(list, nIndex);
	if (pTmp)
	{
		pTmp->pData = pData;
		return 1;
	}

	ErrorInfor("ModifyNode", ERROR_EDITQUEUE);
	return 0;
}

int Full(List *list)
{
	if (!list)
	{
		ErrorInfor("Full", ERROR_ARGNULL);
		return 0;
	}

	if (GetCurQueueLen(list) < GetMaxQueueLen(list))
	{
		return 0;
	}

	return 1;
}

int InsertLog(List *list, void *pData, int nPrio)
{
	if (!list)
	{
		printf("InsertLog-%s!\n", ERROR_ARGNULL);
		return 0;
	}

	if (Full(list))
	{
		printf("InsertLog-%s!\n", ERROR_OUTQUEUE);
		return 0;
	}

	QueueNode *pNode = (QueueNode *)malloc(sizeof(QueueNode));
	if (!pNode)
	{
		printf("InsertLog-分配内存失败!\n");
		return 0;
	}

	/*填充结构体*/
	pNode->pData = pData;
	pNode->pNext = NULL;
	pNode->pPre = NULL;
	pNode->nPrio = nPrio;

	/*第一次插入元素*/
	if (Empty(list))
	{
		list->pQueueHead = pNode;
		pNode->pNext = pNode;
		pNode->pPre = pNode;
	}
	/*链表存在元素*/
	else
	{
		//找到头指针
		QueueNode *pHead = list->pQueueHead;
		if (!pHead)
		{
			free(pNode);
			pNode = NULL;

			ErrorInfor("Insert", ERROR_INSEQUEUE);
			return 0;
		}

		//优先级队列
		if (list->nOpenPrio == 1)
		{
			while (pHead->pNext != list->pQueueHead)
			{
				if (pNode->nPrio < pHead->nPrio)
				{
					break;
				}

				pHead = pHead->pNext;
			}

			pNode->pPre = pHead;
			pNode->pNext = pHead->pNext;
			pHead->pNext->pPre = pNode;
			pHead->pNext = pNode;
		}
		//非优先级队列
		else
		{
			pNode->pPre = pHead->pPre;
			pHead->pPre->pNext = pNode;
			pHead->pPre = pNode;
			pNode->pNext = pHead;
		}
	}

	list->nCurQueueLen++;

	return 1;
}
