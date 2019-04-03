/** 
 * @file
 * @brief 数据解析接口
 * @author JunHuanChen
 * @date 2018年2月8日10点22分
 * @version 1.1
 * @remark 提供传输协议解析的模块
 */
#ifndef ZW_PARSE_H
#define ZW_PARSE_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum ZwParseFlag
{
	ZwParseOk,	// 解析成功
	ZwParseNo,	// 解析失败
	ZwParseErr, // 缓冲区错误
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
