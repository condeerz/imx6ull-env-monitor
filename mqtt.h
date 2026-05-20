#ifndef __MQTT_H__
#define __MQTT_H__

extern int mqtt_thread(void);

#include <MQTTAsync.h>
#include <MQTTClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include <string.h>
//#define OLD_ADDRESS     "tcp://218.201.45.7:1883"
#define NEW_ADDRESS     "tcp://183.230.40.96:1883"
#define DEV_NAME    "device_01"
#define CLIENTID DEV_NAME
#define PRODUCT_ID "K0Y74T2644"
#define PASSWD "version=2018-10-31&res=products%2FK0Y74T2644%2Fdevices%2Fdevice_01&et=1837255523&method=md5&sign=SEaiJxkKmNUy8ivFlx5%2BmQ%3D%3D";
#define QOS         0
#define TIMEOUT     10000L
//#define __cplusplus
#endif // HEAD_H