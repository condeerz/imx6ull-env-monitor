#include "collect.h"
#include <stdio.h>
#include <pthread.h>
#include "linkqueue.h"
#include "mailbox.h"

pthread_mutex_t lock_temp;
pthread_mutex_t lock_hum;
pthread_mutex_t lock_light;
pthread_mutex_t lock_smoke;
double curtemp;
double curhum;
double curlight;
double cursmoke;

void *tempthread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock_temp);
        curtemp = rand() % 10000 / 100.0;
        pthread_mutex_unlock(&lock_temp);
        sleep(2);
    }

    return NULL;
}

void *humthread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock_hum);
        curhum = rand() % 10000 / 100.0;
        pthread_mutex_unlock(&lock_hum);
        sleep(4);
    }

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
    pthread_t tid_temp;
    pthread_t tid_hum;
    pthread_t tid_light;
    pthread_t tid_smoke;
    data_t senddata;

    //等待LVGL界面启动
    sleep(5);

    pthread_mutex_init(&lock_temp, NULL);
    pthread_mutex_init(&lock_hum, NULL);
    pthread_mutex_init(&lock_light, NULL);
    pthread_mutex_init(&lock_smoke, NULL);
    pthread_create(&tid_temp, NULL, tempthread, NULL);
    pthread_create(&tid_temp, NULL, humthread, NULL);
    pthread_create(&tid_temp, NULL, lightthread, NULL);
    pthread_create(&tid_temp, NULL, smokethread, NULL);

    while (1)
    {
        memset(&senddata, 0, sizeof(senddata));
        
        pthread_mutex_lock(&lock_temp);
        senddata.temp = curtemp;
        pthread_mutex_unlock(&lock_temp);

        pthread_mutex_lock(&lock_hum);
        senddata.hum = curhum;
        pthread_mutex_unlock(&lock_hum);

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