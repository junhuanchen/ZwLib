
/**
 * @file
 * @brief List模块：单向链表
 * @author JunHuanChen
 * @date 2018-01-21 23:07:43
 * @version 1.1
 * @remark 基于 node_one_way 二次开发的单向循环链表。
 */
#ifndef LIST_ONE_WAY_H
#define LIST_ONE_WAY_H

#include "../Node/NodeOneWay.h"

/**
 * @struct list_one_way
 * @brief 单向带哑结点链表
 */
typedef struct list_one_way
{
    NodeOneWay Head;	/**< 链表头结点，无数据域结点，用作遍历的起始和终止标记。 */
    NodeOneWay *Tail;	/**< 链表尾指针，须指向链表尾端的结点。 */
} ListOneWay;

/**
 * @brief 判断链表是否为空
 * @param[in] Self 链表头结点指针
 * @return 链表为空返回 true ，否则返回 false。
 * @par 代码示例：
 * @code
 * // 声明一个链表
 * ListOneWay list;
 *
 * // 使用链表前须初始化链表
 * ListOneWayInit(&list);
 *
 * // 然后判断链表是否为空
 * if(true == ListOneWayempty(&list.Head))
 * {
 * 	// after operator
 * }
 * @endcode
 */
#define ListOneWayEmpty(Self) (bool)((Self)->Next == (Self))

/**
 * @brief 迭代访问链表（通用）
 * @remark 提供访问链表每个结点的操作，增加代码可读性。
 * @param[in] Pos 链表起始指针
 * @param[out] Pos 链表迭代到的结点指针
 * @param[in] End 链表终止指针
 * @return 无
 * @par 代码示例：
 * @code
 * // 声明一个链表
 * ListOneWay list;
 *
 * // 使用链表前须初始化链表
 * ListOneWayinit(&list);
 *
 * // 然后给链表添加结点
 * NodeOneWay node_0, node_1, node_2;
 * ListOneWayadd(&list, &node_0), ListOneWayadd(&list, &node_1), ListOneWayadd(&list, &node_2);
 *
 * // 声明必要的迭代访问指针
 * NodeOneWay *end, *pos;
 * ListOneWayforeach(pos, end)
 * {
 * 	// pos 依次等于 &node_0 &node_1 &node_2
 * }
 *
 * @endcode
 */
#define ListOneWayForeach(Pos, End) \
    for (; Pos != End; Pos = Pos->Next)

/**
 * @brief 迭代访问链表（支持安全迭代）
 * @param[in] pos 链表起始指针
 * @param[in] end 链表终止指针
 * @param[in] bak 提供一个保存结点数据的变量
 * @param[out] bak 存储 pos 结点指针的后继结点指针
 * @return 无
 * @see ListOneWayForeach
 */
#define ListOneWayForeachSafe(Pos, End, Bak) \
  for (Bak = Pos->Next; Pos != End; Pos = Bak, Bak = Pos->Next)

/**
 * @brief 迭代访问链表（支持存储前向结点指针）
 * @param[in] Pos 链表起始指针
 * @param[out] Pos 链表迭代到的结点指针
 * @param[in] End 链表终止指针
 * @param[in] Prev 提供一个保存结点数据的变量
 * @param[out] Prev 存储 pos 结点指针的前向结点指针
 * @return 无
 * @see ListOneWayForeach
 */
#define ListOneWayForeachPrev(Pos, End, Prev) \
    for (Prev = End; Pos != End; Prev = Pos, Pos = Pos->Next)

/**
 * @brief 初始化链表
 * @param[in] Self 链表指针
 * @return 无
 * @note 初始化链表头结点，并将链表尾指针指向链表头结点的后继指针。
 */
__inline void ListOneWayInit(ListOneWay *Self)
{
    NodeOneWayInit(&Self->Head);
    Self->Tail = Self->Head.Next;
}

/**
 * @brief 增加链表结点
 * @remark 在链表的末尾增加结点。
 * @param[in] Self 链表指针
 * @param[in] Goal 待增加的结点指针
 * @return 无
 * @par 代码示例：
 * @code
 * // 声明一个链表
 * ListOneWay list;
 *
 * // 使用链表前须初始化链表
 * ListOneWayinit(&list);
 *
 * // 然后给链表添加结点
 * NodeOneWay node_0;
 * ListOneWayadd(&list, &node_0);
 *
 * if(list.head.next == &node_0)
 * {
 * 	// 添加成功
 * }
 * @endcode
 * @note 在链表指针的尾指针处链接 Goal 结点。
 *see node_one_way_add
 */
__inline void ListOneWayTailAdd(ListOneWay *Self, NodeOneWay *Goal)
{
    NodeOneWayAdd(Self->Tail, Goal);
    Self->Tail = Goal;
}

#endif

#ifdef UNIT_TEST

#include <assert.h>

/**
 * @brief list_one_way 单元测试
 * @remark 单元测试中采用assert（断言）处理测试结果。
 */
static void UnitTestListOneWay()
{
    ListOneWay list;
    ListOneWayInit(&list);
    // 测试链表初始化
    assert(list.Head.Next == &list.Head && list.Head.Next == list.Tail);
    // 测试链表是否为空
    assert(1 == ListOneWayEmpty(&list.Head));
    NodeOneWay node_0, node_1, node_2;
    NodeOneWayInit(&node_0), NodeOneWayInit(&node_1), NodeOneWayInit(&node_1);
    // 测试链表首位元素是否添加成功
    ListOneWayAdd(&list, &node_0);
    assert(list.Head.Next == &node_0);
    // 再次测试链表是否为空
    assert(0 == ListOneWayEmpty(&list.Head));
    // 依次添加第二第三结点
    ListOneWayAdd(&list, &node_1), ListOneWayAdd(&list, &node_2);
    NodeOneWay *end = NULL, *pos = NULL, *bak = NULL, *prev = NULL;
    // 测试链表迭代访问的正确性
    end = &list.Head, pos = end->Next;
    ListOneWayForeach(pos, end)
    {
        assert(pos == &node_0 || pos == &node_1 || pos == &node_2);
    }

    end = &list.Head, pos = end->Next;
    ListOneWayForeachPrev(pos, prev, end)
    {
        assert(pos == &node_0 || pos == &node_1 || pos == &node_2);
        assert(prev == end || prev == &node_0 || prev == &node_1);
    }

    end = &list.Head, pos = end->Next;
    ListOneWayForeachSafe(pos, bak, end)
    {
        assert(pos == &node_0 || pos == &node_1 || pos == &node_2);
        pos = 0; // 修改 pos 指针
    }
}

#endif 
