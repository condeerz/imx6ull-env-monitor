#ifndef __LINKQUEUE_H__
#define __LINKQUEUE_H__

//队列中存储的数据
typedef struct datatype {
    double temp;
    double hum;     
    double light;
    double smoke;
}data_t;

//队列节点的类型
typedef struct node {
    data_t data;
    struct node *pnext;
}node_t;

//队列节点的标签
typedef struct linkqueue {
    node_t *phead;                  //指向链表的空白节点
    node_t *ptail;                  //指向链表的最末尾的节点
}linkqueue_t;

#include "linkqueue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern linkqueue_t *linkcreate_empty_linkqueue(void);
extern int is_empty_linkqueue(linkqueue_t *ptmpqueue);
extern int enter_linkqueue(linkqueue_t *ptmpqueue, data_t tmpdata);
extern int quit_linkqueue(linkqueue_t *ptmpqueue, data_t *ptmpdata);
extern int destroy_linkqueue(linkqueue_t **pptmpqueue);

#endif