#include <stdlib.h>
#include <stdio.h>
#include "mqtt.h"
#include "mailbox.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "sqlite3.h"

// 添加全局标志用于控制线程退出
static volatile int sqlite_thread_running = 1;

// 用于停止线程的函数
void stop_sqlite_thread(void)
{
    sqlite_thread_running = 0;
}

int sqlite_thread(void *arg)
{
    int ret = 0;         // 函数返回值
    sqlite3 *pdb = NULL;
    char cmdbuf[1024] = {0};

    printf("sqlite_thread starting...\n");

    // 打开数据库连接
    ret = sqlite3_open("data.db", &pdb);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "fail to sqlite3_open: %s\n", sqlite3_errmsg(pdb));
        if(pdb) sqlite3_close(pdb);
        return -1;
    }
    
    // 创建数据表
    sprintf(cmdbuf, "create table if not exists data(temp REAL, hum REAL, light REAL, smoke REAL);");
    ret = sqlite3_exec(pdb, cmdbuf, NULL, NULL, NULL);
    if(ret != SQLITE_OK)
    {
        fprintf(stderr, "fail to sqlite3_exec: %s\n", sqlite3_errmsg(pdb));
        sqlite3_close(pdb);
        return -1;
    }

    data_t tmpdata;
    // 主循环处理邮件队列消息
    while (sqlite_thread_running)
    {
        
        ret = mailbox_recv_msg(&tmpdata);
        if(ret == -1) {
            // 真正的错误：找不到当前线程对应的邮箱任务
            printf("sqlite:mailbox_recv_msg error, ret=%d\n", ret);
            usleep(10000); // 延迟10毫秒避免忙等待
            continue;
        } else if (ret == 0) {
            // 队列为空，正常情况，稍作延迟后继续
            usleep(10000); // 延迟10毫秒
            continue;
        }

        // 插入数据到数据库
        sprintf(cmdbuf, "insert into data values(\"%.2lf\", \"%.2lf\", %.2lf, %.2lf);", tmpdata.temp, tmpdata.hum, tmpdata.light, tmpdata.smoke);
        ret = sqlite3_exec(pdb, cmdbuf, NULL, NULL, NULL);
        if(ret != SQLITE_OK)
        {
            fprintf(stderr, "fail to sqlite3_exec: %s\n", sqlite3_errmsg(pdb));
            continue;
        }
        sleep(5);
    }

    // 关闭数据库连接
    if(pdb) {
        sqlite3_close(pdb);
    }
    return 0;  // 返回整数值而非空值
}