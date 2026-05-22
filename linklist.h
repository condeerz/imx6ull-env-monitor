#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#include <pthread.h>
#include "linkqueue.h"

/**
 * @brief 邮箱结构体，用于存储线程相关信息
 * 
 * 该结构体包含了线程ID、线程函数、线程名称以及关联的消息队列，
 * 主要用于邮箱系统中管理各个线程的通信
 */
typedef struct mailbox {
    pthread_t tid;                              ///< 线程ID
    void *(*pthreadfun)(void *arg);             ///< 线程执行函数指针
    char threadname[32];                        ///< 线程名称，最大32字符
    linkqueue_t *queue;                         ///< 该线程关联的消息队列
}mailbox_t;

typedef mailbox_t DataType;                     ///< 定义数据类型别名，用于链表节点存储

/**
 * @brief 链表节点结构体
 * 
 * 单向链表的节点结构，包含数据域和指向下一个节点的指针
 */
typedef struct listnode 
{
	DataType Data;                              ///< 节点存储的数据
	struct listnode *pNext;                     ///< 指向下一个节点的指针
}Node_t;

/**
 * @brief 创建一个空的链表头节点
 * @return 成功返回指向新创建头节点的指针，失败返回NULL
 * 
 * 此函数创建一个链表头节点，该节点不存储实际数据，仅作为链表的起始标识
 */
extern Node_t *CreateEmptyLinkList(void);

/**
 * @brief 在链表头部插入一个新节点
 * @param pHead 指向链表头节点的指针
 * @param TmpData 要插入的数据
 * @return 成功返回0，失败返回-1
 * 
 * 此函数将新节点插入到链表的第一个位置（头节点之后）
 */
extern int InsertHeadLinkNode(Node_t *pHead, DataType TmpData);

/**
 * @brief 删除链表中指定线程名称的节点
 * @param pHead 指向链表头节点的指针
 * @param pthreadname 要删除的节点的线程名称
 * @return 成功删除的节点数量
 * 
 * 此函数会遍历链表，查找并删除所有匹配给定线程名称的节点
 */
extern int DeleteLinkNode(Node_t *pHead, const char *pthreadname);

/**
 * @brief 根据线程名称查找链表中的节点
 * @param pHead 指向链表头节点的指针
 * @param pthreadname 要查找的线程名称
 * @return 找到则返回指向该节点的指针，否则返回NULL
 * 
 * 此函数会遍历链表，查找第一个匹配给定线程名称的节点
 */
extern Node_t *FindLinkNode(Node_t *pHead, const char *pthreadname);

/**
 * @brief 在链表尾部插入一个新节点
 * @param pHead 指向链表头节点的指针
 * @param TmpData 要插入的数据
 * @return 成功返回0，失败返回-1
 * 
 * 此函数将新节点插入到链表的最后一个位置
 */
extern int InsertTailLinkNode(Node_t *pHead, DataType TmpData);

/**
 * @brief 销毁整个链表，释放所有节点占用的内存
 * @param ppHead 指向链表头节点指针的指针
 * @return 总是返回0
 * 
 * 此函数会遍历整个链表，释放每个节点占用的内存，并将头节点指针设为NULL
 */
extern int DestroyLinkList(Node_t **ppHead);

/**
 * @brief 根据线程ID查找链表中的节点
 * @param pHead 指向链表头节点的指针
 * @param tid 要查找的线程ID
 * @return 找到则返回指向该节点的指针，否则返回NULL
 * 
 * 此函数会遍历链表，查找第一个匹配给定线程ID的节点
 */
extern Node_t *FindLinkNodeById(Node_t *pHead, pthread_t tid);

#endif