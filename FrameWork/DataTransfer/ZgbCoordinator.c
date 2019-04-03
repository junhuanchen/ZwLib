
#include "ZgbCoordinator.h"

// 返回为真时将会转发缓冲数据到终端结点
bool ZgbTranEndDevice(uint16_t *Addr, uint8_t Buffer[ ], uint8_t BufLen)
{
	return ZwEncodeGetDevIP(Buffer, BufLen, Addr);
}

// 返回为真时将会转发缓冲数据到采集PC
bool ZgbTranComputer(uint8_t Buffer[ ], uint8_t BufLen)
{
	return 0 == ZwTranCheck(Buffer, BufLen);
}
