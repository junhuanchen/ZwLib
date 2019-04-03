/** 
 * @file
 * @brief ���ݽ����ӿ�
 * @author JunHuanChen
 * @date 2018��2��8��10��22��
 * @version 1.1
 * @remark �ṩ����Э�������ģ��
 */
#ifndef ZW_PARSE_H
#define ZW_PARSE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum ZwParseFlag
{
	ZwParseOk,	// �����ɹ�
	ZwParseNo,	// ����ʧ��
	ZwParseErr, // ����������
};

enum ZwParseData
{
	ZwParseSumOffset,
	ZwParseSourceLenOffset,
	ZwParseTypeLenOffset,
	ZwParseArrayOffset,
	ZwParseHeadSize = 4,
};

bool ZwParseCheckKvNano( uint8_t Buffer[], uint8_t BufLen );

enum ZwParseFlag ZwParseKvNano( uint8_t Buffer[], uint8_t BufLen, uint8_t Data[], uint8_t *DataLen );

bool ZwParseCheckHostLink( uint8_t Buffer[], uint8_t BufLen );

enum ZwParseFlag ZwParseHostLink( uint8_t Buffer[], uint8_t BufLen, uint8_t Data[], uint8_t *DataLen );

#include "ZwCache.h"
enum ZwParseFlag ZwParseHostLinkCache(uint8_t Buffer[], uint8_t BufLen, ZwCache *ZwCache);

#endif // ZW_PARSE_H
