#include <stdio.h>
#include <pthread.h>
#include "linkqueue.h"
#include "mailbox.h"
#include "collect.h"
#include "mqtt.h"
#include "sqlite.h"
#include "log.h"

extern int lvgl_thread(void);

int main(void)
{
    int ret = 0;
    LogInit("./demo.log");
    LogSetLevel(LOG_LEVEL_TRACE);
    LogWrite(LOG_LEVEL_INFO,"程序开始执行");
    mailbox_init();
    LogWrite(LOG_LEVEL_INFO,"mailbox 初始化完成");
    mailbox_add_task("collect_thread", collect_thread);
    LogWrite(LOG_LEVEL_INFO,"collect_thread 添加完成");
    mailbox_add_task("sqlite_thread", sqlite_thread);
    LogWrite(LOG_LEVEL_INFO,"sqlite_thread 添加完成");
    mailbox_add_task("lvgl_thread", lvgl_thread);
    LogWrite(LOG_LEVEL_INFO,"lvgl_thread 添加完成");
    mailbox_add_task("mqtt_thread", mqtt_thread);
    LogWrite(LOG_LEVEL_INFO,"mqtt_thread 添加完成");
    mailbox_waitall_thread_end();
    LogWrite(LOG_LEVEL_INFO,"所有线程结束");

    mailbox_destroy();
    LogWrite(LOG_LEVEL_INFO,"mailbox 销毁完成");
    LogDeInit();
    LogWrite(LOG_LEVEL_INFO,"程序结束执行");

    return 0;
}