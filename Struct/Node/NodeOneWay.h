/**
 * @file
 * @brief ����ģ�飺ѭ����㣨����
 * @author JunHuanChen
 * @date 2018-01-21 21:16:49
 * @version 1.3
 * @remark �ṩ�ɶ��ο����ĵ���ѭ�����ģ�飬�ɻ��ڴ����������������������
 */
#ifndef NODE_ONE_WAY_H
#define NODE_ONE_WAY_H

/**
 * @struct node_one_way
 * @brief ����ѭ�����
 */
typedef struct node_one_way
{
    struct node_one_way *Next;	/**< ��̽��ָ��  */
}NodeOneWay;

/**
 * @brief �ƶ�����һ���
 * @param[in] Self ���ָ��
 * @return ��
 * @note �� Self ���ָ��ָ���̽��ָ��
 */
#define NodeOneWayMoveNext(Self) ((Self) = (Self)->Next)

/**
 * @brief ��ӱ�ǽ��
 * @param[in] Self ���ָ��
 * @return Դָ��
 * @note ����32λָ�����ϲ���ָ��ĩβһλֵ��
 */
#define NodeMark(Self) (Self = (NodeOneWay *) (((uintptr_t) Self) | 0x1))

/**
 * @brief �жϽ���Ƿ񱻱��
 * @param[in] Self ���ָ��
 * @return ����Ƿ��طǿ�ֵ
 * @note ����32λָ�����ϲ���ָ��ĩβһλֵ��
 */
#define NodeCheck(Self) (bool)(((uintptr_t) Self) & 0x1)

/**
 * @brief ��������
 * @param[in] Self ���ָ��
 * @return Դָ��
 * @note ����32λָ�����ϲ���ָ��ĩβһλֵ��
 */
#define NodeClean(Self) (Self = (NodeOneWay *) (((uintptr_t) Self) ^ 0x1))

/**
 * @brief ��ʼ�����
 * @param[in] Self ���ָ��
 * @return ��
 * @note �� Self ���ĺ��ָ��ָ�����׵�ַ��
 */
__inline void NodeOneWayInit(NodeOneWay *Self)
{
    Self->Next = Self;
}

/**
 * @brief ���ӽ��
 * @remark ��ָ���Ľ��֮�����ӽ�㡣
 * @param[in] Self ���ָ��
 * @param[in] Goal ���ӵĽ��ָ��
 * @return ��
 * @par ����ʾ����
 * @code
 * 	// �����������
 * 	NodeOneWay n0, n1;
 *
 * 	// ʹ�ý��֮ǰ��Գ䵱����ͷ�Ľ����г�ʼ��
 * 	NodeOneWayInit(&n0);
 *
 * 	// ��n0���֮������n1
 * 	NodeOneWayAdd(&n0, &n1);
 *
 * 	// ���ӵĽ��Ӧ����
 * 	if (n0.next == &n1 && n1.next == &n0)
 * 	{
 * 		// ���ӳɹ�
 * 	}
 * @endcode
 * @note �� Goal ���ĺ��ָ��ָ�� Self ����Self �����ָ����ָ�� Goal ��㡣
 */
__inline void NodeOneWayAdd(NodeOneWay *Self, NodeOneWay *Goal)
{
    Goal->Next = Self->Next, Self->Next = Goal;
}

/**
 * @brief ɾ�����
 * @remark ɾ��ָ�����ĺ�̽�㡣
 * @param[in] Self ���ָ��
 * @return ��
 * @par ����ʾ����
 * @code
 * 	// �����������
 * 	NodeOneWay n0, n1;
 *
 * 	// ʹ�ý��֮ǰ��Գ䵱����ͷ�Ľ����г�ʼ��
 * 	NodeOneWayInit(&n0);
 *
 * 	// ��n0���֮������n1
 * 	NodeOneWayAdd(&n0, &n1);
 *
 * 	// ɾ��n0���֮��Ľ��n1
 * 	NodeOneWayDel(&n0);
 *
 * 	// ɾ�����Ӧ����
 * 	if (n0.next == &n0)
 * 	{
 * 		// ɾ���ɹ�
 * 	}
 * @endcode
 * @note ָ�����ĺ�̽��ָ��ֱ��ָ�����̽��ĺ��ָ�롣
 */
__inline void NodeOneWayDel(NodeOneWay *Self)
{
    Self->Next = Self->Next->Next;
}

#endif // NODE_ONE_WAY_H
