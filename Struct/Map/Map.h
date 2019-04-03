/**
 * @file
 * @brief 映射容器：Map (C接口)
 * @author JunHuanChen
 * @date 2016-10-19 18:50:13
 * @version 1.0
 * @remark Map 擅长处理一对一映射数据，它可以在编程上提供快速访问通道。
 */
#ifndef MAP_H
#define MAP_H

#include "../Struct.h"

#define StartupMapIteratorPair // true 启动迭代器函数

typedef void *(*map_new)(unsigned int); /**< 内存申请函数声明  */
typedef void(*map_del)(void *); /**< 内存释放函数声明  */

#include "../List/ListOneWay.h"
typedef NodeOneWay MapNode;	/**< @see NodeOneWay */
typedef ListOneWay MapList;	/**< @see ListOneWay */

/**
 *@brief 映射键类型
 */
typedef struct map_key
{
    uint8_t Len, Data[];
}MapKey;

MapKey *MapKeyNew( map_new New, uint8_t Len );

MapKey *MapKeySet( uint8_t Area[], uint8_t AreaLen, uint8_t Key[], uint8_t Len );

#define MapKeyDel(Del, Key) (Del(Key))

#define MapKeyCopy(target, source) memcpy(target->Data, source, target->Len)

typedef uint8_t MapHash; /**< 哈希数值类型 */

/**
* @brief 映射键比较相等函数
* @param[in] Left 比较左值
* @param[in] Right 比较右值
* @return 相等返回 true 否则 false
*/
bool MapKeyEqual(MapKey *Left, MapKey *Right);

/**
 * @brief 映射值类型
 */
typedef void *MapValue;

typedef MapValue(*map_value_new)(void *param); /**< 键值对的值析构函数声明  */

typedef void(*map_value_del)(MapValue); /**< 键值对的值析构函数声明  */

/**
 * @struct map_pair
 * @brief 键值对
 * @remark 这是一个存储映射键值对的链表结点结构，提供给 map 内部的链表使用。
 * @see MapKey MapValue
 */
typedef struct map_pair
{
    MapKey *Key;	/**< 键域，又称关键词，在映射容器中键唯一。  */
    MapValue Value;	/**< 值域，与键映射，同时存储用户自定义数据。  */
    MapNode Node;	/**< 结点，提供链表基本元素  */
}MapPair;			/**< @see map_pair  */

/**
 * @brief 查找链表中的键值对
 * @remark 根据键查找链表里是否存在与指定键相等的键值对。
 * @param[in] list 链表指针
 * @param[in] key 指定操作的键
 * @return 成功返回符合条件的键值对指针，失败返回空指针
 */
MapPair *MapListFind(MapList *List, MapKey *Key);

/**
 * @struct map
 * @brief 映射容器
 * @remark 这是一个支持快速访问数据的哈希链表容器。
 */
typedef struct map
{

#ifdef StartupMapIteratorPair

    uint32_t Iterator;
    NodeOneWay *IteratorEnd, *IteratorPos;
    MapPair *IteratorResult;

#endif

    map_new New;            /**< 申请内部内存  */
    map_del Del;            /**< 释放内部内存  */

    map_value_new NewValue; /**< 申请映射值函数  */
	
    map_value_del DelValue;	/**< 释放映射值函数  */

    MapHash ListSum;		/**< 链表总数  */
    MapList ListSet[];		/**< 链表柔性数组（C99）  */

} Map;						/**< @see map  */

/**
 * @brief 创建映射容器
 * @remark 使用者设定一个链表总数以生成需要的映射容器。
 * @param[in] ListSum 需要的链表总数
 * @param[in] New 内存申请接口
 * @param[in] Del 内存释放接口
 * @param[in] NewValue 映射值申请接口
 * @param[in] DelValue 映射值释放接口
 * @return 成功返回映射容器指针，失败返回空指针
 * @par 代码示例：
 * @code
 *  Map *map = MapNew(1);
 *  if (NULL == map)
 *  {
 *  	// 创建成功
 *  }
 * @endcode
 */
Map *MapNew(MapHash ListSum, map_new New, map_del Del, map_value_new NewValue, map_value_del DelValue);

/**
 * @brief 销毁映射容器
 * @param[in] Self 映射容器指针
 * @return 无
 * @code
 *  Map *map = MapNew(1);
 *  if (NULL == map)
 *  {
 *  	// 创建成功后才能销毁
 *  	MapDel(map);
 *  }
 * @endcode
 */
void MapDel(Map *Self);

/**
 * @brief 查找映射容器中的键值对
 * @param[in] Self 映射容器指针
 * @param[in] PairKey 指定操作的键
 * @return 成功返回符合条件的键值对指针，失败返回空指针
 * @code
 *  Map *map = MapNew(1);=
 *  // 查找键为1的键值对
 *  Map *pair = MapFindPair(map, 1);
 *  if (NULL != pair)
 *  {
 *  	// 键为1的键值对存在
 *  }
 *  else
 *  {
 *  	// 键为1的键值对不存在
 *  }
 * @endcode
 */
MapPair *MapFindPair(Map *Self, MapKey *Key);

/**
 * @brief 获取映射容器中的键值对
 * @remark 与 MapFindPair 不同的是查找失败后将创建数据并导入链表后做返回值。
 * @param[in] Self 映射容器指针
 * @param[in] key 指定操作的键
 * @param[in] NewValueParam Value 创建数据回调函数
 * @return 成功返回符合条件的键值对指针，失败返回空指针
 * @code
 *  Map *map = MapNew(1);
 *  // 查找键为1的键值对
 *  Map *pair = MapGetPair(map, 1, NULL);
 *  if (NULL != pair)
 *  {
 *  	// 获得键为1的键值对，若不存在则创建。
 *  }
 *  else
 *  {
 *  	// 键为1的键值对创建失败，内存不足。
 *  }
 * @endcode
 */
MapPair *MapGetPair( Map *Self, MapKey *Key, void *NewValueParam );

/**
 * @brief 移除映射容器中的键值对
 * @remark 警告：此函数在并行操作与其他函数存在冲突，请在多线程（或中断）中确保该函数执行期间独占映射容器。
 * @param[in] Self 映射容器指针
 * @param[in] Key 指定操作的键
 * @return 成功返回1，失败0。
 * @code
 *  Map *map = MapNew(1);
 *  // 存放键值对指针
 *  Map *pair;
 *  // 移除键为1的键值对
 *  if (true == MapRemovePair(map, 1))
 *  {
 *  	// 删除成功
 *  }
 *  else
 *  {
 *  	// 删除失败
 *  }
 * @endcode
 */
bool MapRemovePair(Map *Self, MapKey *Key);

#ifdef StartupMapIteratorPair
/**
 * @brief 映射容器访问迭代器
 * @remark 这是一个阻塞的遍历访问容器的函数。
 * @param[in] Self 映射容器指针
 * @return 阻塞至返回当前迭代访问的键值对指针。
 * @code
 *  Map *end = MapIteratorPair(map);
 *  do
 *  {
 *      // 操作
 *  	// 等待返回映射容器中的键值对指针
 *  	pair = MapIteratorPair(map);
 *  	// 获取成功
 *  }while(end != pair);
 * @endcode
 */
MapPair *MapIteratorPair(Map *Self);

#endif

#endif // MAP_H
