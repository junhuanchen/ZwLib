
/**
 * Copyright (c) 2017 DataPass.inc
 * All rights reserved.
 *
 * @file    property.c
 * @brief   配置读写模块实现
 * @version 1.0
 * @author  Junhuan Chen
 * @Email   741380738@qq.com
 * @date    2017-06-17
 */

#include "property.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

/**
* @struct ini_ctrl
* @brief 配置数据结构
*/
typedef struct ini_ctrl
{
	char * ini, * last;
	int16_t len;
}IniCtrl;

/**
 * IniCtrlInit
 * @brief   初始化配置数据结构
 * @param[in]   Self 配置数据结构指针
 * @param[in]   Ini  配置数组指针
 * @param[in]   Len  配置数组长度
 */
void IniCtrlInit(IniCtrl * Self, char * Ini, int16_t Len)
{
	assert(Len > 0);
	// lock ini string buffer.
	memset(Ini, '\0', Len);
	Self->last = Self->ini = Ini, Self->len = Len;
}

/**
 * IniCtrlSet
 * @brief   为IniCtrl结构添加键值对。
 * @param[in]   Self  配置数据结构指针
 * @param[in]   Key   配置数据键
 * @param[in]   Value 配置数据值
 * @return  bool  返回是否添加成功的布尔值。
 */
bool IniCtrlSet(IniCtrl * Self, const char * Key, const char * Value)
{
	char * pos = Self->last;
	char ZwBuf[MAX_KEY + MAX_VALUE + 3];
	if (EOF != sprintf(ZwBuf, "%.*s%c%.*s%c", MAX_KEY, Key, DelimiterZwPair, MAX_VALUE, Value, DelimiterStatement))
	{
		int16_t ZwLen = strlen(ZwBuf);
		if (Self->len > (pos - Self->ini) + ZwLen)
		{
			strncpy(pos, ZwBuf, ZwLen), Self->last += ZwLen;
			return true;
		}
	}
	return false;
}

/**
 * IniCtrlGet
 * @brief   从配置数据数组中取出指定键值对数据
 * @param[in]   IniIn 配置数据数组
 * @param[in]   Key	  配置数据键
 * @param[out]   Value 配置数据值缓冲区
 * @param[in]   ValueLen 配置数据值缓冲长度
 * @return  bool
 */
bool IniCtrlGet(const char * IniIn, const char * Key, char * Value, int8_t ValueLen)
{
	char key[MAX_KEY + 2] = { '\0' };
	if (EOF != sprintf(key, "%.*s%c", MAX_KEY, Key, DelimiterZwPair))
	{
		char * pos = strstr(IniIn, key);
		if (NULL != pos)
		{
			pos += strlen(key);
			char * tmp = strchr(pos, DelimiterStatement);
			if (NULL != tmp)
			{
				*tmp = '\0';
				if (EOF != sprintf(Value, "%.*s", ValueLen, pos))
				{
					*tmp = DelimiterStatement;
					return true;
				}
			}
		}
	}
	return false;
}

#ifdef UNIT_TEST

/**
 * IniCtrlDump
 * @brief   配置控制结构内容字符串输出
 * @param[in]   Self 配置控制结构指针
 */
void IniCtrlDump(IniCtrl * Self)
{
	puts(Self->ini);
}

int main()
{
	char IniBuf[0xFF];
	IniCtrl Ini;
	IniCtrlInit(&Ini, IniBuf, sizeof(IniBuf));
	assert(true == IniCtrlSet(&Ini, "01", "0123456"));
	IniCtrlDump(&Ini);
	assert(true == IniCtrlSet(&Ini, "0123456789", "0123456789"));
	IniCtrlDump(&Ini);
	char Value[MAX_VALUE + 1];
	IniCtrlGet(Ini.ini, "01", Value, sizeof(Value));
	puts(Value);
	IniCtrlGet(Ini.ini, "0123456789", Value, sizeof(Value));
	puts(Value);
	return 0;
}

#endif