#ifndef ZGB_END_DEVICE_H
#define ZGB_END_DEVICE_H

#include "ZgbTran.h"

// 设置本终端的入网时得到的短地址
void SetShortAddr(uint16_t Addr);

// 返回为真时将会转发缓冲数据到单片机
bool ZgbTranScm(uint8_t Buf[ ], uint8_t BufLen);

// 返回为真时将会转发缓冲数据到协调器
bool ZgbTranCoord(uint8_t Buf[ ], uint8_t BufLen);

#endif