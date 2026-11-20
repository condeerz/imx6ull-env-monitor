#include "linkqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/*
 * 内部辅助函数：在【已经持有 lock】的状态下把队头数据取出来。
 * 调用者负责加解锁，这样"判空 + 取数据 + 改头尾指针"可以合成一个原子动作。
 * 返回 1 表示取到数据，0 表示队列为空。
 */
static int pop_unlocked(linkqueue_t *ptmpqueue, data_t *ptmpdata)
{
    node_t *pfreenode = NULL;

    // 头尾相等说明队列为空
    if (ptmpqueue->phead == ptmpqueue->ptail)
    {
        return 0;
    }

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

    return 1;
}

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
    ptmpqueue->closed = 0;

    // 每条队列一把互斥锁 + 一个条件变量（不是全系统一把锁）
    pthread_mutex_init(&ptmpqueue->lock, NULL);
    pthread_cond_init(&ptmpqueue->cond, NULL);

    return ptmpqueue;
}

/**
 * @brief 判断链式队列是否为空
 * @param ptmpqueue 队列指针
 * @return 队列为空返回1，否则返回0
 */
int is_empty_linkqueue(linkqueue_t *ptmpqueue)
{
    int ret = 0;

    pthread_mutex_lock(&ptmpqueue->lock);
    // 当头指针和尾指针相等时，队列为空
    ret = (ptmpqueue->phead == ptmpqueue->ptail ? 1 : 0);
    pthread_mutex_unlock(&ptmpqueue->lock);

    return ret;
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

    // 为新节点分配内存（放在锁外面，缩短持锁时间）
    pnewnode = malloc(sizeof(node_t));
    if (NULL == pnewnode)
    {
        perror("fail to malloc");
        return -1;
    }

    // 设置新节点的数据和指针
    pnewnode->data = tmpdata;
    pnewnode->pnext = NULL;

    pthread_mutex_lock(&ptmpqueue->lock);
    // 将新节点连接到队列尾部
    ptmpqueue->ptail->pnext = pnewnode;
    // 更新尾指针
    ptmpqueue->ptail = pnewnode;
    // 有数据了，唤醒正在等待的消费者
    pthread_cond_signal(&ptmpqueue->cond);
    pthread_mutex_unlock(&ptmpqueue->lock);

    return 0;
}

/**
 * @brief 出队操作，从队列头部移除元素
 * @param ptmpqueue 队列指针
 * @param ptmpdata 用于接收出队数据的指针
 * @return 成功返回0，队列为空返回-1
 */
int quit_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata)
{
    int ret = 0;

    pthread_mutex_lock(&ptmpqueue->lock);
    ret = pop_unlocked(ptmpqueue, ptmpdata);
    pthread_mutex_unlock(&ptmpqueue->lock);

    return (1 == ret ? 0 : -1);
}

/**
 * @brief 非阻塞出队：原子地"判空 + 出队"
 * @param ptmpqueue 队列指针
 * @param ptmpdata 用于接收出队数据的指针
 * @return 取到数据返回1，队列为空返回0
 */
int try_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata)
{
    int ret = 0;

    pthread_mutex_lock(&ptmpqueue->lock);
    ret = pop_unlocked(ptmpqueue, ptmpdata);
    pthread_mutex_unlock(&ptmpqueue->lock);

    return ret;
}

/**
 * @brief 阻塞出队：队列为空时睡眠，直到有数据或被关闭
 * @param ptmpqueue 队列指针
 * @param ptmpdata 用于接收出队数据的指针
 * @return 取到数据返回1，队列已关闭且为空返回0
 */
int wait_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata)
{
    int ret = 0;

    pthread_mutex_lock(&ptmpqueue->lock);

    // while 而不是 if：条件变量会虚假唤醒，醒来要重新检查
    while ((ptmpqueue->phead == ptmpqueue->ptail) && !ptmpqueue->closed)
    {
        pthread_cond_wait(&ptmpqueue->cond, &ptmpqueue->lock);
    }

    // 已关闭且仍然为空，告诉调用者"没有数据了，可以退出"
    if (ptmpqueue->phead == ptmpqueue->ptail)
    {
        pthread_mutex_unlock(&ptmpqueue->lock);
        return 0;
    }

    ret = pop_unlocked(ptmpqueue, ptmpdata);
    pthread_mutex_unlock(&ptmpqueue->lock);

    return ret;
}

/**
 * @brief 关闭队列并唤醒所有等待者
 * @param ptmpqueue 队列指针
 * @details 这里必须用 broadcast：要让所有卡在 wait_linkqueue 的线程都能醒来看 closed 标志
 */
void close_linkqueue(linkqueue_t *ptmpqueue)
{
    pthread_mutex_lock(&ptmpqueue->lock);
    ptmpqueue->closed = 1;
    pthread_cond_broadcast(&ptmpqueue->cond);
    pthread_mutex_unlock(&ptmpqueue->lock);
}

/**
 * @brief 销毁链式队列，释放所有内存
 * @param ptmpqueue 队列指针
 * @return 成功返回0
 */
int destroy_linkqueue(linkqueue_t *ptmpqueue)
{
    node_t *ptmpnode = NULL;
    node_t *pfreenode = NULL;

    if (NULL == ptmpqueue)
    {
        return 0;
    }

    // 先关闭并放行所有等待者，避免销毁时还有线程在 wait
    close_linkqueue(ptmpqueue);

    // 从头节点开始遍历并释放所有节点
    ptmpnode = pfreenode = ptmpqueue->phead;
    while (ptmpnode != NULL)
    {
        ptmpnode = ptmpnode->pnext;
        free(pfreenode);
        pfreenode = ptmpnode;
    }

    pthread_mutex_destroy(&ptmpqueue->lock);
    pthread_cond_destroy(&ptmpqueue->cond);

    // 释放队列结构体本身
    free(ptmpqueue);

    return 0;
}
