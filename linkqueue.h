#ifndef __LINKQUEUE_H__
#define __LINKQUEUE_H__

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
}linkqueue_t;

// 注意：这里包含了重复的#include语句，这可能是原代码的一个小错误
// #include "linkqueue.h" 应该是多余的，保留其他必要的头文件
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
 * @brief 销毁链式队列，释放所有内存
 * @param pptmpqueue 指向队列指针的指针
 * @return 成功返回0
 */
extern int destroy_linkqueue(linkqueue_t **pptmpqueue);

#endif