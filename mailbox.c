#include "mailbox.h"
#include "linklist.h"
#include "linkqueue.h"
#include <string.h>
#include <pthread.h>

/* 
 * 全局邮箱链表头指针 
 * 该链表用于管理系统中所有注册的任务（线程）。
 * 每个链表节点包含一个 DataType 结构，其中持有线程ID、线程函数、线程名称以及该线程专属的消息队列。
 */
Node_t *pmailbox = NULL;

/**
 * @brief 初始化邮箱系统
 * @details 创建一个空的带头节点的链表，作为邮箱系统的容器。
 *          后续添加的任务都将挂载到这个链表上。
 * @return 成功返回0
 */
int mailbox_init(void)
{
    // 创建一个空的链表作为邮箱系统的根节点（头节点）
    pmailbox = CreateEmptyLinkList();
    
    return 0;
}

/**
 * @brief 添加一个任务到邮箱系统
 * @details 该函数执行以下操作：
 *          1. 初始化任务数据结构，包括分配消息队列和设置线程函数。
 *          2. 创建新的 POSIX 线程。
 *          3. 将新任务作为节点插入到全局邮箱链表的尾部。
 * @param pthreadname 线程名称，用于标识和查找任务
 * @param pthreadfun 线程执行函数入口
 * @return 成功返回0
 */
int mailbox_add_task(const char *pthreadname, void *(*pthreadfun)(void *arg))
{
    DataType tmptask; // 定义临时任务数据结构体，用于构建新节点的数据

    // 设置线程执行函数指针
    tmptask.pthreadfun = pthreadfun;
    
    // 为该线程创建一个空的消息队列，用于后续线程间通信
    tmptask.queue = linkcreate_empty_linkqueue();
    
    // 复制线程名称到任务结构中，便于通过名称查找
    strcpy(tmptask.threadname, pthreadname);

    // 创建新线程
    // 注意：此处创建线程时传入的参数为 NULL，若线程函数需要参数，需修改此接口或结构体以支持参数传递
    pthread_create(&tmptask.tid, NULL, pthreadfun, NULL);

    // 将构建好的任务数据作为新节点，插入到邮箱链表的末尾
    InsertTailLinkNode(pmailbox, tmptask);

    return 0;
}

/**
 * @brief 向指定线程发送消息
 * @details 根据线程名称在邮箱链表中查找对应的任务节点，
 *          如果找到，则将数据入队到该线程专属的消息队列中。
 * @param pthreadname 目标线程名称
 * @param tmpdata 要发送的数据内容
 * @return 成功返回0，失败（未找到线程）返回-1
 */
int mailbox_send_msg(const char *pthreadname, data_t tmpdata)
{
    Node_t *ptmptask = NULL; // 指向目标线程任务节点的指针

    // 根据线程名称在链表中查找对应的任务节点
    ptmptask = FindLinkNode(pmailbox, pthreadname);
    if (NULL == ptmptask)
    {
        // 如果没找到对应的线程节点，说明该线程未注册或名称错误，返回错误码
        return -1;
    }

    // 将数据添加到该线程专属的消息队列尾部
    enter_linkqueue(ptmptask->Data.queue, tmpdata);

    return 0;
}

/**
 * @brief 接收当前线程的消息
 * @details 该函数通常在线程内部调用。
 *          1. 获取调用者的线程ID。
 *          2. 在邮箱链表中查找匹配该ID的任务节点。
 *          3. 尝试从该节点的消息队列中非阻塞地取出一条消息。
 * @param ptmpdata 接收数据的指针，用于存储取出的消息
 * @return 
 *         - 1: 成功接收到数据
 *         - 0: 队列为空，无消息可收
 *         - -1: 找不到当前线程对应的任务节点（可能未通过 mailbox_add_task 注册）
 */
int mailbox_recv_msg(data_t *ptmpdata)
{
    pthread_t tmpthread;      // 用于存储当前调用线程的ID
    Node_t *ptmptask = NULL;  // 指向当前线程对应任务节点的指针

    // 获取当前调用该函数的线程ID
    tmpthread = pthread_self();

    // 根据线程ID在链表中查找对应的任务节点
    ptmptask = FindLinkNodeById(pmailbox, tmpthread);
    if (NULL == ptmptask)
    {
        // 如果没找到当前线程对应的任务节点，返回错误
        return -1;
    }

    // 检查当前线程的消息队列是否非空
    if (!is_empty_linkqueue(ptmptask->Data.queue))
    {
        // 如果队列不为空，则从队列头部取出一条消息并存入 ptmpdata
        quit_linkqueue(ptmptask->Data.queue, ptmpdata);
        // 成功取到消息，返回1
        return 1;
    }

    // 队列为空，没有新消息，返回0
    return 0;
}

/**
 * @brief 等待所有线程结束
 * @details 遍历邮箱链表中的所有任务节点，对每个节点持有的线程ID调用 pthread_join。
 *          这将阻塞当前调用者，直到所有注册的子线程都执行完毕。
 * @return 成功返回0
 */
int mailbox_waitall_thread_end(void)
{
    Node_t *ptmpnode = NULL; // 用于遍历链表的临时指针

    // 从链表的第一个有效数据节点开始遍历（跳过头节点）
    ptmpnode = pmailbox->pNext;
    while (ptmpnode != NULL)
    {
        // 阻塞等待该节点对应的线程结束
        pthread_join(ptmpnode->Data.tid, NULL);
        
        // 移动到下一个节点
        ptmpnode = ptmpnode->pNext;
    }

    return 0;
}

/**
 * @brief 销毁邮箱系统，释放所有资源
 * @details 1. 遍历链表，销毁每个任务节点中动态分配的消息队列。
 *          2. 销毁邮箱链表本身，释放所有节点内存。
 *          3. 将全局指针置空。
 * @note 在调用此函数前，应确保所有线程已经结束（例如先调用 mailbox_waitall_thread_end），
 *       否则可能导致正在运行的线程访问已释放的队列内存。
 * @return 成功返回0
 */
int mailbox_destroy(void)
{
    Node_t *ptmpnode = NULL; // 用于遍历链表的临时指针

    // 从链表的第一个有效数据节点开始遍历
    ptmpnode = pmailbox->pNext;
    while (ptmpnode != NULL)
    {
        // 销毁该线程关联的消息队列，释放队列占用的内存
        destroy_linkqueue(ptmptask->Data.queue); // 注意：原代码此处变量名可能有误，应为 ptmpnode
        
        // 移动到下一个节点
        ptmpnode = ptmpnode->pNext;
    }
    
    // 销毁邮箱链表结构本身（包括头节点和所有剩余节点）
    DestroyLinkList(&pmailbox);

    return 0;
}