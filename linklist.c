/**
 * @file linklist.c
 * @brief 链表实现文件，提供了基本的链表操作功能
 * @author 
 * @version 
 * @date 2026-05-22
 */

#include "linklist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 创建一个空的链表头节点
 * @return 成功返回指向新创建头节点的指针，失败返回NULL
 * 
 * 此函数创建一个链表头节点，该节点不存储实际数据，仅作为链表的起始标识
 */
Node_t *CreateEmptyLinkList(void)
{
	Node_t *pNewNode = NULL;

	// 为新的头节点分配内存空间
	pNewNode = malloc(sizeof(Node_t));
	if (NULL == pNewNode)
	{
		perror("fail to malloc");
		return NULL;
	}

	// 初始化头节点的下一个指针为NULL，表示这是一个空链表
	pNewNode->pNext = NULL;
	
	return pNewNode;
}

/**
 * @brief 在链表头部插入一个新节点
 * @param pHead 指向链表头节点的指针
 * @param TmpData 要插入的数据
 * @return 成功返回0，失败返回-1
 * 
 * 此函数将新节点插入到链表的第一个位置（头节点之后）
 */
int InsertHeadLinkNode(Node_t *pHead, DataType TmpData)
{
	Node_t *pNewNode = NULL;

	// 为新节点分配内存空间
	pNewNode = malloc(sizeof(Node_t));
	if (NULL == pNewNode)
	{
		perror("fail to malloc");
		return -1;
	}

	// 设置新节点的数据域和指针域
	pNewNode->Data = TmpData;
	pNewNode->pNext = pHead->pNext;  // 新节点指向原来的第一个节点
	pHead->pNext = pNewNode;         // 头节点指向新节点，使其成为新的第一个节点
	
	return 0;
}

/**
 * @brief 删除链表中指定线程名称的节点
 * @param pHead 指向链表头节点的指针
 * @param pthreadname 要删除的节点的线程名称
 * @return 成功删除的节点数量
 * 
 * 此函数会遍历链表，查找并删除所有匹配给定线程名称的节点
 */
int DeleteLinkNode(Node_t *pHead, const char *pthreadname)
{
	int Cnt = 0;                    // 记录删除的节点数量
	Node_t *pTmpNode = NULL;        // 当前遍历的节点指针
	Node_t *pPreNode = NULL;        // 当前节点的前一个节点指针

	// 从第一个实际数据节点开始遍历
	pTmpNode = pHead->pNext;
	pPreNode = pHead;

	// 遍历整个链表
	while (pTmpNode != NULL)
	{
		// 检查当前节点的线程名称是否匹配
		if (0 == strcmp(pTmpNode->Data.threadname, pthreadname))
		{
			// 找到匹配的节点，从链表中移除
			pPreNode->pNext = pTmpNode->pNext;
			// 释放节点内存
			free(pTmpNode);
			// 更新当前节点指针为下一个节点
			pTmpNode = pPreNode->pNext;
			// 增加删除计数
			Cnt++;
		}
		else 
		{
			// 当前节点不匹配，继续遍历
			pTmpNode = pTmpNode->pNext;
			pPreNode = pPreNode->pNext;
		}
	}

	return Cnt;
}

/**
 * @brief 根据线程名称查找链表中的节点
 * @param pHead 指向链表头节点的指针
 * @param pthreadname 要查找的线程名称
 * @return 找到则返回指向该节点的指针，否则返回NULL
 * 
 * 此函数会遍历链表，查找第一个匹配给定线程名称的节点
 */
Node_t *FindLinkNode(Node_t *pHead, const char *pthreadname)
{
	Node_t *pTmpNode = NULL;
	
	// 从第一个实际数据节点开始查找
	pTmpNode = pHead->pNext;
	while (pTmpNode != NULL)
	{
		// 比较线程名称是否匹配
		if (0 == strcmp(pTmpNode->Data.threadname, pthreadname))
		{
			// 找到匹配的节点，返回其指针
			return pTmpNode;
		}
		// 继续查找下一个节点
		pTmpNode = pTmpNode->pNext;
	}

	// 未找到匹配的节点，返回NULL
	return NULL;
}

/**
 * @brief 根据线程ID查找链表中的节点
 * @param pHead 指向链表头节点的指针
 * @param tid 要查找的线程ID
 * @return 找到则返回指向该节点的指针，否则返回NULL
 * 
 * 此函数会遍历链表，查找第一个匹配给定线程ID的节点
 */
Node_t *FindLinkNodeById(Node_t *pHead, pthread_t tid)
{
	Node_t *pTmpNode = NULL;
	
	// 从第一个实际数据节点开始查找
	pTmpNode = pHead->pNext;
	while (pTmpNode != NULL)
	{
		// 比较线程ID是否匹配
		if (pTmpNode->Data.tid == tid)
		{
			// 找到匹配的节点，返回其指针
			return pTmpNode;
		}
		// 继续查找下一个节点
		pTmpNode = pTmpNode->pNext;
	}

	// 未找到匹配的节点，返回NULL
	return NULL;
}

/**
 * @brief 在链表尾部插入一个新节点
 * @param pHead 指向链表头节点的指针
 * @param TmpData 要插入的数据
 * @return 成功返回0，失败返回-1
 * 
 * 此函数将新节点插入到链表的最后一个位置
 */
int InsertTailLinkNode(Node_t *pHead, DataType TmpData)
{
	Node_t *pNewNode = NULL;
	Node_t *pLastNode = NULL;

	// 为新节点分配内存空间
	pNewNode = malloc(sizeof(Node_t));
	if (NULL == pNewNode)
	{
		perror("fail to malloc");
		return -1;
	}

	// 设置新节点的数据和指针
	pNewNode->Data = TmpData;
	pNewNode->pNext = NULL;

	// 找到最后一个节点
	pLastNode = pHead;
	while (pLastNode->pNext != NULL)
	{
		pLastNode = pLastNode->pNext;
	}
	// 将新节点连接到链表末尾
	pLastNode->pNext = pNewNode;

	return 0;
}

/**
 * @brief 销毁整个链表，释放所有节点占用的内存
 * @param ppHead 指向链表头节点指针的指针
 * @return 总是返回0
 * 
 * 此函数会遍历整个链表，释放每个节点占用的内存，并将头节点指针设为NULL
 */
int DestroyLinkList(Node_t **ppHead)
{
	Node_t *pTmpNode = NULL;
	Node_t *pFreeNode = NULL;

	// 初始化两个指针，一个用于遍历，一个用于释放内存
	pTmpNode = *ppHead;
	pFreeNode = *ppHead;
	while (pTmpNode != NULL)
	{
		// 先保存当前节点，然后移动到下一个节点
		pTmpNode = pTmpNode->pNext;
		// 释放之前保存的节点
		free(pFreeNode);
		// 更新要释放的节点
		pFreeNode = pTmpNode;
	}
	// 将头节点指针设为NULL，表示链表已被销毁
	*ppHead = NULL;

	return 0;
}

