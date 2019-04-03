
/**
 * @file
 * @brief Structȫ�ֶ���
 * @author JunHuanChen
 * @date 2018-01-21 22:22:32
 * @version 1.1
 * @remark �������������Ľṹ�巽������
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
 * @brief ��ȡ�ṹ�����ַָ�루StructOf��
 * @param[in] Addr �ṹ����ĳ����Ա�����ĵ�ַ
 * @param[in] Type �ṹ��������
 * @param[in] Field �ṹ��� addr ��Ա��
 * @return �ṹ�����ַ
 * @note ���ݽṹ��ĳ�Աָ�������ڽṹ�����ַ��0����ƫ��������õ��ṹ�����ַ��
 * @par ����ʾ����
 * @code
 * // ���Խṹ��
 * struct test
 * {
 * 	void *a;
 * }t, *pt;
 *
 * // �������еĳ�Աָ��
 * void *pa = &t.a;
 *
 * // ͨ��struct_of���ȡ���׵�ַ
 * pt = StructOf(pa, struct test, a);
 *
 * if(t.a == pt->a)
 * {
 * 	// ��ȡ�ɹ�
 * }
 * @endcode
 */
#define StructOf(Addr, Type, Field) \
  ((Type *)((char *)(Addr) - (unsigned long)(&((Type *)0)->Field)))

#endif // STRUCT_H
