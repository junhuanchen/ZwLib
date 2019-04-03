
#include "ZgbEndDevice.h"

// ����������Ķ̵�ַ
static uint16_t SelfShortAddr;

// ���ñ��ն˵�����ʱ�õ��Ķ̵�ַ
void SetShortAddr(uint16_t Addr)
{
    SelfShortAddr = Addr;
}

// ����Ϊ��ʱ����ת���������ݵ���Ƭ��
bool ZgbTranScm(uint8_t Buf[ ], uint8_t BufLen)
{
    return 0 == ZwTranCheck(Buf, BufLen);
}

// ����Ϊ��ʱ����ת���������ݵ�Э����
bool ZgbTranCoord(uint8_t Buf[ ], uint8_t BufLen)
{
	return ZwEncodeSetDevIP(Buf, BufLen, SelfShortAddr);
}
