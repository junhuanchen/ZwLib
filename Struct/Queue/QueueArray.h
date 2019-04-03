#ifndef QUEUE_ARRAY_H
#define QUEUE_ARRAY_H

#include "..\Struct.h"

typedef uint16_t QueueAarrayType;  /** < ���н���������������  */

typedef void *QueueAarrayValue;  /** < ���н���������ָ��  */

typedef void *(*queue_array_new)(unsigned int); /** < �ڴ����뺯������  */

typedef void(*queue_array_del)(void *); /** < �ڴ��ͷź�������  */

typedef struct queue_array
{
    volatile QueueAarrayType Front, Tail, Size, MaxSize;
    queue_array_new New;   /** < �����ڴ溯��  */
    queue_array_del Del;   /** < �ͷ��ڴ溯��  */
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
