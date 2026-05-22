//https://eclipse.dev/paho/files/mqttdoc/MQTTClient/html/_m_q_t_t_client_8h.html#a9a0518d9ca924d12c1329dbe3de5f2b6
#include <stdio.h>
#include "mqtt.h"
#include "mailbox.h"
#include <time.h>
#include <sys/time.h>

// 存储MQTT主题的静态数组
static char topic[2][200] = {0};
// MQTT客户端实例
static MQTTClient client;
// 消息ID计数器，用于唯一标识每条消息
static int id =10000;
// 用于确认消息投递的令牌
volatile static MQTTClient_deliveryToken deliveredtoken;

/**
 * @brief 构造MQTT主题字符串
 * @param dev_name 设备名称
 * @param pro_id 产品ID
 * 
 * 该函数构造两个主题字符串：
 * - topic[0]: 用于订阅服务器回复的主题
 * - topic[1]: 用于发布消息的主题
 */
void pack_topic(char * dev_name, char * pro_id)
{
	// 构造订阅主题：服务器回复主题
	sprintf(topic[0], "$sys/%s/%s/thing/property/post/reply", pro_id, dev_name);	
	// 构造发布主题：向服务器发送数据的主题
	sprintf(topic[1], "$sys/%s/%s/thing/property/post", pro_id, dev_name);			
}

/**
 * @brief 消息发送成功的回调函数
 * @param context 上下文指针
 * @param dt 消息投递令牌
 * 
 * 当MQTT消息成功发送到服务器后，此回调函数会被调用
 */
void delivered(void *context, MQTTClient_deliveryToken dt)
{
	// 输出确认消息
	printf("Message with token value %d delivery confirmed\n", dt);
	// 保存确认的令牌
	deliveredtoken = dt;
}

/**
 * @brief 接收到MQTT消息的回调函数
 * @param context 上下文指针
 * @param topicName 接收到消息的主题名称
 * @param topicLen 主题名称长度
 * @param message 接收到的消息内容
 * @return 成功处理返回1
 * 
 * 当从MQTT服务器接收到消息时，此回调函数会被调用
 */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	int i;
	char* payloadptr;

	// 输出接收到的消息信息
	printf("Message arrived\n");
	printf("     topic: %s\n", topicName);
	printf("   message: ");

	// 输出消息负载内容
	payloadptr = (char*)message->payload;
	for(i=0; i<message->payloadlen; i++)
	{
		putchar(*payloadptr++);
	}
	putchar('\n');
	// 释放消息内存
	MQTTClient_freeMessage(&message);
	// 释放主题名称内存
	MQTTClient_free(topicName);
	return 1;
}

/**
 * @brief 连接丢失的回调函数
 * @param context 上下文指针
 * @param cause 连接丢失的原因
 * 
 * 当与MQTT服务器的连接断开时，此回调函数会被调用
 */
void connlost(void *context, char *cause)
{
	// 输出连接丢失信息
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}

/**
 * @brief 初始化MQTT客户端
 * @return 成功返回MQTTCLIENT_SUCCESS，失败则退出程序
 * 
 * 该函数初始化MQTT客户端，设置连接参数，建立与服务器的连接
 */
int mqtt_init()
{
	// 构造MQTT主题
	pack_topic(DEV_NAME, PRODUCT_ID);
    
    int rc = MQTTClient_create(&client, NEW_ADDRESS, CLIENTID,
                               MQTTCLIENT_PERSISTENCE_NONE, NULL);
    int ch;
    // 检查客户端创建是否成功
    if(MQTTCLIENT_SUCCESS != rc)
    {
        printf("create mqtt client failre...\n");
        exit(1);
    }
    
    // 初始化连接选项
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;  // 保活间隔20秒
    conn_opts.cleansession = 1;        // 清理会话标志
    conn_opts.username=PRODUCT_ID;     // 用户名设为产品ID
    conn_opts.password=PASSWD;         // 密码
    
    // 设置回调函数
    rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if(MQTTCLIENT_SUCCESS !=rc )
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    // 建立与MQTT服务器的连接
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

#if 0
    // 订阅主题（如果需要）
    MQTTClient_subscribe(client, topic[0], QOS);
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           , topic[0], CLIENTID, QOS);
#endif 

	return rc;
}

/**
 * @brief 发送MQTT消息
 * @param key 数据键名（如温度、湿度等）
 * @param value 数据值
 * @return 成功返回MQTTCLIENT_SUCCESS
 * 
 * 该函数将指定键值对打包为JSON格式并通过MQTT协议发送到服务器
 */
int mqtt_send(char * key, float value)
{
	// 消息投递令牌
	MQTTClient_deliveryToken deliveryToken;
    // 初始化发布消息结构
    MQTTClient_message test2_pubmsg = MQTTClient_message_initializer;
    // 用于存储待发送消息的缓冲区
    char message[1024]={0};
    
    // 设置消息QoS等级
    test2_pubmsg.qos = QOS;
    // 设置消息不保留
    test2_pubmsg.retained = 0;
    // 设置消息负载
    test2_pubmsg.payload =message;

	// 格式化消息为JSON格式
	sprintf(message,"{\"id\":\"%d\",\"version\":\"1.0\",\"params\":{\"%s\":{\"value\":%.2lf}}}",id++, key, value);
	// 设置消息长度
	test2_pubmsg.payloadlen = strlen(message);

	// 输出即将发送的消息
	printf("%s\n",message);
	// 发布消息
	int rc = MQTTClient_publishMessage(client,topic[1],&test2_pubmsg,&deliveryToken);
	if(MQTTCLIENT_SUCCESS != rc)
	{
		// 发布失败，输出错误信息并退出
		printf("client to publish failure.. %lu\n",pthread_self());
		exit(1);
	}
	/*
     // 等待消息发送完成（可选）
     printf("Waiting for up to %d seconds for publication on topic %s for client with ClientID: %s\n"
			,(int)(TIMEOUT/1000), topic[0], CLIENTID);
	MQTTClient_waitForCompletion(client,deliveryToken,TIMEOUT);
    */
	// 等待1秒
	sleep(1);

	return rc;
}

/**
 * @brief 断开MQTT连接并清理资源
 * 
 * 该函数断开与MQTT服务器的连接并销毁客户端实例
 */
void mqtt_deinit()
{
    // 断开连接，超时时间为10秒
    MQTTClient_disconnect(client, 10000);
    // 销毁客户端实例
    MQTTClient_destroy(&client);
}


/**
 * @brief MQTT通信线程函数
 * @return 成功返回0
 * 
 * 该函数实现了MQTT通信线程的主要逻辑，从邮箱中获取传感器数据并发送到MQTT服务器
 */
int mqtt_thread(void)
{
    data_t tmpdata;      // 临时存储从邮箱获取的数据
    int ret = 0;         // 函数返回值
    
	// 初始化MQTT客户端
	mqtt_init();
	
    // 持续运行的主循环
    while(1)
    {
       // 从邮箱接收数据
       ret = mailbox_recv_msg(&tmpdata);
       
       // 检查接收结果
        if (ret == -1) {
            // 真正的错误：找不到当前线程对应的邮箱任务
            printf("mqtt:mailbox_recv_msg error, ret=%d\n", ret);
            usleep(10000); // 延迟10毫秒避免忙等待
            continue;
        } else if (ret == 0) {
            // 队列为空，正常情况，稍作延迟后继续
            usleep(10000); // 延迟10毫秒
            continue;
        }
        
        // ret == 1，成功接收到数据，发送各种传感器数据到MQTT服务器
        mqtt_send("tempval", tmpdata.temp);   // 发送温度数据
        mqtt_send("humval", tmpdata.hum);     // 发送湿度数据
        mqtt_send("lightval", tmpdata.light); // 发送光照数据
        mqtt_send("smokeval", tmpdata.smoke); // 发送烟雾数据
    }
    
    // 退出前清理MQTT资源
    mqtt_deinit();
	return 0;
}