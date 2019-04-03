
/**
 * Copyright (c) 2017 DataPass.inc
 * All rights reserved.
 *
 * @file    property.c
 * @brief   ���ö�дģ��ʵ��
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
* @brief �������ݽṹ
*/
typedef struct ini_ctrl
{
	char * ini, * last;
	int16_t len;
}IniCtrl;

/**
 * IniCtrlInit
 * @brief   ��ʼ���������ݽṹ
 * @param[in]   Self �������ݽṹָ��
 * @param[in]   Ini  ��������ָ��
 * @param[in]   Len  �������鳤��
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
 * @brief   ΪIniCtrl�ṹ��Ӽ�ֵ�ԡ�
 * @param[in]   Self  �������ݽṹָ��
 * @param[in]   Key   �������ݼ�
 * @param[in]   Value ��������ֵ
 * @return  bool  �����Ƿ���ӳɹ��Ĳ���ֵ��
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
 * @brief   ����������������ȡ��ָ����ֵ������
 * @param[in]   IniIn ������������
 * @param[in]   Key	  �������ݼ�
 * @param[out]   Value ��������ֵ������
 * @param[in]   ValueLen ��������ֵ���峤��
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
 * @brief   ���ÿ��ƽṹ�����ַ������
 * @param[in]   Self ���ÿ��ƽṹָ��
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