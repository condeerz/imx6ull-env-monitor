#ifndef __LINKQUEUE_H__
#define __LINKQUEUE_H__

/* pthread 类型必须放在结构体定义之前，否则 pthread_mutex_t 会被当成未知类型 */
#include <pthread.h>

/**
 * @brief 队列中存储的数据类型
 * 
 * 该结构体定义了队列中存储的数据，包含温度、湿度、光照和烟雾浓度四个参数
 */
typedef struct datatype {
    double temp;      ///< 温度值
    double hum;       ///< 湿度值     
    double light;     ///< 光照强度
    double smoke;     ///< 烟雾浓度
}data_t;

/**
 * @brief 队列节点类型定义
 * 
 * 该结构体定义了链式队列中单个节点的数据结构，包含数据域和指向下一个节点的指针
 */
typedef struct node {
    data_t data;              ///< 节点存储的数据
    struct node *pnext;       ///< 指向下一个节点的指针
}node_t;

/**
 * @brief 队列结构体定义
 * 
 * 该结构体定义了链式队列的整体结构，包含头指针和尾指针
 */
typedef struct linkqueue {
    node_t *phead;            ///< 指向链表的头节点（哨兵节点，不存储实际数据）
    node_t *ptail;            ///< 指向链表的最后一个节点（队尾）
    pthread_mutex_t lock;     ///< 保护本条队列：每条队列一把锁，不是全局一把
    pthread_cond_t  cond;     ///< 队列有数据时唤醒消费者
    int             closed;   ///< 置 1 后放行所有等待者，消费者可退出
}linkqueue_t;

// 注意：这里包含了重复的#include语句，这可能是原代码的一个小错误
// #include "linkqueue.h" 应该是多余的，保留其他必要的头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/**
 * @brief 创建一个空的链式队列
 * @return 成功返回队列指针，失败返回NULL
 */
extern linkqueue_t *linkcreate_empty_linkqueue(void);

/**
 * @brief 判断链式队列是否为空
 * @param ptmpqueue 队列指针
 * @return 队列为空返回1，否则返回0
 */
extern int is_empty_linkqueue(linkqueue_t *ptmpqueue);

/**
 * @brief 入队操作，在队列尾部添加元素
 * @param ptmpqueue 队列指针
 * @param tmpdata 要添加的数据
 * @return 成功返回0，失败返回-1
 */
extern int enter_linkqueue(linkqueue_t *ptmpqueue, data_t tmpdata);

/**
 * @brief 出队操作，从队列头部移除元素
 * @param ptmpqueue 队列指针
 * @param ptmpdata 用于接收出队数据的指针
 * @return 成功返回0
 */
extern int quit_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata);

/**
 * @brief 非阻塞出队：原子地"判空 + 出队"，不会再出现判空后队列被抢空
 * @param ptmpqueue 队列指针
 * @param ptmpdata 用于接收出队数据的指针
 * @return 取到数据返回1，队列为空返回0
 */
extern int try_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata);

/**
 * @brief 阻塞出队：队列为空时睡眠等待，直到有数据或队列被关闭
 * @param ptmpqueue 队列指针
 * @param ptmpdata 用于接收出队数据的指针
 * @return 取到数据返回1，队列已关闭且为空返回0
 */
extern int wait_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata);

/**
 * @brief 关闭队列并唤醒所有等待者，用于让消费者线程退出
 * @param ptmpqueue 队列指针
 */
extern void close_linkqueue(linkqueue_t *ptmpqueue);

/**
 * @brief 销毁链式队列，释放所有内存
 * @param ptmpqueue 队列指针
 * @return 成功返回0
 */
extern int destroy_linkqueue(linkqueue_t *ptmpqueue);

#endif