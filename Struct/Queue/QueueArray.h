#ifndef QUEUE_ARRAY_H
#define QUEUE_ARRAY_H

#include "..\Struct.h"

typedef uint16_t QueueAarrayType;  /** < 队列结点的数据索引类型  */

typedef void *QueueAarrayValue;  /** < 队列结点的数据域指针  */

typedef void *(*queue_array_new)(unsigned int); /** < 内存申请函数声明  */

typedef void(*queue_array_del)(void *); /** < 内存释放函数声明  */

typedef struct queue_array
{
    volatile QueueAarrayType Front, Tail, Size, MaxSize;
    queue_array_new New;   /** < 申请内存函数  */
    queue_array_del Del;   /** < 释放内存函数  */
    QueueAarrayValue Array[];
}QueueArray;

QueueArray * QueueArrayNew(QueueAarrayType MaxSize, queue_array_new New, queue_array_del Del);

void QueueArrayDel(QueueArray * Self);

bool QueueArrayMaxSize(QueueArray * Self);

QueueAarrayType QueueArraySize(QueueArray * Self);

bool QueueArrayEmpty(QueueArray * Self);

bool QueueArrayExist(QueueArray * Self);

bool QueueArrayFull(QueueArray * Self);

QueueAarrayValue QueueArrayFront(QueueArray * Self);

QueueAarrayValue QueueArrayBack(QueueArray * Self);

bool QueueArrayPush( QueueArray * Self, QueueAarrayValue Value );

void QueueArrayBreakPush(QueueArray * Self, QueueAarrayValue Value);

void QueueArrayPop(QueueArray * Self);

#endif // QUEUE_ARRAY_H
