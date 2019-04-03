
/**
 * @file
 * @brief Listģ�飺��������
 * @author JunHuanChen
 * @date 2018-01-21 23:07:43
 * @version 1.1
 * @remark ���� node_one_way ���ο����ĵ���ѭ������
 */
#ifndef LIST_ONE_WAY_H
#define LIST_ONE_WAY_H

#include "../Node/NodeOneWay.h"

/**
 * @struct list_one_way
 * @brief ������ƽ������
 */
typedef struct list_one_way
{
    NodeOneWay Head;	/**< ����ͷ��㣬���������㣬������������ʼ����ֹ��ǡ� */
    NodeOneWay *Tail;	/**< ����βָ�룬��ָ������β�˵Ľ�㡣 */
} ListOneWay;

/**
 * @brief �ж������Ƿ�Ϊ��
 * @param[in] Self ����ͷ���ָ��
 * @return ����Ϊ�շ��� true �����򷵻� false��
 * @par ����ʾ����
 * @code
 * // ����һ������
 * ListOneWay list;
 *
 * // ʹ������ǰ���ʼ������
 * ListOneWayInit(&list);
 *
 * // Ȼ���ж������Ƿ�Ϊ��
 * if(true == ListOneWayempty(&list.Head))
 * {
 * 	// after operator
 * }
 * @endcode
 */
#define ListOneWayEmpty(Self) (bool)((Self)->Next == (Self))

/**
 * @brief ������������ͨ�ã�
 * @remark �ṩ��������ÿ�����Ĳ��������Ӵ���ɶ��ԡ�
 * @param[in] Pos ������ʼָ��
 * @param[out] Pos ����������Ľ��ָ��
 * @param[in] End ������ָֹ��
 * @return ��
 * @par ����ʾ����
 * @code
 * // ����һ������
 * ListOneWay list;
 *
 * // ʹ������ǰ���ʼ������
 * ListOneWayinit(&list);
 *
 * // Ȼ���������ӽ��
 * NodeOneWay node_0, node_1, node_2;
 * ListOneWayadd(&list, &node_0), ListOneWayadd(&list, &node_1), ListOneWayadd(&list, &node_2);
 *
 * // ������Ҫ�ĵ�������ָ��
 * NodeOneWay *end, *pos;
 * ListOneWayforeach(pos, end)
 * {
 * 	// pos ���ε��� &node_0 &node_1 &node_2
 * }
 *
 * @endcode
 */
#define ListOneWayForeach(Pos, End) \
    for (; Pos != End; Pos = Pos->Next)

/**
 * @brief ������������֧�ְ�ȫ������
 * @param[in] pos ������ʼָ��
 * @param[in] end ������ָֹ��
 * @param[in] bak �ṩһ�����������ݵı���
 * @param[out] bak �洢 pos ���ָ��ĺ�̽��ָ��
 * @return ��
 * @see ListOneWayForeach
 */
#define ListOneWayForeachSafe(Pos, End, Bak) \
  for (Bak = Pos->Next; Pos != End; Pos = Bak, Bak = Pos->Next)

/**
 * @brief ������������֧�ִ洢ǰ����ָ�룩
 * @param[in] Pos ������ʼָ��
 * @param[out] Pos ����������Ľ��ָ��
 * @param[in] End ������ָֹ��
 * @param[in] Prev �ṩһ�����������ݵı���
 * @param[out] Prev �洢 pos ���ָ���ǰ����ָ��
 * @return ��
 * @see ListOneWayForeach
 */
#define ListOneWayForeachPrev(Pos, End, Prev) \
    for (Prev = End; Pos != End; Prev = Pos, Pos = Pos->Next)

/**
 * @brief ��ʼ������
 * @param[in] Self ����ָ��
 * @return ��
 * @note ��ʼ������ͷ��㣬��������βָ��ָ������ͷ���ĺ��ָ�롣
 */
__inline void ListOneWayInit(ListOneWay *Self)
{
    NodeOneWayInit(&Self->Head);
    Self->Tail = Self->Head.Next;
}

/**
 * @brief ����������
 * @remark �������ĩβ���ӽ�㡣
 * @param[in] Self ����ָ��
 * @param[in] Goal �����ӵĽ��ָ��
 * @return ��
 * @par ����ʾ����
 * @code
 * // ����һ������
 * ListOneWay list;
 *
 * // ʹ������ǰ���ʼ������
 * ListOneWayinit(&list);
 *
 * // Ȼ���������ӽ��
 * NodeOneWay node_0;
 * ListOneWayadd(&list, &node_0);
 *
 * if(list.head.next == &node_0)
 * {
 * 	// ��ӳɹ�
 * }
 * @endcode
 * @note ������ָ���βָ�봦���� Goal ��㡣
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
 * @brief list_one_way ��Ԫ����
 * @remark ��Ԫ�����в���assert�����ԣ�������Խ����
 */
static void UnitTestListOneWay()
{
    ListOneWay list;
    ListOneWayInit(&list);
    // ���������ʼ��
    assert(list.Head.Next == &list.Head && list.Head.Next == list.Tail);
    // ���������Ƿ�Ϊ��
    assert(1 == ListOneWayEmpty(&list.Head));
    NodeOneWay node_0, node_1, node_2;
    NodeOneWayInit(&node_0), NodeOneWayInit(&node_1), NodeOneWayInit(&node_1);
    // ����������λԪ���Ƿ���ӳɹ�
    ListOneWayAdd(&list, &node_0);
    assert(list.Head.Next == &node_0);
    // �ٴβ��������Ƿ�Ϊ��
    assert(0 == ListOneWayEmpty(&list.Head));
    // ������ӵڶ��������
    ListOneWayAdd(&list, &node_1), ListOneWayAdd(&list, &node_2);
    NodeOneWay *end = NULL, *pos = NULL, *bak = NULL, *prev = NULL;
    // ��������������ʵ���ȷ��
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
        pos = 0; // �޸� pos ָ��
    }
}

#endif 
