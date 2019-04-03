/**
 * @file
 * @brief 链表模块：循环结点（单向）
 * @author JunHuanChen
 * @date 2018-01-21 21:16:49
 * @version 1.3
 * @remark 提供可二次开发的单向循环结点模块，可基于此制作其他容器所需的链表。
 */
#ifndef NODE_ONE_WAY_H
#define NODE_ONE_WAY_H

/**
 * @struct node_one_way
 * @brief 单向循环结点
 */
typedef struct node_one_way
{
    struct node_one_way *Next;	/**< 后继结点指针  */
}NodeOneWay;

/**
 * @brief 移动至下一结点
 * @param[in] Self 结点指针
 * @return 无
 * @note 将 Self 结点指针指向后继结点指针
 */
#define NodeOneWayMoveNext(Self) ((Self) = (Self)->Next)

/**
 * @brief 添加标记结点
 * @param[in] Self 结点指针
 * @return 源指针
 * @note 用于32位指针以上操作指针末尾一位值。
 */
#define NodeMark(Self) (Self = (NodeOneWay *) (((uintptr_t) Self) | 0x1))

/**
 * @brief 判断结点是否被标记
 * @param[in] Self 结点指针
 * @return 被标记返回非空值
 * @note 用于32位指针以上操作指针末尾一位值。
 */
#define NodeCheck(Self) (bool)(((uintptr_t) Self) & 0x1)

/**
 * @brief 清理结点标记
 * @param[in] Self 结点指针
 * @return 源指针
 * @note 用于32位指针以上操作指针末尾一位值。
 */
#define NodeClean(Self) (Self = (NodeOneWay *) (((uintptr_t) Self) ^ 0x1))

/**
 * @brief 初始化结点
 * @param[in] Self 结点指针
 * @return 无
 * @note 将 Self 结点的后继指针指向其首地址。
 */
__inline void NodeOneWayInit(NodeOneWay *Self)
{
    Self->Next = Self;
}

/**
 * @brief 链接结点
 * @remark 在指定的结点之后链接结点。
 * @param[in] Self 结点指针
 * @param[in] Goal 链接的结点指针
 * @return 无
 * @par 代码示例：
 * @code
 * 	// 声明两个结点
 * 	NodeOneWay n0, n1;
 *
 * 	// 使用结点之前须对充当链表头的结点进行初始化
 * 	NodeOneWayInit(&n0);
 *
 * 	// 在n0结点之后链接n1
 * 	NodeOneWayAdd(&n0, &n1);
 *
 * 	// 链接的结果应满足
 * 	if (n0.next == &n1 && n1.next == &n0)
 * 	{
 * 		// 链接成功
 * 	}
 * @endcode
 * @note 将 Goal 结点的后继指针指向 Self 结点后，Self 结点后继指针再指向 Goal 结点。
 */
__inline void NodeOneWayAdd(NodeOneWay *Self, NodeOneWay *Goal)
{
    Goal->Next = Self->Next, Self->Next = Goal;
}

/**
 * @brief 删除结点
 * @remark 删除指定结点的后继结点。
 * @param[in] Self 结点指针
 * @return 无
 * @par 代码示例：
 * @code
 * 	// 声明两个结点
 * 	NodeOneWay n0, n1;
 *
 * 	// 使用结点之前须对充当链表头的结点进行初始化
 * 	NodeOneWayInit(&n0);
 *
 * 	// 在n0结点之后链接n1
 * 	NodeOneWayAdd(&n0, &n1);
 *
 * 	// 删除n0结点之后的结点n1
 * 	NodeOneWayDel(&n0);
 *
 * 	// 删除结果应满足
 * 	if (n0.next == &n0)
 * 	{
 * 		// 删除成功
 * 	}
 * @endcode
 * @note 指定结点的后继结点指针直接指向其后继结点的后继指针。
 */
__inline void NodeOneWayDel(NodeOneWay *Self)
{
    Self->Next = Self->Next->Next;
}

#endif // NODE_ONE_WAY_H
