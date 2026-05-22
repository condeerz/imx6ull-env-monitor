#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "ui.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "mailbox.h"
#include "ui.h"

// 定义显示缓冲区大小为128KB
#define DISP_BUF_SIZE (128 * 1024)

/**
 * @brief LVGL定时器回调函数，用于更新UI界面数据
 * @param timer LVGL定时器对象指针
 * 
 * 该函数定期从邮箱系统接收传感器数据并更新界面上的显示值
 */
void my_timer(lv_timer_t * timer)
{
    char tmpbuff[8] = {0};      // 临时缓冲区，用于格式化数据
    data_t tmpdata;             // 存储接收到的数据
    int ret = 0;                // 函数返回值

    // 从邮箱系统接收消息
    ret = mailbox_recv_msg(&tmpdata);
    // 如果接收失败或队列为空，则直接返回
    if (ret <= 0)
    {
        return;
    }

    // 将温度数据格式化并更新到UI标签
    sprintf(tmpbuff, "%.2lf", tmpdata.temp);
    lv_label_set_text(ui_LabelTempVal, tmpbuff);

    // 将湿度数据格式化并更新到UI标签
    sprintf(tmpbuff, "%.2lf", tmpdata.hum);
    lv_label_set_text(ui_LabelHumVal, tmpbuff);

    // 将光照数据格式化并更新到UI标签
    sprintf(tmpbuff, "%.2lf", tmpdata.light);
    lv_label_set_text(ui_LabellightVal, tmpbuff);

    // 将烟雾数据格式化并更新到UI标签
    sprintf(tmpbuff, "%.2lf", tmpdata.smoke);
    lv_label_set_text(ui_Label11, tmpbuff);

    return;
}

/**
 * @brief LVGL主线程函数，初始化LVGL库并启动GUI循环
 * @return 成功返回0
 * 
 * 该函数负责初始化LVGL库、显示驱动、输入设备驱动，并创建UI界面
 */
int lvgl_thread(void)
{
    lv_timer_t *timer = NULL;

    /* 初始化LVGL库 */
    lv_init();

    /* 初始化Linux帧缓冲设备驱动 */
    fbdev_init();

    /* 为LVGL分配一个小的缓冲区用于绘制屏幕内容 */
    static lv_color_t buf[DISP_BUF_SIZE];

    /* 初始化显示缓冲区描述符 */
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /* 初始化并注册显示驱动 */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;        // 设置绘图缓冲区
    disp_drv.flush_cb   = fbdev_flush;      // 设置刷新回调函数
    disp_drv.hor_res    = 800;              // 设置水平分辨率
    disp_drv.ver_res    = 480;              // 设置垂直分辨率
    lv_disp_drv_register(&disp_drv);

    /* 初始化输入设备驱动（触摸屏/鼠标） */
    evdev_init();
    static lv_indev_drv_t indev_drv_1;
    lv_indev_drv_init(&indev_drv_1);        /* 基本初始化 */
    indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    /* 此函数将定期被库调用来获取鼠标位置和状态 */
    indev_drv_1.read_cb = evdev_read;
    lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);


    /* 为鼠标设置光标 */
    LV_IMG_DECLARE(mouse_cursor_icon)       // 声明鼠标光标图像
    lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /* 为光标创建一个图像对象 */
    lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /* 设置图像源 */
    lv_indev_set_cursor(mouse_indev, cursor_obj);             /* 将图像对象连接到驱动 */


    /* 创建UI界面 */
    //lv_demo_widgets();  // 可选：运行LVGL演示程序
    ui_init();            // 初始化用户界面

    // 创建定时器，每1000ms调用一次my_timer函数
    timer = lv_timer_create(my_timer, 1000, NULL);

    /* 处理LVGL任务（节拍模式） */
    while(1) {
        lv_timer_handler();     // 处理LVGL定时器
        usleep(5000);          // 延迟5毫秒，降低CPU使用率
    }

    // 理论上不会执行到这里，因为是无限循环
    ui_destroy();

    return 0;
}

/**
 * @brief 自定义的系统时钟获取函数
 * @return 当前系统时间戳（毫秒）
 * 
 * 该函数实现了LVGL库所需的自定义时钟函数，用于提供系统时间
 */
/* 在 lv_conf.h 中设置为 `LV_TICK_CUSTOM_SYS_TIME_EXPR` */
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;       // 记录程序启动时的时间戳
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}