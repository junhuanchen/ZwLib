/**
 * @file
 * @brief ��Ԫ���������ͣ�ZwData��
 * @author JunHuanChen
 * @date 2018-01-28 14:28:48
 * @version 1.2
 * @remark �ṩ����Դ�ṹ���洢���ʵȹ��ܡ�������������Key-Value���ݴ洢��
 */
#ifndef ZW_DATA_H
#define ZW_DATA_H

#include "../../Struct/Map/Map.h"

typedef MapKey ZwSource;

enum
{
	ZwSourceMax = sizeof( ZwSource ) + 8, // ��ֵ�������ֵ���ɵ�����
	ZwDataMax = 20						/**< ������Ԫ�����洢���ݳ��ȣ���64λʮ���Ƶ�ASCII��õ���  */
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
 * @brief ��Ԫ�����ݽṹ
 * @remark ��Ŵ������ݣ�����������Key-Value���ݡ�
 * @note ����ʹ�����������飬������C99��׼��
 */
typedef struct zw_data
{
	uint32_t Count;         /**< ����������ͨ������������������ݱ仯��  */
	volatile bool Used : 1;	/**< ռ�ñ�������Ǳ����Ƿ�����ʹ�á�		 */
	uint8_t DataLen : 7;    /**< ���ݳ��ȣ���ʶ��Ա data �����ĳ��ȡ�	 */
	uint8_t Data[];         /**< �䳤���ݣ��洢�Զ��峤�ȵ����ݡ�		 */
}ZwData;

typedef union zw_max_data
{
    ZwSource Self;
    uint8_t Area[ZwDataMax + 1];
}ZwMaxData;

/**
 * @brief �����ݸ��Ƶ���Ԫ������
 * @param[in] Self ZwDataָ��
 * @param[in] Target ��д��ZwData������
 * @return ��
 * @see memcpy
 */
#define ZwDataCpy(Self, Target) memcpy(Self->Data, Target, Self->DataLen)

/**
 * @brief ����Ԫ���������Ƚ�
 * @param[in] Self ��Ԫ������ָ��
 * @param[in] Target ��Ƚϵ�����
 * @return �������ʱ����0�����򷵻ط�0ֵ��
 * @see memcmp
 */
#define ZwDataCmp(Self, Target) memcmp(Self->Data, Target, Self->DataLen)

/**
 * @brief ������Ԫ������
 * @remark ʹ�����趨һ�����ݳ�����������Ҫ����Ԫ�����ݡ�
 * @param[in] DataLen ZwData �������ͳ���
 * @return �ɹ�����ZwDataָ�룬ʧ�ܷ��ؿ�ָ�롣
 * @par ����ʾ����
 * @code
 *	ZwData *data = ZwDataNew(1);
 *	if (NULL != data)
 *	{
 *		// �����ɹ�
 *	}
 * @endcode
 */
ZwData *ZwDataNew( uint8_t *DataLen );

/**
 * @brief ������Ԫ������
 * @param[in] Self ��Ԫ������ָ��
 * @return ��
 * @par ����ʾ����
 * @code
 *	ZwData *data = ZwDataNew(1);
 *	if (NULL != data)
 *	{
 *		// �����ɹ�
 *		ZwDataDel(data);
 *	}
 * @endcode
 */
void ZwDataDel( ZwData *Self );

#endif // ZW_DATA_H
