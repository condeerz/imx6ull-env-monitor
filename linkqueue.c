#include "linkqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief 创建一个空的链式队列
 * @return 成功返回队列指针，失败返回NULL
 */
linkqueue_t *linkcreate_empty_linkqueue(void)
{
    linkqueue_t *ptmpqueue = NULL;

    // 分配队列结构体内存
    ptmpqueue = malloc(sizeof(linkqueue_t));
    if (NULL == ptmpqueue) 
    {
        perror("fail to malloc");
        return NULL;
    }

    // 为头节点分配内存，头节点不存储实际数据，仅作为哨兵节点
    ptmpqueue->phead = malloc(sizeof(node_t));
    if (NULL == ptmpqueue->phead)
    {
        perror("fail to malloc");
        return NULL;
    }

    // 初始化尾指针指向头节点
    ptmpqueue->ptail = ptmpqueue->phead;

    return ptmpqueue;
}

/**
 * @brief 判断链式队列是否为空
 * @param ptmpqueue 队列指针
 * @return 队列为空返回1，否则返回0
 */
int is_empty_linkqueue(linkqueue_t *ptmpqueue)
{
    // 当头指针和尾指针相等时，队列为空
    return (ptmpqueue->phead == ptmpqueue->ptail ? 1 : 0);
}

/**
 * @brief 入队操作，在队列尾部添加元素
 * @param ptmpqueue 队列指针
 * @param tmpdata 要添加的数据
 * @return 成功返回0，失败返回-1
 */
int enter_linkqueue(linkqueue_t *ptmpqueue, data_t tmpdata)
{
    node_t *pnewnode = NULL;

    // 为新节点分配内存
    pnewnode = malloc(sizeof(node_t));
    if (NULL == pnewnode)
    {
        perror("fail to malloc");
        return -1;
    }

    // 设置新节点的数据和指针
    pnewnode->data = tmpdata;
    pnewnode->pnext = NULL;
    // 将新节点连接到队列尾部
    ptmpqueue->ptail->pnext = pnewnode;
    // 更新尾指针
    ptmpqueue->ptail = pnewnode;

    return 0;
}

/**
 * @brief 出队操作，从队列头部移除元素
 * @param ptmpqueue 队列指针
 * @param ptmpdata 用于接收出队数据的指针
 * @return 成功返回0
 */
int quit_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata)
{
    node_t *pfreenode = NULL;

    // 获取要删除的节点（头节点的下一个节点）
    pfreenode = ptmpqueue->phead->pnext;
    // 将节点中的数据复制到输出参数
    *ptmpdata = pfreenode->data;
    // 更新头节点的下一个指针
    ptmpqueue->phead->pnext = pfreenode->pnext;
    // 如果删除的是最后一个元素，需要更新尾指针
    if (NULL == pfreenode->pnext)
    {
        ptmpqueue->ptail = ptmpqueue->phead;
    }
    // 释放被删除的节点内存
    free(pfreenode);

    return 0;
}

/**
 * @brief 销毁链式队列，释放所有内存
 * @param pptmpqueue 指向队列指针的指针
 * @return 成功返回0
 */
int destroy_linkqueue(linkqueue_t **pptmpqueue)
{
    node_t *ptmpnode = NULL;
    node_t *pfreenode = NULL;

    // 从头节点开始遍历并释放所有节点
    ptmpnode = pfreenode = (*pptmpqueue)->phead;
    while (ptmpnode != NULL)
    {
        ptmpnode = ptmpnode->pnext;
        free(pfreenode);
        pfreenode = ptmpnode;
    }
    // 释放队列结构体本身
    free((*pptmpqueue));
    *pptmpqueue = NULL;

    return 0;
}