#ifndef ZGB_COORDINATOR_H
#define ZGB_COORDINATOR_H

#include "ZgbTran.h"

// ����Ϊ��ʱ����ת���������ݵ��ն˽��
bool ZgbTranEndDevice(uint16_t *Addr, uint8_t Buffer[ ], uint8_t BufLen);
// ����Ϊ��ʱ����ת���������ݵ��ɼ�PC
bool ZgbTranComputer(uint8_t Buffer[ ], uint8_t BufLen);

#endif