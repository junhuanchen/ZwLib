/**  
 * @file
 * @brief ����������QueueList
 * @author JunHuanChen
 * @date 2018-01-22 16:04:34
 * @version 1.1
 * @remark
 * ������һ����������Ա�����֮��������ֻ�����ڱ��ǰ�ˣ�front������ɾ��������
 * ���ڱ�ĺ�ˣ�rear�����в�����������в�������Ķ˳�Ϊ��β������ɾ�������Ķ˳�Ϊ��ͷ��
 * ���Ǵ洢��ָ���������ݵĵ�ѭ�����У�������Ӻͳ���֮����ڲ��в�����
 */
#ifndef QUEUE_LIST_H
#define QUEUE_LIST_H

#include "../Node/NodeOneWay.h"
#include "../Struct.h"

typedef void *QueueListValue;  /** < ���н���������ָ��  */

typedef void *(*queue_list_new)(unsigned int); /** < �ڴ����뺯������  */

typedef void(*queue_list_del)(void *); /** < �ڴ��ͷź�������  */

/** 
 * @struct queue_list_node
 * @brief ���н��
 * @remark ����һ���洢ָ��ֵ��Ķ��н��ṹ��
 */
typedef struct queue_list_node
{
    QueueListValue Value;	    /** < ������  */
    NodeOneWay Node;    /** < ������  */
}QueueListNode;

/** 
 * @struct queue_list
 * @brief ����
 */
typedef struct queue_list
{
    NodeOneWay *Front;	    /** < ��ͷָ��  */
    NodeOneWay *Back;	    /** < ��βָ��  */ 
    queue_list_new New;     /** < �����ڴ溯��  */
    queue_list_del Del;     /** < �ͷ��ڴ溯��  */
}QueueList;

/** 
 * @brief ��ȡ���ж�ͷָ��
 * @param[in] Self ����ָ��
 * @return ���ض�ͷ���ֵָ��
 */
QueueListValue * QueueListFront(QueueList *Self);

/** 
 * @brief �ж϶����Ƿ����Ԫ��
 * @remark QueueListPop ǰ���жϡ�
 * @param[in] Self ����ָ��
 * @return ����Ԫ�ط���1�����򷵻�0��
 */
bool QueueListExist(QueueList *Self);

/** 
 * @brief �ж϶����Ƿ�Ϊ��
 * @param[in] Self ����ָ��
 * @return ����Ϊ�շ���1�����򷵻�0��
 * @note ��ͷ�Ͷ�βָ����ȱ�ʾ�����ڶ��н�㣬�⽫���� QueueListFull��
 * @see QueueListFull
 */
bool QueueListEmpty(QueueList *Self);

/** 
 * @brief �ж϶����Ƿ�����
 * @param[in] Self ����ָ��
 * @return ����Ϊ������1�����򷵻�0��
 * @note ���ڶ����д���һ��ռλ���(��ѭ��ʹ�õ�����������������ǣ����ʶ�ͷָ�����ռλ��㣨βָ��ĺ�̽�㣩ʱ�ж�Ϊ�����������⽫���� QueueListEmpty��
 * @see QueueListEmpty
 */
bool QueueListFull(QueueList *Self);

/** 
 * @brief ��������
 * @param[in] New �ṩһ��malloc����
 * @param[in] Del �ṩһ��Del����
 * @remark ��ѭ�����д���ʱ���ṩ��һ��δʹ�õĶ��н�㣬�����ǽ�����������ָ�롣
 * @return �ɹ����ض���ָ�룬ʧ�ܷ��ؿ�ָ��
 * @par ����ʾ����
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	if (NULL != queue)
 * 	{
 * 		// �����ɹ�
 * 	}
 * @endcode
 */
QueueList *QueueListNew(queue_list_new New, queue_list_del Del);

/**
 * @brief �Ƴ�������������Ľ��
 * @param[in] Self ����ָ��
 * @return ��
 * @note �������е�ռλ��㣨��β���ĺ��ָ�����ɸ��������ݣ���ָ���ͷָ�룬Ȼ��ռλ�����ָ��ĸ�������ͷָ��֮��Ľ���Ƴ���
 */
void QueueListReNew(QueueList *Self);

/** 
 * @brief ���ٶ���
 * @param[in] Self ����ָ��
 * @return ��
 * @par ����ʾ��
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	if (NULL != queue)
 * 	{
 * 		// �����ɹ����������
 * 		QueueListDel(queue);
 * 	}
 * @endcode
 */
void QueueListDel(QueueList *Self);

/** 
 * @brief ���н�����
 * @remark ��ǰ�����������㣬�貹����н�㡣
 * @param[in] Self ����ָ��
 * @param[in] PushNode ���н��ָ��
 * @return ��
 * @note �ڶ���βָ�����ӽ�㡣
 */
void QueueListPushNode(QueueList *Self, QueueListNode *PushNode);

/** 
 * @brief �������������
 * @remark ��ǰ���������㹻����ֱ�ӽ�������ָ����ӡ�
 * @param[in] Self ����ָ��
 * @param[in] Value ��洢��ֵ
 * @return ��
 * @note �ڶ���βָ���Ӧ������ֵ��
 */

void QueueListPushValue(QueueList *Self, QueueListValue Value);

/** 
 * @brief �������
 * @remark ����ʹ�õĶ�����Ӳ�����
 * @param[in] Self ����ָ��
 * @param[in] Value ��洢��ֵ
 * @return �ɹ����ش洢 Value �Ķ��н��ָ�룬���򷵻ؿ�ָ�루�ڴ�����ʧ�ܣ���
 * @par ����ʾ��
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	// ��Qnodeָ���������
 * 	QueueListValue *Qnode = QueueListPush(queue, (QueueListValue)Qnode);
 * 	if (NULL != Qnode)
 * 	{
 * 		// �����ɣ����ش洢Qnodeָ�����ݵĽ��ָ�롣
 * 	}
 * 	else
 * 	{
 * 		// ���н�㴴��ʧ�ܣ��������ڴ治�㡣
 * 	}
 * @endcode
 */
QueueListValue *QueueListPush(QueueList *Self, QueueListValue Value);

/** 
 * @brief ���н�����
 * @param[in] Self ����ָ��
 * @return ��
 * @note ��ʼ����ͷָ����������ָ������һ��㡣
 * @par ����ʾ��
 * @code
 * 	QueueList *queue = QueueListNew();
 * 	if (QueueListExist(queue))
 * 	{
 * 		// ���в�Ϊ�գ����ڽ����Գ��ӡ�
 * 		QueueListPop(queue);
 * 	}
 * @endcode
 */
void QueueListPop(QueueList *Self);

/** 
 * @brief ���ض��������ĵĽ������
 * @param[in] Self ����ָ��
 * @param[in] All ����ѡ�Ϊ0�����ѱ�ʹ�õĽ������Ϊ1�������н��
 * @return ���н������
 */
uint32_t QueueListSize(QueueList *Self, bool All);

#endif // QUEUE_LIST_H
