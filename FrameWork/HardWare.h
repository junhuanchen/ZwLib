
#ifndef HARDWARE_H
#define HARDWARE_H

#define _CRT_SECURE_NO_WARNINGS // 屏蔽 VS2013 sprintf

#ifdef _8051_ZSTACK_
typedef char   int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int    int32_t;
typedef unsigned int    uint32_t;
#define __inline inline

// 兼容 Iar For 8051 的 ZStack - Clib 没有 stdint.h 
// ZStack的Iar环境中需要Stm32的一些系统信息长度定义。
#else
#include <stdint.h> 
#endif

#include <stdbool.h>

#include <string.h>

enum EnumHardWare
{
	DevTmLen = sizeof(uint32_t),// 设备 二进制秒值 长度
	DevMsLen = sizeof(uint16_t),// 设备 二进制毫秒 长度
	DevIdLen = 12,				// 设备 ID 长度
	DevIpLen = sizeof(uint16_t) // 设备 IP 长度
};

#define	TRAN_MAX		0x40	// 采集数据传输长度

#ifdef STM32
// STM32 获取唯一的设备ID二进制串
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
