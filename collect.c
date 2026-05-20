#include "collect.h"
#include <stdio.h>
#include <pthread.h>
#include "linkqueue.h"
#include "mailbox.h"
#include <fcntl.h>
#include <sys/types.h>


// pthread_mutex_t lock_temp;
// pthread_mutex_t lock_hum;
pthread_mutex_t lock_dht11;
pthread_mutex_t lock_light;
pthread_mutex_t lock_smoke;
double curtemp;
double curhum;
double curlight;
double cursmoke;

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



void *lightthread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock_light);
        curlight = rand() % 10000 / 100.0;
        pthread_mutex_unlock(&lock_light);
        sleep(3);
    }

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
    pthread_t tid_light;
    pthread_t tid_smoke;
    data_t senddata;

    //等待LVGL界面启动
    sleep(5);

    pthread_mutex_init(&lock_dht11, NULL);
    pthread_mutex_init(&lock_light, NULL);
    pthread_mutex_init(&lock_smoke, NULL);

    pthread_create(&tid_dht11, NULL, dht11thread, NULL);
    pthread_create(&tid_light, NULL, lightthread, NULL);
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

        sleep(1);
    }



    return NULL;
}