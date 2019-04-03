
/**
 * Copyright (c) 2017 DataPass.inc
 * All rights reserved.
 *
 * @file    property.h
 * @brief   ���ö�дģ��ӿ�
 * @version 1.0
 * @author  Junhuan Chen
 * @Email   741380738@qq.com
 * @date    2017-06-17
 */
#include <stdbool.h>
#include <stdint.h>

#define DelimiterStatement	'\n'	// ����ָ���
#define DelimiterZwPair		'='		// Ԫ�ضԷָ���		

#define MAX_KEY		4				// ������
#define MAX_VALUE	8				// ֵ����

typedef struct ini_ctrl IniCtrl;

void IniCtrlInit(IniCtrl * Self, char * Ini, int16_t Len);

bool IniCtrlSet(IniCtrl * Self, const char * Key, const char * Value);

bool IniCtrlGet(const char * IniIn, const char * Key, char * Value, int8_t ValueLen);
