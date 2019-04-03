/**
 * @file
 * @brief ӳ��������Map (C�ӿ�)
 * @author JunHuanChen
 * @date 2016-10-19 18:50:13
 * @version 1.0
 * @remark Map �ó�����һ��һӳ�����ݣ��������ڱ�����ṩ���ٷ���ͨ����
 */
#ifndef MAP_H
#define MAP_H

#include "../Struct.h"

#define StartupMapIteratorPair // true ��������������

typedef void *(*map_new)(unsigned int); /**< �ڴ����뺯������  */
typedef void(*map_del)(void *); /**< �ڴ��ͷź�������  */

#include "../List/ListOneWay.h"
typedef NodeOneWay MapNode;	/**< @see NodeOneWay */
typedef ListOneWay MapList;	/**< @see ListOneWay */

/**
 *@brief ӳ�������
 */
typedef struct map_key
{
    uint8_t Len, Data[];
}MapKey;

MapKey *MapKeyNew( map_new New, uint8_t Len );

MapKey *MapKeySet( uint8_t Area[], uint8_t AreaLen, uint8_t Key[], uint8_t Len );

#define MapKeyDel(Del, Key) (Del(Key))

#define MapKeyCopy(target, source) memcpy(target->Data, source, target->Len)

typedef uint8_t MapHash; /**< ��ϣ��ֵ���� */

/**
* @brief ӳ����Ƚ���Ⱥ���
* @param[in] Left �Ƚ���ֵ
* @param[in] Right �Ƚ���ֵ
* @return ��ȷ��� true ���� false
*/
bool MapKeyEqual(MapKey *Left, MapKey *Right);

/**
 * @brief ӳ��ֵ����
 */
typedef void *MapValue;

typedef MapValue(*map_value_new)(void *param); /**< ��ֵ�Ե�ֵ������������  */

typedef void(*map_value_del)(MapValue); /**< ��ֵ�Ե�ֵ������������  */

/**
 * @struct map_pair
 * @brief ��ֵ��
 * @remark ����һ���洢ӳ���ֵ�Ե�������ṹ���ṩ�� map �ڲ�������ʹ�á�
 * @see MapKey MapValue
 */
typedef struct map_pair
{
    MapKey *Key;	/**< �����ֳƹؼ��ʣ���ӳ�������м�Ψһ��  */
    MapValue Value;	/**< ֵ�����ӳ�䣬ͬʱ�洢�û��Զ������ݡ�  */
    MapNode Node;	/**< ��㣬�ṩ�������Ԫ��  */
}MapPair;			/**< @see map_pair  */

/**
 * @brief ���������еļ�ֵ��
 * @remark ���ݼ������������Ƿ������ָ������ȵļ�ֵ�ԡ�
 * @param[in] list ����ָ��
 * @param[in] key ָ�������ļ�
 * @return �ɹ����ط��������ļ�ֵ��ָ�룬ʧ�ܷ��ؿ�ָ��
 */
MapPair *MapListFind(MapList *List, MapKey *Key);

/**
 * @struct map
 * @brief ӳ������
 * @remark ����һ��֧�ֿ��ٷ������ݵĹ�ϣ����������
 */
typedef struct map
{

#ifdef StartupMapIteratorPair

    uint32_t Iterator;
    NodeOneWay *IteratorEnd, *IteratorPos;
    MapPair *IteratorResult;

#endif

    map_new New;            /**< �����ڲ��ڴ�  */
    map_del Del;            /**< �ͷ��ڲ��ڴ�  */

    map_value_new NewValue; /**< ����ӳ��ֵ����  */
	
    map_value_del DelValue;	/**< �ͷ�ӳ��ֵ����  */

    MapHash ListSum;		/**< ��������  */
    MapList ListSet[];		/**< �����������飨C99��  */

} Map;						/**< @see map  */

/**
 * @brief ����ӳ������
 * @remark ʹ�����趨һ������������������Ҫ��ӳ��������
 * @param[in] ListSum ��Ҫ����������
 * @param[in] New �ڴ�����ӿ�
 * @param[in] Del �ڴ��ͷŽӿ�
 * @param[in] NewValue ӳ��ֵ����ӿ�
 * @param[in] DelValue ӳ��ֵ�ͷŽӿ�
 * @return �ɹ�����ӳ������ָ�룬ʧ�ܷ��ؿ�ָ��
 * @par ����ʾ����
 * @code
 *  Map *map = MapNew(1);
 *  if (NULL == map)
 *  {
 *  	// �����ɹ�
 *  }
 * @endcode
 */
Map *MapNew(MapHash ListSum, map_new New, map_del Del, map_value_new NewValue, map_value_del DelValue);

/**
 * @brief ����ӳ������
 * @param[in] Self ӳ������ָ��
 * @return ��
 * @code
 *  Map *map = MapNew(1);
 *  if (NULL == map)
 *  {
 *  	// �����ɹ����������
 *  	MapDel(map);
 *  }
 * @endcode
 */
void MapDel(Map *Self);

/**
 * @brief ����ӳ�������еļ�ֵ��
 * @param[in] Self ӳ������ָ��
 * @param[in] PairKey ָ�������ļ�
 * @return �ɹ����ط��������ļ�ֵ��ָ�룬ʧ�ܷ��ؿ�ָ��
 * @code
 *  Map *map = MapNew(1);=
 *  // ���Ҽ�Ϊ1�ļ�ֵ��
 *  Map *pair = MapFindPair(map, 1);
 *  if (NULL != pair)
 *  {
 *  	// ��Ϊ1�ļ�ֵ�Դ���
 *  }
 *  else
 *  {
 *  	// ��Ϊ1�ļ�ֵ�Բ�����
 *  }
 * @endcode
 */
MapPair *MapFindPair(Map *Self, MapKey *Key);

/**
 * @brief ��ȡӳ�������еļ�ֵ��
 * @remark �� MapFindPair ��ͬ���ǲ���ʧ�ܺ󽫴������ݲ����������������ֵ��
 * @param[in] Self ӳ������ָ��
 * @param[in] key ָ�������ļ�
 * @param[in] NewValueParam Value �������ݻص�����
 * @return �ɹ����ط��������ļ�ֵ��ָ�룬ʧ�ܷ��ؿ�ָ��
 * @code
 *  Map *map = MapNew(1);
 *  // ���Ҽ�Ϊ1�ļ�ֵ��
 *  Map *pair = MapGetPair(map, 1, NULL);
 *  if (NULL != pair)
 *  {
 *  	// ��ü�Ϊ1�ļ�ֵ�ԣ����������򴴽���
 *  }
 *  else
 *  {
 *  	// ��Ϊ1�ļ�ֵ�Դ���ʧ�ܣ��ڴ治�㡣
 *  }
 * @endcode
 */
MapPair *MapGetPair( Map *Self, MapKey *Key, void *NewValueParam );

/**
 * @brief �Ƴ�ӳ�������еļ�ֵ��
 * @remark ���棺�˺����ڲ��в����������������ڳ�ͻ�����ڶ��̣߳����жϣ���ȷ���ú���ִ���ڼ��ռӳ��������
 * @param[in] Self ӳ������ָ��
 * @param[in] Key ָ�������ļ�
 * @return �ɹ�����1��ʧ��0��
 * @code
 *  Map *map = MapNew(1);
 *  // ��ż�ֵ��ָ��
 *  Map *pair;
 *  // �Ƴ���Ϊ1�ļ�ֵ��
 *  if (true == MapRemovePair(map, 1))
 *  {
 *  	// ɾ���ɹ�
 *  }
 *  else
 *  {
 *  	// ɾ��ʧ��
 *  }
 * @endcode
 */
bool MapRemovePair(Map *Self, MapKey *Key);

#ifdef StartupMapIteratorPair
/**
 * @brief ӳ���������ʵ�����
 * @remark ����һ�������ı������������ĺ�����
 * @param[in] Self ӳ������ָ��
 * @return ���������ص�ǰ�������ʵļ�ֵ��ָ�롣
 * @code
 *  Map *end = MapIteratorPair(map);
 *  do
 *  {
 *      // ����
 *  	// �ȴ�����ӳ�������еļ�ֵ��ָ��
 *  	pair = MapIteratorPair(map);
 *  	// ��ȡ�ɹ�
 *  }while(end != pair);
 * @endcode
 */
MapPair *MapIteratorPair(Map *Self);

#endif

#endif // MAP_H
