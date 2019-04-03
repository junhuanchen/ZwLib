
/**
 * @file
 * @brief 队列容器：QueueList
 * @author JunHuanChen
 * @date 2018-01-22 16:03:56
 * @version 1.1
 * @remark QueueList 模板版本
 * @see QueueList
 */
#ifndef QUEUE_LIST_HPP
#define QUEUE_LIST_HPP

#include "../Node/NodeOneWay.h"
#include "../Struct.h"

#define StartupMapIteratorPair // true 启动迭代器函数

template < typename QueueValue >
class TemplateQueueList
{
    NodeOneWay *Front;		    /**< 队头指针  */
    NodeOneWay *Back;			/**< 队尾指针  */

#ifdef StartupMapIteratorPair
    NodeOneWay *End, *Pos;      // 迭代器指针
#endif

public:

    struct QueueNode
    {
        QueueValue Value;	    /**< 数据域  */
        NodeOneWay Node;	/**< 链表结点  */

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
        End = Pos = NULL;   // 初始化迭代器指针
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
                
                Back = Front = NULL; // 初始化
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
        // 链表的最后一个结点是不使用的，用作标记。
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

#define _CRTDBG_MAP_ALLOC /**< VS 提供的 malloc 内存泄漏检测宏  */
#include <crtdbg.h>
#include <memory.h>
#include <assert.h>

static void UnitTestTemplateQueueList()
{
    // 初始化测试数据
    double test[10];
    for (int i = 0; i < 10; i++) test[i] = i;
    // 初始化测试队列数据
    TemplateQueueList<double> queue;
    double *node, *bak;
    // 测试队列初始化是否成功
    assert(true == queue.New());
    // 结点入队
    assert((node = queue.Push(test[0])));
    // 判断队列内存在结点
    assert(queue.Exist());
    // 队头结点出队
    queue.Pop();
    // 判断队列为空
    assert(queue.Empty());
    // 结点入队
    assert((node = queue.Push(test[0])));
    // 备份队列结点0
    bak = node;
    // 结点入队，此时应扩充了队列。
    assert((node = queue.Push(test[1])));
    // 判断队头为先前入队的0x1
    assert(0 == memcmp(queue.GetFront(), &test[0], sizeof(test[0])));
    // 队头结点出队
    queue.Pop();
    // 判断此时队头为后来入队的0x2
    assert(0 == memcmp(queue.GetFront(), &test[1], sizeof(test[1])));
    // 结点入队，此时应占用0x2入队时扩充的结点。
    assert((node = queue.Push(test[2])));
    // 结点入队，此时应占用了0x1入队时（bak）的结点指针，因为其已出队，否则当前容量不足导致扩充。
    assert((node = queue.Push(test[3])));
    // 然后判断结点是否循环使用了
    assert(bak == node);
    // 队头结点出队
    queue.Pop();
    assert(0 == memcmp(queue.GetFront(), &test[2], sizeof(test[2])));
    // 队头结点出队
    queue.Pop();
    assert(0 == memcmp(queue.GetFront(), &test[3], sizeof(test[3])));
    // 队头结点出队
    queue.Pop();
    // 此时队列应为空
    assert(queue.Empty());
    // 对队列容量重新调整
    queue.ReNew();
    // 此时队列中应该只剩一个占位标记结点
    assert(queue.Full());
    queue.Del();
    // 检测内存是否泄漏，请放在程序最终执行位置而不是函数
    _CrtDumpMemoryLeaks();
}
#endif
