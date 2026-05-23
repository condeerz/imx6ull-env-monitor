#include "collect.h"
#include <stdio.h>
#include <pthread.h>
#include "linkqueue.h"
#include "mailbox.h"
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
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

// BH1750光照传感器相关常量
#define BH1750_ADDR             0x23    // 默认从地址
#define BH1750_POWER_DOWN       0x00    // 断电模式
#define BH1750_POWER_ON         0x01    // 上电模式
#define BH1750_RESET            0x07    // 复位
#define BH1750_CONTINUOUS_HRES_MODE  0x10  // 连续高分辨率模式 1 Lux 分辨率
#define BH1750_CONTINUOUS_HRES_MODE2 0x11  // 连续高分辨率模式2 0.5 Lux 分辨率
#define BH1750_CONTINUOUS_LRES_MODE  0x13  // 连续低分辨率模式 4 Lux 分辨率
#define BH1750_ONE_TIME_HRES_MODE    0x20  // 单次高分辨率模式
#define BH1750_ONE_TIME_HRES_MODE2   0x21  // 单次高分辨率模式2
#define BH1750_ONE_TIME_LRES_MODE    0x23  // 单次低分辨率模式

// 全局变量用于控制BH1750线程
static volatile int bh1750_thread_running = 0;

/**
 * @brief 初始化BH1750传感器
 * @param fd I2C设备文件描述符
 * @return 成功返回0，失败返回-1
 */
int bh1750_init(int fd) {
    unsigned char cmd_power_on = BH1750_POWER_ON;
    unsigned char cmd_reset = BH1750_RESET;
    unsigned char cmd_mode = BH1750_CONTINUOUS_HRES_MODE;

    // 发送上电命令
    if (write(fd, &cmd_power_on, 1) != 1) {
        perror("BH1750: Failed to power on");
        return -1;
    }
    usleep(10000); // 延迟10ms

    // 发送复位命令
    if (write(fd, &cmd_reset, 1) != 1) {
        perror("BH1750: Failed to reset");
        return -1;
    }
    usleep(10000); // 延迟10ms

    // 设置测量模式
    if (write(fd, &cmd_mode, 1) != 1) {
        perror("BH1750: Failed to set measurement mode");
        return -1;
    }

    printf("BH1750 sensor initialized successfully\n");
    return 0;
}

/**
 * @brief 读取BH1750传感器的原始数据
 * @param fd I2C设备文件描述符
 * @param lux 存储转换后的光照度值
 * @return 成功返回0，失败返回-1
 */
int bh1750_read_data(int fd, float *lux) {
    unsigned char data[2];
    int ret;

    // 读取2字节数据
    ret = read(fd, data, 2);
    if (ret != 2) {
        perror("BH1750: Failed to read data");
        return -1;
    }

    // 将两字节数据转换为光照度值
    // 计算公式: lux = (high_byte << 8 | low_byte) / 1.2
    uint16_t raw_value = (data[0] << 8) | data[1];
    *lux = (float)raw_value / 1.2f;

    return 0;
}

/**
 * @brief BH1750光照传感器读取线程
 * @param arg 参数
 * @return 线程返回值
 */
void *bh1750thread(void *arg)
{
    int fd = -1;
    char *filename = "/dev/i2c-1"; // IMX6ULL通常使用i2c-1
    float light_intensity = 0.0f;
    int init_result;

    // 打开I2C设备
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        perror("Failed to open I2C device");
        return (void*)-1;
    }

    // 设置I2C从设备地址
    if (ioctl(fd, I2C_SLAVE, BH1750_ADDR) < 0) {
        perror("Failed to set I2C slave address for BH1750");
        close(fd);
        return (void*)-1;
    }

    // 初始化BH1750传感器
    init_result = bh1750_init(fd);
    if (init_result != 0) {
        fprintf(stderr, "Failed to initialize BH1750 sensor\n");
        close(fd);
        return (void*)-1;
    }

    // 设置线程运行标志
    bh1750_thread_running = 1;

    // 主循环：持续读取光照传感器数据
    while (bh1750_thread_running) {
        if (bh1750_read_data(fd, &light_intensity) == 0) {
            // 更新全局光照值，使用互斥锁保护
            pthread_mutex_lock(&lock_light);
            curlight = (double)light_intensity;
            pthread_mutex_unlock(&lock_light);

            printf("BH1750 Light intensity: %.2f lux\n", light_intensity);
        } else {
            fprintf(stderr, "Error reading data from BH1750 sensor\n");
        }

        // 延迟3秒后继续下一次读取
        sleep(3);
    }

    // 关闭I2C设备
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