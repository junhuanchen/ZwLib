/**  
 * @file
 * @brief 队列容器：QueueList
 * @author JunHuanChen
 * @date 2018-01-22 16:04:34
 * @version 1.1
 * @remark
 * 队列是一种特殊的线性表，特殊之处在于它只允许在表的前端（front）进行删除操作，
 * 而在表的后端（rear）进行插入操作，进行插入操作的端称为队尾，进行删除操作的端称为队头。
 * 这是存储空指针类型数据的单循环队列，仅有入队和出队之间存在并行操作。
 */
#ifndef QUEUE_LIST_H
#define QUEUE_LIST_H

#include "../Node/NodeOneWay.h"
#include "../Struct.h"

typedef void *QueueListValue;  /** < 队列结点的数据域指针  */

typedef void *(*queue_list_new)(unsigned int); /** < 内存申请函数声明  */

typedef void(*queue_list_del)(void *); /** < 内存释放函数声明  */

/** 
 * @struct queue_list_node
 * @brief 队列结点
 * @remark 这是一个存储指针值域的队列结点结构。
 */
typedef struct queue_list_node
{
    QueueListValue Value;	    /** < 数据域  */
    NodeOneWay Node;    /** < 链表结点  */
}QueueListNode;

/** 
 * @struct queue_list
 * @brief 队列
 */
typedef struct queue_list
{
    NodeOneWay *Front;	    /** < 队头指针  */
    NodeOneWay *Back;	    /** < 队尾指针  */ 
    queue_list_new New;     /** < 申请内存函数  */
    queue_list_del Del;     /** < 释放内存函数  */
}QueueList;

/** 
 * @brief 获取队列队头指针
 * @param[in] Self 队列指针
 * @return 返回队头结点值指针
 */
QueueListValue * QueueListFront(QueueList *Self);

/** 
 * @brief 判断队列是否存在元素
 * @remark QueueListPop 前须判断。
 * @param[in] Self 队列指针
 * @return 存在元素返回1，否则返回0。
 */
bool QueueListExist(QueueList *Self);

/** 
 * @brief 判断队列是否为空
 * @param[in] Self 队列指针
 * @return 队列为空返回1，否则返回0。
 * @note 队头和队尾指针相等表示不存在队列结点，这将区分 QueueListFull。
 * @see QueueListFull
 */
bool QueueListEmpty(QueueList *Self);

/** 
 * @brief 判断队列是否已满
 * @param[in] Self 队列指针
 * @return 队列为满返回1，否则返回0。
 * @note 由于队列中存在一个占位结点(可循环使用但又用作队列已满标记），故队头指针等于占位结点（尾指针的后继结点）时判断为队列已满，这将区分 QueueListEmpty。
 * @see QueueListEmpty
 */
bool QueueListFull(QueueList *Self);

/** 
 * @brief 创建队列
 * @param[in] New 提供一个malloc函数
 * @param[in] Del 提供一个Del函数
 * @remark 此循环队列创建时已提供了一个未使用的队列结点，而不是仅仅生成两个指针。
 * @return 成功返回队列指针，失败返回空指针
 * @par 代码示例：
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	if (NULL != queue)
 * 	{
 * 		// 创建成功
 * 	}
 * @endcode
 */
QueueList *QueueListNew(queue_list_new New, queue_list_del Del);

/**
 * @brief 移除队列容器多余的结点
 * @param[in] Self 队列指针
 * @return 无
 * @note 将队列中的占位结点（队尾）的后继指针生成副本（备份）后指向队头指针，然后将占位结点后继指针的副本至队头指针之间的结点移除。
 */
void QueueListReNew(QueueList *Self);

/** 
 * @brief 销毁队列
 * @param[in] Self 队列指针
 * @return 无
 * @par 代码示例
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	if (NULL != queue)
 * 	{
 * 		// 创建成功后才能销毁
 * 		QueueListDel(queue);
 * 	}
 * @endcode
 */
void QueueListDel(QueueList *Self);

/** 
 * @brief 队列结点入队
 * @remark 当前队列容量不足，需补充队列结点。
 * @param[in] Self 队列指针
 * @param[in] PushNode 队列结点指针
 * @return 无
 * @note 在队列尾指针链接结点。
 */
void QueueListPushNode(QueueList *Self, QueueListNode *PushNode);

/** 
 * @brief 队列数据域入队
 * @remark 当前队列容量足够，则直接将数据域指针入队。
 * @param[in] Self 队列指针
 * @param[in] Value 需存储的值
 * @return 无
 * @note 在队列尾指针对应数据域赋值。
 */

void QueueListPushValue(QueueList *Self, QueueListValue Value);

/** 
 * @brief 队列入队
 * @remark 对外使用的队列入队操作。
 * @param[in] Self 队列指针
 * @param[in] Value 需存储的值
 * @return 成功返回存储 Value 的队列结点指针，否则返回空指针（内存申请失败）。
 * @par 代码示例
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	// 将Qnode指针数据入队
 * 	QueueListValue *Qnode = QueueListPush(queue, (QueueListValue)Qnode);
 * 	if (NULL != Qnode)
 * 	{
 * 		// 入队完成，返回存储Qnode指针数据的结点指针。
 * 	}
 * 	else
 * 	{
 * 		// 队列结点创建失败，可能是内存不足。
 * 	}
 * @endcode
 */
QueueListValue *QueueListPush(QueueList *Self, QueueListValue Value);

/** 
 * @brief 队列结点出队
 * @param[in] Self 队列指针
 * @return 无
 * @note 初始化队头指针的数据域后指向其下一结点。
 * @par 代码示例
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	if (QueueListExist(queue))
 * 	{
 * 		// 队列不为空，存在结点可以出队。
 * 		QueueListPop(queue);
 * 	}
 * @endcode
 */
void QueueListPop(QueueList *Self);

/** 
 * @brief 返回队列容器的的结点总数
 * @param[in] Self 队列指针
 * @param[in] All 控制选项：为0返回已被使用的结点数，为1返回所有结点
 * @return 队列结点总数
 */
uint32_t QueueListSize(QueueList *Self, bool All);

#endif // QUEUE_LIST_H
