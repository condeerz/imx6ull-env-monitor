/**
 * @file linklist.c
 * @brief 链表实现：哨兵头节点 + 头插/尾插/按线程名查找删除
 * @author 徐煜果
 * @date 2026-05-22
 */

#include "linklist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 创建空链表：只分配一个哨兵节点，不存数据，next 置 NULL */
Node_t *CreateEmptyLinkList(void)
{
	Node_t *pNewNode = NULL;

	pNewNode = malloc(sizeof(Node_t));
	if (NULL == pNewNode)
	{
		perror("fail to malloc");
		return NULL;
	}

	// 空链表：哨兵 next 置空
	pNewNode->pNext = NULL;
	
	return pNewNode;
}

/* 头插：新节点挂在哨兵后面，O(1)；调用方保证 pHead 有效 */
int InsertHeadLinkNode(Node_t *pHead, DataType TmpData)
{
	Node_t *pNewNode = NULL;

	pNewNode = malloc(sizeof(Node_t));
	if (NULL == pNewNode)
	{
		perror("fail to malloc");
		return -1;
	}

	pNewNode->Data = TmpData;
	pNewNode->pNext = pHead->pNext;  // 先接上原来的第一个节点
	pHead->pNext = pNewNode;         // 哨兵指向新节点
	
	return 0;
}

/* 按线程名删除所有匹配节点，返回删除个数；先摘链再 free */
int DeleteLinkNode(Node_t *pHead, const char *pthreadname)
{
	int Cnt = 0;                    // 记录删除的节点数量
	Node_t *pTmpNode = NULL;        // 当前遍历的节点指针
	Node_t *pPreNode = NULL;        // 当前节点的前一个节点指针

	pTmpNode = pHead->pNext;
	pPreNode = pHead;

	while (pTmpNode != NULL)
	{
		if (0 == strcmp(pTmpNode->Data.threadname, pthreadname))
		{
			// 先摘链
			pPreNode->pNext = pTmpNode->pNext;
			free(pTmpNode);
			pTmpNode = pPreNode->pNext;
			Cnt++;
		}
		else 
		{
			pTmpNode = pTmpNode->pNext;
			pPreNode = pPreNode->pNext;
		}
	}

	return Cnt;
}

/* 按线程名找第一个匹配节点；从哨兵的下一个开始 strcmp，O(n) */
Node_t *FindLinkNode(Node_t *pHead, const char *pthreadname)
{
	Node_t *pTmpNode = NULL;
	
	pTmpNode = pHead->pNext;
	while (pTmpNode != NULL)
	{
		if (0 == strcmp(pTmpNode->Data.threadname, pthreadname))
		{
			return pTmpNode;
		}
		pTmpNode = pTmpNode->pNext;
	}

	return NULL;
}

/* 按 pthread_t 找节点；tid 来自 pthread_self() */
Node_t *FindLinkNodeById(Node_t *pHead, pthread_t tid)
{
	Node_t *pTmpNode = NULL;
	
	pTmpNode = pHead->pNext;
	while (pTmpNode != NULL)
	{
		if (pTmpNode->Data.tid == tid)
		{
			return pTmpNode;
		}
		pTmpNode = pTmpNode->pNext;
	}

	return NULL;
}

/* 尾插：先走到最后一个节点再挂新节点，O(n) */
int InsertTailLinkNode(Node_t *pHead, DataType TmpData)
{
	Node_t *pNewNode = NULL;
	Node_t *pLastNode = NULL;

	pNewNode = malloc(sizeof(Node_t));
	if (NULL == pNewNode)
	{
		perror("fail to malloc");
		return -1;
	}

	pNewNode->Data = TmpData;
	pNewNode->pNext = NULL;

	pLastNode = pHead;
	while (pLastNode->pNext != NULL)
	{
		pLastNode = pLastNode->pNext;
	}
	pLastNode->pNext = pNewNode;

	return 0;
}

/* 销毁链表：逐个 free，最后把调用方的头指针置 NULL */
int DestroyLinkList(Node_t **ppHead)
{
	Node_t *pTmpNode = NULL;
	Node_t *pFreeNode = NULL;

	pTmpNode = *ppHead;
	pFreeNode = *ppHead;
	while (pTmpNode != NULL)
	{
		// 先存 next，再 free 当前节点
		pTmpNode = pTmpNode->pNext;
		free(pFreeNode);
		pFreeNode = pTmpNode;
	}
	// 头指针置空，防止调用方继续用
	*ppHead = NULL;

	return 0;
}

