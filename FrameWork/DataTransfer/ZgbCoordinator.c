
#include "ZgbCoordinator.h"

// ����Ϊ��ʱ����ת���������ݵ��ն˽��
bool ZgbTranEndDevice(uint16_t *Addr, uint8_t Buffer[ ], uint8_t BufLen)
{
	return ZwEncodeGetDevIP(Buffer, BufLen, Addr);
}

// ����Ϊ��ʱ����ת���������ݵ��ɼ�PC
bool ZgbTranComputer(uint8_t Buffer[ ], uint8_t BufLen)
{
	return 0 == ZwTranCheck(Buffer, BufLen);
}
