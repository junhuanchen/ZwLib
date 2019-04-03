
#include "ZgbEndDevice.h"

// 本结点入网的短地址
static uint16_t SelfShortAddr;

// 设置本终端的入网时得到的短地址
void SetShortAddr(uint16_t Addr)
{
    SelfShortAddr = Addr;
}

// 返回为真时将会转发缓冲数据到单片机
bool ZgbTranScm(uint8_t Buf[ ], uint8_t BufLen)
{
    return 0 == ZwTranCheck(Buf, BufLen);
}

// 返回为真时将会转发缓冲数据到协调器
bool ZgbTranCoord(uint8_t Buf[ ], uint8_t BufLen)
{
	return ZwEncodeSetDevIP(Buf, BufLen, SelfShortAddr);
}
