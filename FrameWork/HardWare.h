
#ifndef HARDWARE_H
#define HARDWARE_H

#define _CRT_SECURE_NO_WARNINGS // ���� VS2013 sprintf

#ifdef _8051_ZSTACK_
typedef char   int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int    int32_t;
typedef unsigned int    uint32_t;
#define __inline inline

// ���� Iar For 8051 �� ZStack - Clib û�� stdint.h 
// ZStack��Iar��������ҪStm32��һЩϵͳ��Ϣ���ȶ��塣
#else
#include <stdint.h> 
#endif

#include <stdbool.h>

#include <string.h>

enum EnumHardWare
{
	DevTmLen = sizeof(uint32_t),// �豸 ��������ֵ ����
	DevMsLen = sizeof(uint16_t),// �豸 �����ƺ��� ����
	DevIdLen = 12,				// �豸 ID ����
	DevIpLen = sizeof(uint16_t) // �豸 IP ����
};

#define	TRAN_MAX		0x40	// �ɼ����ݴ��䳤��

#ifdef STM32
// STM32 ��ȡΨһ���豸ID�����ƴ�
static void Stm32GetDevID(uint8_t DevID[DevIdLen])
{
	memcpy(DevID, (void*)(0x1FFFF7E8), sizeof(uint32_t));
	memcpy(DevID + 4, (void*)(0x1FFFF7EC), sizeof(uint32_t));
	memcpy(DevID + 8, (void*)(0x1FFFF7F0), sizeof(uint32_t));
}
#else
static void Stm32GetDevID(uint8_t DevID[DevIdLen])
{
	;
}
#endif
#endif//HARDWARE_H
