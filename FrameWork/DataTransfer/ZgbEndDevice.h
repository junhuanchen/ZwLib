#ifndef ZGB_END_DEVICE_H
#define ZGB_END_DEVICE_H

#include "ZgbTran.h"

// ���ñ��ն˵�����ʱ�õ��Ķ̵�ַ
void SetShortAddr(uint16_t Addr);

// ����Ϊ��ʱ����ת���������ݵ���Ƭ��
bool ZgbTranScm(uint8_t Buf[ ], uint8_t BufLen);

// ����Ϊ��ʱ����ת���������ݵ�Э����
bool ZgbTranCoord(uint8_t Buf[ ], uint8_t BufLen);

#endif