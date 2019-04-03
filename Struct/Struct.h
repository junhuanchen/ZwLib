
/**
 * @file
 * @brief Struct全局定义
 * @author JunHuanChen
 * @date 2018-01-21 22:22:32
 * @version 1.1
 * @remark 关联各类容器的结构体方法定义
 */
#ifndef STRUCT_H
#define STRUCT_H

#ifdef _8051_ZSTACK_

typedef char   int8_t;
typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;

#define __inline inline

#else

#include <stdint.h>

#endif

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/**
 * @brief 获取结构体基地址指针（StructOf）
 * @param[in] Addr 结构体中某个成员变量的地址
 * @param[in] Type 结构体类型名
 * @param[in] Field 结构体的 addr 成员名
 * @return 结构体基地址
 * @note 根据结构体的成员指针与其在结构体基地址（0）的偏移量做差得到结构体基地址。
 * @par 代码示例：
 * @code
 * // 测试结构体
 * struct test
 * {
 * 	void *a;
 * }t, *pt;
 *
 * // 保存其中的成员指针
 * void *pa = &t.a;
 *
 * // 通过struct_of宏获取其首地址
 * pt = StructOf(pa, struct test, a);
 *
 * if(t.a == pt->a)
 * {
 * 	// 获取成功
 * }
 * @endcode
 */
#define StructOf(Addr, Type, Field) \
  ((Type *)((char *)(Addr) - (unsigned long)(&((Type *)0)->Field)))

#endif // STRUCT_H
