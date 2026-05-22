#ifndef __MAILBOX_H__
#define __MAILBOX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "linklist.h"
#include "linkqueue.h"

/**
 * @brief 全局邮箱链表指针，用于管理所有线程的任务队列
 * 
 * 该指针指向一个链表，链表中存储了所有线程的信息，包括线程ID、函数、名称和消息队列
 */
extern Node_t *pmailbox;

/**
 * @brief 初始化邮箱系统
 * @return 成功返回0
 * 
 * 该函数创建一个空的链表作为邮箱系统的根节点，用于管理所有线程的任务队列
 */
extern int mailbox_init(void);

/**
 * @brief 添加一个任务到邮箱系统
 * @param pthreadname 线程名称
 * @param pthreadfun 线程执行函数
 * @return 成功返回0
 * 
 * 该函数创建一个新的任务节点，包括线程ID、线程函数、线程名称和消息队列，
 * 并启动该线程，将其加入邮箱系统管理
 */
extern int mailbox_add_task(const char *pthreadname, void *(*pthreadfun)(void *arg));

/**
 * @brief 向指定线程发送消息
 * @param pthreadname 目标线程名称
 * @param tmpdata 要发送的数据
 * @return 成功返回0，失败返回-1
 * 
 * 该函数根据线程名称查找对应的任务节点，然后将数据添加到该线程的消息队列中
 */
extern int mailbox_send_msg(const char *pthreadname, data_t tmpdata);

/**
 * @brief 接收当前线程的消息
 * @param ptmpdata 接收数据的指针
 * @return 成功接收数据返回1，队列为空返回0，找不到当前线程返回-1
 * 
 * 该函数获取当前线程的ID，查找对应的任务节点，然后从该线程的消息队列中取出一条消息
 */
extern int mailbox_recv_msg(data_t *ptmpdata);

/**
 * @brief 等待所有线程结束
 * @return 成功返回0
 * 
 * 该函数遍历邮箱系统中的所有线程，等待每个线程执行完毕后才返回
 */
extern int mailbox_waitall_thread_end(void);

/**
 * @brief 销毁邮箱系统，释放所有资源
 * @return 成功返回0
 * 
 * 该函数遍历邮箱系统中的所有节点，销毁每个线程的消息队列，最后销毁整个邮箱链表
 */
extern int mailbox_destroy(void);

#endif