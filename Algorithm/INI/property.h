
/**
 * Copyright (c) 2017 DataPass.inc
 * All rights reserved.
 *
 * @file    property.h
 * @brief   配置读写模块接口
 * @version 1.0
 * @author  Junhuan Chen
 * @Email   741380738@qq.com
 * @date    2017-06-17
 */
#include <stdbool.h>
#include <stdint.h>

#define DelimiterStatement	'\n'	// 语句块分隔符
#define DelimiterZwPair		'='		// 元素对分隔符		

#define MAX_KEY		4				// 键长度
#define MAX_VALUE	8				// 值长度

typedef struct ini_ctrl IniCtrl;

void IniCtrlInit(IniCtrl * Self, char * Ini, int16_t Len);

bool IniCtrlSet(IniCtrl * Self, const char * Key, const char * Value);

bool IniCtrlGet(const char * IniIn, const char * Key, char * Value, int8_t ValueLen);
