
/**
 * @file
 * @brief ����������QueueList
 * @author JunHuanChen
 * @date 2018-01-22 16:03:56
 * @version 1.1
 * @remark QueueList ģ��汾
 * @see QueueList
 */
#ifndef QUEUE_LIST_HPP
#define QUEUE_LIST_HPP

#include "../Node/NodeOneWay.h"
#include "../Struct.h"

#define StartupMapIteratorPair // true ��������������

template < typename QueueValue >
class TemplateQueueList
{
    NodeOneWay *Front;		    /**< ��ͷָ��  */
    NodeOneWay *Back;			/**< ��βָ��  */

#ifdef StartupMapIteratorPair
    NodeOneWay *End, *Pos;      // ������ָ��
#endif

public:

    struct QueueNode
    {
        QueueValue Value;	    /**< ������  */
        NodeOneWay Node;	/**< ������  */

        QueueNode()
        {
            NodeOneWayInit(&Node);
        }
    };

private:

    inline QueueNode * GetNode(NodeOneWay * Node)
    {
        return StructOf(Node, QueueNode, Node);
    }

public:

    TemplateQueueList() :Back(NULL), Front(NULL)
    {
        ;
    }

    ~TemplateQueueList()
    {
        Del();
    }

    bool New()
    {
#ifdef StartupMapIteratorPair
        End = Pos = NULL;   // ��ʼ��������ָ��
#endif
        if (NULL == Back && NULL == Front)
        {
            QueueNode *node = new QueueNode;
            if (NULL != node)
            {
                Back = Front = &node->Node;
                return true;
            }
        }
        return false;
    }

    void ReNew()
    {
        if (NULL != Back && NULL != Front)
        {
            NodeOneWay *bak, *back = Back->Next;
            Back->Next = Front;
            while (Front != back)
            {
                bak = back, NodeOneWayMoveNext(back);
                delete GetNode(bak);
            }
        }
    }

    void Del()
    {
        if (NULL != Back && NULL != Front)
        {
            if (Front)
            {
                Back = Front;
                do
                {
                    NodeOneWay *bak = Front;
                    NodeOneWayMoveNext(Front);
                    delete GetNode(bak);
                } while (Front != Back);
                
                Back = Front = NULL; // ��ʼ��
            }
        }
    }

    inline bool Exist()
    {
        return Front != Back;
    }

    inline bool Empty()
    {
        return Front == Back;
    }

    inline bool Full()
    {
        return Front == Back->Next;
    }

    QueueValue *GetFront()
    {
        return &GetNode(Front)->Value;
    }

    QueueValue *GetBack()
    {
        return &GetNode(Back)->Value;
    }

    inline void Pop()
    {
        NodeOneWayMoveNext(Front);
    }

    QueueValue *Push(QueueValue & Value)
    {
        QueueNode * bak = GetNode(Back);
        if (Full())
        {
            QueueNode * node = new QueueNode;
            if (node)
            {
                NodeOneWayAdd(Back, &node->Node);
            }
            else
            {
                return NULL;
            }
        }
        bak->Value = Value;
        NodeOneWayMoveNext(Back);
        return &bak->Value;
    }

    QueueValue *Get()
    {
        QueueNode * bak = GetNode(Back);
        if (Full())
        {
            QueueNode * node = new QueueNode;
            if (node)
            {
                NodeOneWayAdd(Back, &node->Node);
            }
            else
            {
                return NULL;
            }
        }
        NodeOneWayMoveNext(Back);
        return &bak->Value;
    }

#ifdef StartupMapIteratorPair
    QueueValue *Iterator()
    {
        // ��������һ������ǲ�ʹ�õģ�������ǡ�
        while (End == Pos)
        {
            End = Back, Pos = End->Next;
        }
        QueueNode *result = GetNode(Pos);
        NodeOneWayMoveNext(Pos);
        return &result->Value;
    }
#endif

};

#endif // QUEUE_LIST_HPP

#ifdef UNIT_TEST

#define _CRTDBG_MAP_ALLOC /**< VS �ṩ�� malloc �ڴ�й©����  */
#include <crtdbg.h>
#include <memory.h>
#include <assert.h>

static void UnitTestTemplateQueueList()
{
    // ��ʼ����������
    double test[10];
    for (int i = 0; i < 10; i++) test[i] = i;
    // ��ʼ�����Զ�������
    TemplateQueueList<double> queue;
    double *node, *bak;
    // ���Զ��г�ʼ���Ƿ�ɹ�
    assert(true == queue.New());
    // ������
    assert((node = queue.Push(test[0])));
    // �ж϶����ڴ��ڽ��
    assert(queue.Exist());
    // ��ͷ������
    queue.Pop();
    // �ж϶���Ϊ��
    assert(queue.Empty());
    // ������
    assert((node = queue.Push(test[0])));
    // ���ݶ��н��0
    bak = node;
    // �����ӣ���ʱӦ�����˶��С�
    assert((node = queue.Push(test[1])));
    // �ж϶�ͷΪ��ǰ��ӵ�0x1
    assert(0 == memcmp(queue.GetFront(), &test[0], sizeof(test[0])));
    // ��ͷ������
    queue.Pop();
    // �жϴ�ʱ��ͷΪ������ӵ�0x2
    assert(0 == memcmp(queue.GetFront(), &test[1], sizeof(test[1])));
    // �����ӣ���ʱӦռ��0x2���ʱ����Ľ�㡣
    assert((node = queue.Push(test[2])));
    // �����ӣ���ʱӦռ����0x1���ʱ��bak���Ľ��ָ�룬��Ϊ���ѳ��ӣ�����ǰ�������㵼�����䡣
    assert((node = queue.Push(test[3])));
    // Ȼ���жϽ���Ƿ�ѭ��ʹ����
    assert(bak == node);
    // ��ͷ������
    queue.Pop();
    assert(0 == memcmp(queue.GetFront(), &test[2], sizeof(test[2])));
    // ��ͷ������
    queue.Pop();
    assert(0 == memcmp(queue.GetFront(), &test[3], sizeof(test[3])));
    // ��ͷ������
    queue.Pop();
    // ��ʱ����ӦΪ��
    assert(queue.Empty());
    // �Զ����������µ���
    queue.ReNew();
    // ��ʱ������Ӧ��ֻʣһ��ռλ��ǽ��
    assert(queue.Full());
    queue.Del();
    // ����ڴ��Ƿ�й©������ڳ�������ִ��λ�ö����Ǻ���
    _CrtDumpMemoryLeaks();
}
#endif
