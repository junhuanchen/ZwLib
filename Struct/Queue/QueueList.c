
#include "QueueList.h"

/**
 * @brief ��ʼ�����н���������
 * @param[in] node ���н��ָ��
 * @return ��
 * @see QueueListValue
 */
static void QueueListValueInit(QueueListNode *Self)
{
    Self->Value = NULL;
}

/**
 * @brief ��ʼ�����н��
 * @param[in] Qnode ���н��ָ��
 * @return ��
 */
static void QueueListNodeInit(QueueListNode *Self)
{
    QueueListValueInit(Self), NodeOneWayInit(&Self->Node);
}

/**
 * @brief ��ʼ������
 * @remark Ϊ��������ռλ���н�㣬ռλ���н�����;�鿴�μ���
 * @param[in] Queue ����ָ��
 * @param[in] Qnode ���н��ָ��
 * @return ��
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

#define _CRTDBG_MAP_ALLOC /**< VS �ṩ�� malloc �ڴ�й©����  */
#include <crtdbg.h>
#include <stdlib.h>
#include <assert.h>

/**
 * @brief Queue ��Ԫ����
 * @remark ��Ԫ�����в���assert�����ԣ�������Խ����
 */
void UnitTestQueueList(void)
{
    // ����ڴ��Ƿ�й©
    _CrtDumpMemoryLeaks( );
    QueueList *queue = QueueListNew(malloc, free);
    QueueListValue *value;
    // ���Զ����Ƿ�ѭ�����Ƿ��������
    assert(queue);
    // ȷ�϶���Ϊ��
    // �����н����Ŀ
    assert(0 == QueueListSize(queue, 0));
    assert(1 == QueueListSize(queue, 1));
    // ������
    assert((QueueListValue)0x1 == (value = QueueListPush(queue, (QueueListValue)0x1)));
    // �ж϶�ͷ���ָ���Ƿ��õĽ��ָ����ȡ�
    assert(QueueListFront(queue) == value);
    // �ж϶����ڴ��ڽ���Ҵ�СΪһ
    assert(true == QueueListExist(queue));
    // �����н����Ŀ
    assert(1 == QueueListSize(queue, 0));
    assert(2 == QueueListSize(queue, 1));
    // ��ͷ������
    QueueListPop(queue);
    // �ж϶���Ϊ��
    assert(QueueListEmpty(queue));
    // �����н����Ŀ
    assert(0 == QueueListSize(queue, 0));
    assert(2 == QueueListSize(queue, 1));
    // ������
    assert((value = QueueListPush(queue, (QueueListValue)0x1)));
    // �����ӣ���ʱӦ�����˶��С�
    assert((value = QueueListPush(queue, (QueueListValue)0x2)));
    // �����н����Ŀ
    assert(2 == QueueListSize(queue, 0));
    assert(3 == QueueListSize(queue, 1));
    // �ж϶�ͷΪ��ǰ��ӵ�0x1
    assert(QueueListFront(queue) == (QueueListValue)0x1);
    // ��ͷ������
    QueueListPop(queue);
    // �жϴ�ʱ��ͷΪ������ӵ�0x2
    assert(QueueListFront(queue) == (QueueListValue)0x2);
    // �����ӣ���ʱӦռ��0x2���ʱ����Ľ�㡣
    assert((value = QueueListPush(queue, (QueueListValue)0x3)));
    // �����ӣ���ʱӦռ����0x1���ʱ��bak���Ľ��ָ�룬��Ϊ���ѳ��ӣ�����ǰ�������㵼�����䡣
    assert((value = QueueListPush(queue, (QueueListValue)0x4)));
    // �����н����Ŀ
    assert(3 == QueueListSize(queue, 0));
    assert(4 == QueueListSize(queue, 1));
    // ��ͷ������
    QueueListPop(queue);
    assert(QueueListFront(queue) == (QueueListValue)0x3);
    // �����н����Ŀ
    assert(2 == QueueListSize(queue, 0));
    assert(4 == QueueListSize(queue, 1));
    // ��ͷ������
    QueueListPop(queue);
    assert(QueueListFront(queue) == (QueueListValue)0x4);
    // �����н����Ŀ
    assert(1 == QueueListSize(queue, 0));
    assert(4 == QueueListSize(queue, 1));
    // ���ݿ��ˣ���������������
    assert(false == QueueListFull(queue));
    // ��ͷ������
    QueueListPop(queue);
    // ��ʱ����ӦΪ��
    assert(true == QueueListEmpty(queue));
    // �Զ����������µ���
    QueueListReNew(queue);
    // �����н����Ŀ
    assert(0 == QueueListSize(queue, 0));
    // ��ʱ������Ӧ��ֻʣһ��ռλ���
    assert(1 == QueueListSize(queue, 1));
    // �����˺����ݿ��ˣ�������Ҳ���ˡ�
    assert(true == QueueListFull(queue));
    // ��ʱ���г�ʼ��״̬
    QueueListDel(queue);
    // ����ڴ��Ƿ�й©
    _CrtDumpMemoryLeaks();
}

#endif 
