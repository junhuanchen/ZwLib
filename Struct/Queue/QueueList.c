
#include "QueueList.h"

/**
 * @brief 初始化队列结点的数据域
 * @param[in] node 队列结点指针
 * @return 无
 * @see QueueListValue
 */
static void QueueListValueInit(QueueListNode *Self)
{
    Self->Value = NULL;
}

/**
 * @brief 初始化队列结点
 * @param[in] Qnode 队列结点指针
 * @return 无
 */
static void QueueListNodeInit(QueueListNode *Self)
{
    QueueListValueInit(Self), NodeOneWayInit(&Self->Node);
}

/**
 * @brief 初始化队列
 * @remark 为队列置入占位队列结点，占位队列结点的用途查看参见。
 * @param[in] Queue 队列指针
 * @param[in] Qnode 队列结点指针
 * @return 无
 * @see queue_list_full queue_list_empty
 */
static void QueueListSetSeatNode(QueueList *Self, QueueListNode *SeatNode)
{
    Self->Back = Self->Front = &SeatNode->Node;
}

#define QueueListGetNode(StructNode) StructOf(StructNode, QueueListNode, Node)

QueueList *QueueListNew(queue_list_new New, queue_list_del Del)
{
    QueueList * queue = (QueueList *)New(sizeof(*queue));
    if (queue)
    {
        queue->New = New, queue->Del = Del;
        QueueListNode * node = (QueueListNode *)New(sizeof(*node));
        if (node)
        {
            QueueListNodeInit(node);
            QueueListSetSeatNode(queue, node);
            return queue;
        }
        queue->Del(queue);
    }
    return NULL;
}

void QueueListReNew(QueueList *Self)
{
    NodeOneWay *back = Self->Back->Next;
    Self->Back->Next = Self->Front;
    while (Self->Front != back)
    {
        NodeOneWay *bak = back;
        NodeOneWayMoveNext(back);
        Self->Del(StructOf(bak, QueueListNode, Node));
    }
}

void QueueListDel(QueueList *Self)
{
    if (NULL != Self->Front)
    {
        Self->Back = Self->Front;
        do
        {
            NodeOneWay *bak = Self->Front;
            NodeOneWayMoveNext(Self->Front);
            Self->Del(StructOf(bak, QueueListNode, Node));
        } while (NULL != Self->Front && Self->Front != Self->Back);
        Self->Del(Self);
    }
}

uint32_t QueueListSize(QueueList *Self, bool All)
{
    int sum = All ? 1 : 0;
    NodeOneWay *back = Self->Back->Next;
    while (Self->Back != back)
    {
        if (All || NULL != QueueListGetNode(back)->Value)
        {
            sum++;
        }
        NodeOneWayMoveNext(back);
    }
    return sum;
}

QueueListValue * QueueListFront(QueueList *Self)
{
    return QueueListGetNode(Self->Front)->Value;
}

bool QueueListEmpty(QueueList *Self)
{
    return (Self->Front == Self->Back);
}

bool QueueListExist(QueueList *Self)
{
    return (Self->Front != Self->Back);
}

bool QueueListFull(QueueList *Self)
{
    return (Self->Front == Self->Back->Next);
}

void QueueListPop(QueueList *Self)
{
    QueueListValueInit(QueueListGetNode(Self->Front));
    NodeOneWayMoveNext(Self->Front);
}

void QueueListPushNode(QueueList *Self, QueueListNode *PushNode)
{
    NodeOneWayAdd(Self->Back, &PushNode->Node);
}

void QueueListPushValue(QueueList *Self, QueueListValue Value)
{
    QueueListGetNode(Self->Back)->Value = Value;
    NodeOneWayMoveNext(Self->Back);
}

QueueListValue *QueueListPush(QueueList *Self, QueueListValue Value)
{
    QueueListNode *bak = QueueListGetNode(Self->Back);
    if (QueueListFull(Self))
    {
        QueueListNode * node = (QueueListNode *)Self->New(sizeof(*node));
        if ( node )
        {
            QueueListValueInit(node);
            QueueListPushNode(Self, node);
        }
        else
        {
            return NULL;
        }
    }
    QueueListPushValue(Self, Value);
    return bak->Value;
}

#ifdef UNIT_TEST

#define _CRTDBG_MAP_ALLOC /**< VS 提供的 malloc 内存泄漏检测宏  */
#include <crtdbg.h>
#include <stdlib.h>
#include <assert.h>

/**
 * @brief Queue 单元测试
 * @remark 单元测试中采用assert（断言）处理测试结果。
 */
void UnitTestQueueList(void)
{
    // 检测内存是否泄漏
    _CrtDumpMemoryLeaks( );
    QueueList *queue = QueueListNew(malloc, free);
    QueueListValue *value;
    // 测试队列是否循环，是否满足队列
    assert(queue);
    // 确认队列为空
    // 检查队列结点数目
    assert(0 == QueueListSize(queue, 0));
    assert(1 == QueueListSize(queue, 1));
    // 结点入队
    assert((QueueListValue)0x1 == (value = QueueListPush(queue, (QueueListValue)0x1)));
    // 判断队头结点指针是否获得的结点指针相等。
    assert(QueueListFront(queue) == value);
    // 判断队列内存在结点且大小为一
    assert(true == QueueListExist(queue));
    // 检查队列结点数目
    assert(1 == QueueListSize(queue, 0));
    assert(2 == QueueListSize(queue, 1));
    // 队头结点出队
    QueueListPop(queue);
    // 判断队列为空
    assert(QueueListEmpty(queue));
    // 检查队列结点数目
    assert(0 == QueueListSize(queue, 0));
    assert(2 == QueueListSize(queue, 1));
    // 结点入队
    assert((value = QueueListPush(queue, (QueueListValue)0x1)));
    // 结点入队，此时应扩充了队列。
    assert((value = QueueListPush(queue, (QueueListValue)0x2)));
    // 检查队列结点数目
    assert(2 == QueueListSize(queue, 0));
    assert(3 == QueueListSize(queue, 1));
    // 判断队头为先前入队的0x1
    assert(QueueListFront(queue) == (QueueListValue)0x1);
    // 队头结点出队
    QueueListPop(queue);
    // 判断此时队头为后来入队的0x2
    assert(QueueListFront(queue) == (QueueListValue)0x2);
    // 结点入队，此时应占用0x2入队时扩充的结点。
    assert((value = QueueListPush(queue, (QueueListValue)0x3)));
    // 结点入队，此时应占用了0x1入队时（bak）的结点指针，因为其已出队，否则当前容量不足导致扩充。
    assert((value = QueueListPush(queue, (QueueListValue)0x4)));
    // 检查队列结点数目
    assert(3 == QueueListSize(queue, 0));
    assert(4 == QueueListSize(queue, 1));
    // 队头结点出队
    QueueListPop(queue);
    assert(QueueListFront(queue) == (QueueListValue)0x3);
    // 检查队列结点数目
    assert(2 == QueueListSize(queue, 0));
    assert(4 == QueueListSize(queue, 1));
    // 队头结点出队
    QueueListPop(queue);
    assert(QueueListFront(queue) == (QueueListValue)0x4);
    // 检查队列结点数目
    assert(1 == QueueListSize(queue, 0));
    assert(4 == QueueListSize(queue, 1));
    // 数据空了，但缓冲区不满。
    assert(false == QueueListFull(queue));
    // 队头结点出队
    QueueListPop(queue);
    // 此时队列应为空
    assert(true == QueueListEmpty(queue));
    // 对队列容量重新调整
    QueueListReNew(queue);
    // 检查队列结点数目
    assert(0 == QueueListSize(queue, 0));
    // 此时队列中应该只剩一个占位结点
    assert(1 == QueueListSize(queue, 1));
    // 清理了后数据空了，缓冲区也满了。
    assert(true == QueueListFull(queue));
    // 此时队列初始化状态
    QueueListDel(queue);
    // 检测内存是否泄漏
    _CrtDumpMemoryLeaks();
}

#endif 
