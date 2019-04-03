#ifndef BASE_SEVEN_H
#define BASE_SEVEN_H

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;
typedef unsigned int    uint32_t;

enum BaseSeven
{
	BsEncodeLen = 7, // ���뵥λ
	BsDecodeLen = 8, // ���뵥λ
};

uint8_t BaseSevenEncode(uint8_t Buf[], uint32_t BufLen, uint8_t Src[], uint32_t SrcLen);

uint8_t BaseSevenDecode(uint8_t Buf[], uint32_t BufLen, uint8_t Src[], uint32_t SrcLen);

#endif // BASE_SEVEN_H
