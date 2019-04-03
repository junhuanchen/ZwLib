#include "QueueArray.h"

QueueArray * QueueArrayNew(QueueAarrayType MaxSize, queue_array_new New, queue_array_del Del)
{
    QueueArray * queue = (QueueArray *)New(sizeof(*queue) + MaxSize * sizeof(QueueAarrayValue));
    if (NULL != queue)
    {
		queue->Size = 0; 
		queue->Front = 1, queue->Tail = 0;
        queue->MaxSize = MaxSize;
        queue->New = New, queue->Del = Del;
    }
    return queue;
}

void QueueArrayDel(QueueArray * Self)
{
    Self->Del(Self);
}

bool QueueArrayMaxSize(QueueArray * Self)
{
    return Self->MaxSize;
}

QueueAarrayType QueueArraySize(QueueArray * Self)
{
    return Self->Size;
}

bool QueueArrayEmpty(QueueArray * Self)
{
    return 0 == Self->Size;
}

bool QueueArrayExist(QueueArray * Self)
{
    return !QueueArrayEmpty(Self);
}

bool QueueArrayFull(QueueArray * Self)
{
    return Self->Size == Self->MaxSize;
}

QueueAarrayValue QueueArrayFront(QueueArray *Self)
{
    return QueueArrayEmpty(Self) ? (QueueAarrayValue)(0) : Self->Array[Self->Front];
}

#define QueueArrayMove(Pos, Max) Pos = (Pos + 1) % Max;

bool QueueArrayPush( QueueArray * Self, QueueAarrayValue Value )
{
    if (false == QueueArrayFull(Self))
    {
        QueueArrayMove(Self->Tail, Self->MaxSize);
        Self->Array[Self->Tail] = Value;
        Self->Size++;
        return true;
    }
    return false;
}

void QueueArrayBreakPush(QueueArray * Self, QueueAarrayValue Value)
{
    if (true == QueueArrayFull(Self))
    {
        // ������������½��Ƴ���ǰ��ͷ������ƶ���
        QueueArrayMove(Self->Front, Self->MaxSize);
    }
    else
    {
        Self->Size++;
    }
    QueueArrayMove(Self->Tail, Self->MaxSize);
    Self->Array[Self->Tail] = Value;
}

void QueueArrayPop(QueueArray * Self)
{
    if (QueueArrayExist(Self))
    {
        Self->Array[Self->Front] = NULL;
        QueueArrayMove(Self->Front, Self->MaxSize);
        Self->Size--;
    }
}

#ifdef UNIT_TEST

#define _CRTDBG_MAP_ALLOC /**< VS �ṩ�� malloc �ڴ�й©����  */
#include <crtdbg.h>
#include <assert.h>
#include <stdlib.h>

void UnitTestQueueArray(void)
{
    // ��ʼ������
    QueueArray * queue;
    queue = QueueArrayNew(3, malloc, free);
    // ����Ϊ��
    assert(true == QueueArrayEmpty(queue));
    assert(false == QueueArrayExist(queue));
    assert(0 == QueueArraySize(queue));
    // ���Ԫ�أ����Զ���
    QueueArrayPush(queue, (QueueAarrayValue)(1));
    assert((QueueAarrayValue)(1) == QueueArrayFront(queue));
    assert(1 == QueueArraySize(queue));
    QueueArrayPush(queue, (QueueAarrayValue)(2));
    assert((QueueAarrayValue)(1) == QueueArrayFront(queue));
    assert(2 == QueueArraySize(queue));
    QueueArrayPop(queue);
    assert((QueueAarrayValue)(2) == QueueArrayFront(queue));
    assert(1 == QueueArraySize(queue));
    // ���Ԫ�أ����Զ�������
    assert(true == QueueArrayPush(queue, (QueueAarrayValue)(3)));
    assert(2 == QueueArraySize(queue));
    assert(true == QueueArrayPush(queue, (QueueAarrayValue)(4)));
    assert(3 == QueueArraySize(queue));
    assert(false == QueueArrayPush(queue, (QueueAarrayValue)(5))); // ʧ��
    assert(3 == QueueArraySize(queue));
    assert((QueueAarrayValue)(2) == QueueArrayFront(queue));
    QueueArrayPop(queue);
    assert(2 == QueueArraySize(queue));
    assert((QueueAarrayValue)(3) == QueueArrayFront(queue));
    QueueArrayPop(queue);
    assert(1 == QueueArraySize(queue));
    assert((QueueAarrayValue)(4) == QueueArrayFront(queue));
    QueueArrayPop(queue);
    assert(0 == QueueArraySize(queue));
    assert((QueueAarrayValue)(0) == QueueArrayFront(queue));
    // ���Ԫ�أ����Զ���
    QueueArrayBreakPush(queue, (QueueAarrayValue)(1));
    assert((QueueAarrayValue)(1) == QueueArrayFront(queue));
    assert(1 == QueueArraySize(queue));
    QueueArrayBreakPush(queue, (QueueAarrayValue)(2));
    assert((QueueAarrayValue)(1) == QueueArrayFront(queue));
    assert(2 == QueueArraySize(queue));
    QueueArrayPop(queue);
    assert((QueueAarrayValue)(2) == QueueArrayFront(queue));
    // ���Ԫ�أ����Զ�������
    QueueArrayBreakPush(queue, (QueueAarrayValue)(3));
    assert(2 == QueueArraySize(queue));
    QueueArrayBreakPush(queue, (QueueAarrayValue)(4));
    assert(3 == QueueArraySize(queue));
    QueueArrayBreakPush(queue, (QueueAarrayValue)(5));
    assert(3 == QueueArraySize(queue));
    assert((QueueAarrayValue)(3) == QueueArrayFront(queue));
    QueueArrayPop(queue);
    assert(2 == QueueArraySize(queue));
    assert((QueueAarrayValue)(4) == QueueArrayFront(queue));
    QueueArrayPop(queue);
    assert(1 == QueueArraySize(queue));
    assert((QueueAarrayValue)(5) == QueueArrayFront(queue));
    QueueArrayPop(queue);
    assert(0 == QueueArraySize(queue));
    QueueArrayDel(queue);

    _CrtDumpMemoryLeaks();
}

#endif
