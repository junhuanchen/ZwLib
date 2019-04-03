/**
 * @file
 * @brief 软元件数据类型（ZwData）
 * @author JunHuanChen
 * @date 2018-01-28 14:28:48
 * @version 1.2
 * @remark 提供数据源结构、存储访问等功能。将适用于所有Key-Value数据存储。
 */
#ifndef ZW_DATA_H
#define ZW_DATA_H

#include "../../Struct/Map/Map.h"

typedef MapKey ZwSource;

enum
{
	ZwSourceMax = sizeof( ZwSource ) + 8, // 键值长度最大值，可调整。
	ZwDataMax = 20						/**< 定义软元件最大存储数据长度（由64位十进制的ASCII码得到）  */
};

#define ZwSourceSet MapKeySet

#define ZwSourceCopy MapKeyCopy

typedef union zw_max_source
{
	ZwSource Self;
	uint8_t Area[ZwSourceMax + 1];
}ZwMaxSource;

#define ZwMaxSourceCpy(UnionSelf, Source, SourceLen) ( UnionSelf )->Self.Len = SourceLen, memcpy( ( UnionSelf )->Self.Data, Source, SourceLen );

/**
 * @struct zw_data
 * @brief 软元件数据结构
 * @remark 存放传输数据，适用于所有Key-Value数据。
 * @note 由于使用了柔性数组，需启用C99标准。
 */
typedef struct zw_data
{
	uint32_t Count;         /**< 计数变量，通过变量上溢出触发数据变化。  */
	volatile bool Used : 1;	/**< 占用变量，标记变量是否正被使用。		 */
	uint8_t DataLen : 7;    /**< 数据长度，标识成员 data 变量的长度。	 */
	uint8_t Data[];         /**< 变长数据，存储自定义长度的数据。		 */
}ZwData;

typedef union zw_max_data
{
    ZwSource Self;
    uint8_t Area[ZwDataMax + 1];
}ZwMaxData;

/**
 * @brief 将数据复制到软元件数据
 * @param[in] Self ZwData指针
 * @param[in] Target 须写入ZwData的数据
 * @return 无
 * @see memcpy
 */
#define ZwDataCpy(Self, Target) memcpy(Self->Data, Target, Self->DataLen)

/**
 * @brief 与软元件数据作比较
 * @param[in] Self 软元件数据指针
 * @param[in] Target 须比较的数据
 * @return 数据相等时返回0，否则返回非0值。
 * @see memcmp
 */
#define ZwDataCmp(Self, Target) memcmp(Self->Data, Target, Self->DataLen)

/**
 * @brief 创建软元件数据
 * @remark 使用者设定一个数据长度以生成需要的软元件数据。
 * @param[in] DataLen ZwData 数据类型长度
 * @return 成功返回ZwData指针，失败返回空指针。
 * @par 代码示例：
 * @code
 *	ZwData *data = ZwDataNew(1);
 *	if (NULL != data)
 *	{
 *		// 创建成功
 *	}
 * @endcode
 */
ZwData *ZwDataNew( uint8_t *DataLen );

/**
 * @brief 销毁软元件数据
 * @param[in] Self 软元件数据指针
 * @return 无
 * @par 代码示例：
 * @code
 *	ZwData *data = ZwDataNew(1);
 *	if (NULL != data)
 *	{
 *		// 创建成功
 *		ZwDataDel(data);
 *	}
 * @endcode
 */
void ZwDataDel( ZwData *Self );

#endif // ZW_DATA_H
