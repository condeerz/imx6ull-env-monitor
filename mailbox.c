#include "mailbox.h"
#include "linklist.h"
#include "linkqueue.h"

/* 全局邮箱链表指针，用于管理所有线程的任务队列 */
Node_t *pmailbox = NULL;

/**
 * @brief 初始化邮箱系统
 * @return 成功返回0
 */
int mailbox_init(void)
{
    // 创建一个空的链表作为邮箱系统的根节点
    pmailbox = CreateEmptyLinkList();
    
    return 0;
}

/**
 * @brief 添加一个任务到邮箱系统
 * @param pthreadname 线程名称
 * @param pthreadfun 线程执行函数
 * @return 成功返回0
 */
int mailbox_add_task(const char *pthreadname, void *(*pthreadfun)(void *arg))
{
    DataType tmptask; // 定义任务数据结构体

    // 设置线程执行函数
    tmptask.pthreadfun = pthreadfun;
    // 为该线程创建一个空的消息队列
    tmptask.queue = linkcreate_empty_linkqueue();
    // 复制线程名称
    strcpy(tmptask.threadname, pthreadname);

    // 创建线程，并将线程ID保存到tmptask中
    pthread_create(&tmptask.tid, NULL, pthreadfun, NULL);

    // 将任务节点插入到邮箱链表的末尾
    InsertTailLinkNode(pmailbox, tmptask);

    return 0;
}

/**
 * @brief 向指定线程发送消息
 * @param pthreadname 目标线程名称
 * @param tmpdata 要发送的数据
 * @return 成功返回0，失败返回-1
 */
int mailbox_send_msg(const char *pthreadname, data_t tmpdata)
{
    Node_t *ptmptask = NULL; // 指向目标线程任务的指针

    // 根据线程名称查找对应的任务节点
    ptmptask = FindLinkNode(pmailbox, pthreadname);
    if (NULL == ptmptask)
    {
        // 如果没找到对应的线程，则返回错误
        return -1;
    }

    // 将数据添加到该线程的消息队列中
    enter_linkqueue(ptmptask->Data.queue, tmpdata);

    return 0;
}

/**
 * @brief 接收当前线程的消息
 * @param ptmpdata 接收数据的指针
 * @return 成功接收数据返回1，队列为空返回0，找不到当前线程返回-1
 */
int mailbox_recv_msg(data_t *ptmpdata)
{
    pthread_t tmpthread;      // 当前线程ID
    Node_t *ptmptask = NULL;  // 指向当前线程任务的指针

    // 获取当前线程的ID
    tmpthread = pthread_self();

    // 根据线程ID查找对应的任务节点
    ptmptask = FindLinkNodeById(pmailbox, tmpthread);
    if (NULL == ptmptask)
    {
        // 如果没找到当前线程对应的任务节点，则返回错误
        return -1;
    }

    // 检查当前线程的消息队列是否为空
    if (!is_empty_linkqueue(ptmptask->Data.queue))
    {
        // 如果队列不为空，则从队列中取出一条消息
        quit_linkqueue(ptmptask->Data.queue, ptmpdata);
        // 成功取到消息，返回1
        return 1;
    }

    // 队列为空，返回0
    return 0;
}

/**
 * @brief 等待所有线程结束
 * @return 成功返回0
 */
int mailbox_waitall_thread_end(void)
{
    Node_t *ptmpnode = NULL; // 遍历节点的指针

    // 从第一个有效节点开始遍历
    ptmpnode = pmailbox->pNext;
    while (ptmpnode != NULL)
    {
        // 等待线程结束
        pthread_join(ptmpnode->Data.tid, NULL);
        ptmpnode = ptmpnode->pNext;
    }

    return 0;
}

/**
 * @brief 销毁邮箱系统，释放所有资源
 * @return 成功返回0
 */
int mailbox_destroy(void)
{
    Node_t *ptmpnode = NULL; // 遍历节点的指针

    // 从第一个有效节点开始遍历
    ptmpnode = pmailbox->pNext;
    while (ptmpnode != NULL)
    {
        // 销毁该线程的消息队列
        destroy_linkqueue(ptmpnode->Data.queue);
        ptmpnode = ptmpnode->pNext;
    }
    // 销毁邮箱链表
    DestroyLinkList(&pmailbox);

    return 0;
}