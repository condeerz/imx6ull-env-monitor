#include "collect.h"
#include <stdio.h>
#include <pthread.h>
#include "linkqueue.h"
#include "mailbox.h"
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>  // 添加此行以定义uint16_t等类型

// pthread_mutex_t lock_temp;
// pthread_mutex_t lock_hum;
pthread_mutex_t lock_dht11;
pthread_mutex_t lock_light;
pthread_mutex_t lock_smoke;
double curtemp;
double curhum;
double curlight;
double cursmoke;

/* ------------------------------------------------------------------
 * BH1750 光照采集：走内核 I2C 子系统驱动暴露的字符设备 /dev/bh1750_misc
 *   驱动侧：18-BH1750_i2c/bh1750_drv/bh1750_drv.c
 *           i2c_driver + of_match_table(rohm,bh1750fvi) + misc_register
 *   接口约定：驱动 read() 返回 1 个 int，单位毫勒克斯(mLux)，除 1000 得 lux
 * ------------------------------------------------------------------ */
#define BH1750_DEV_NODE      "/dev/bh1750_misc"
#define BH1750_MLUX_PER_LUX  1000

// 全局变量用于控制BH1750线程
static volatile int bh1750_thread_running = 0;

/**
 * @brief BH1750光照传感器读取线程（读内核驱动字符设备）
 * @param arg 参数
 * @return 线程返回值
 */
void *bh1750thread(void *arg)
{
    int fd = -1;
    int mlux = 0;
    float light_intensity = 0.0f;

    fd = open(BH1750_DEV_NODE, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open " BH1750_DEV_NODE);
        return (void *)-1;
    }

    bh1750_thread_running = 1;

    while (bh1750_thread_running) {
        int ret = read(fd, &mlux, sizeof(mlux));

        if (ret == (int)sizeof(mlux)) {
            /* 驱动给的是毫勒克斯，换算成 lux */
            light_intensity = (float)mlux / BH1750_MLUX_PER_LUX;

            pthread_mutex_lock(&lock_light);
            curlight = (double)light_intensity;
            pthread_mutex_unlock(&lock_light);

            printf("BH1750 Light intensity: %.2f lux (raw %d mLux)\n",
                   light_intensity, mlux);
        } else {
            fprintf(stderr, "Error reading %s: ret=%d\n",
                    BH1750_DEV_NODE, ret);
        }

        sleep(3);
    }

    close(fd);
    printf("BH1750 thread terminated\n");

    return NULL;
}

void *dht11thread(void *arg)
{
    int fd = 0;
    unsigned char data[5] = {0};

    fd = open("/dev/dht11_misc", O_RDWR);
    if (-1 == fd) {
        perror("fail to open");
        return (void*)-1;
    }

    while (1)
    {
        ssize_t bytes_read = read(fd, data, sizeof(data));
        if (bytes_read != sizeof(data)) {
            // 如果读取失败或数据长度不正确，跳过此次处理
            if (bytes_read < 0) {
                perror("read failed");
            } else {
                fprintf(stderr, "Incomplete data read: %zd bytes\n", bytes_read);
            }
            sleep(2);
            continue;
        }

        pthread_mutex_lock(&lock_dht11);
        curtemp = data[2] + data[3] / 100.0;
        curhum = data[0] + data[1] / 100.0;
        pthread_mutex_unlock(&lock_dht11);

 //       printf("temp:%.2lf   hum:%.2lf\n", curtemp, curhum);
        sleep(2);
    }
    
    close(fd);

    return NULL;
}



void *smokethread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock_smoke);
        cursmoke = rand() % 10000 / 100.0;
        pthread_mutex_unlock(&lock_smoke);
        sleep(5);
    }

    return NULL;
}

void *collect_thread(void *arg)
{
    
    pthread_t tid_dht11;
    pthread_t tid_bh1750;  // 新增BH1750线程ID
    pthread_t tid_smoke;
    data_t senddata;

    //等待LVGL界面启动
    sleep(5);

    pthread_mutex_init(&lock_dht11, NULL);
    pthread_mutex_init(&lock_light, NULL);
    pthread_mutex_init(&lock_smoke, NULL);

    pthread_create(&tid_dht11, NULL, dht11thread, NULL);
    pthread_create(&tid_bh1750, NULL, bh1750thread, NULL);  // 创建BH1750线程
    pthread_create(&tid_smoke, NULL, smokethread, NULL);

    while (1)
    {
        memset(&senddata, 0, sizeof(senddata));
        
        pthread_mutex_lock(&lock_dht11);
        senddata.temp = curtemp;
        senddata.hum = curhum;
        pthread_mutex_unlock(&lock_dht11);

        pthread_mutex_lock(&lock_light);
        senddata.light = curlight;
        pthread_mutex_unlock(&lock_light);

        pthread_mutex_lock(&lock_smoke);
        senddata.smoke = cursmoke;
        pthread_mutex_unlock(&lock_smoke);

        mailbox_send_msg("lvgl_thread", senddata);
        mailbox_send_msg("mqtt_thread", senddata);
        mailbox_send_msg("sqlite_thread", senddata);

        sleep(1);
    }

    // 清理资源
    bh1750_thread_running = 0;  // 设置停止标志
    pthread_join(tid_bh1750, NULL);  // 等待BH1750线程结束

    return NULL;
}