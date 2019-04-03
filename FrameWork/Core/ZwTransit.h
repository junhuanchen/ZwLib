
#ifndef ZW_TRANSIT_H
#define ZW_TRANSIT_H

#include	"ZwData.h"
#include	"../../Algorithm/CRC/CRC.h"
#include	"../../Algorithm/BaseSeven/BaseSeven.h"

#if defined(_MANAGED)
extern "C++"
{
#endif

	// ��־���
#include "../FrameWork.hpp"

	// ����ģ���ʼ��
	void ZwTranInit(void);

	uint8_t ZwTranCheck(uint8_t Buf[], uint8_t Len);

	typedef struct zw_transit_binary
	{
		uint8_t DevIP[1];	// �豸�ĵ�ֵַ
		uint8_t DevTm[4];	// ���ʱ�豸��ʱ��
		uint8_t EntID[1];	// ������ҵ�ı�ʶֵ
		uint8_t DevMs[2];	// ���ʱ�豸����ʱ��
		uint8_t DevID[13];	// �豸��Ψһ��ʶֵ
	}ZwTransitBinary;

	typedef void(*ExternGetTime)(uint32_t *sec, uint16_t *ms); /** < 32λ������ʱ��ֵ��16λ�����ƺ���ֵ  */

	typedef uint8_t ZwEncodeType;

	enum ZwTransitEnum
	{
		ZwTranTypeCollect = 'T',
		ZwTranTypeCommand = 'D',
		ZwTranMax = 64,
		ZwTranTypeLen = sizeof(ZwEncodeType),
		ZwTranCrcLen = sizeof(crc),
		ZwBinaryLen = sizeof(ZwTransitBinary) / BsEncodeLen * BsDecodeLen,
		ZwEncodeCoreLen = ZwTranTypeLen + ZwTranCrcLen + ZwBinaryLen + 1, // + 1 Ϊ '\0' ռλ��
		ZwContentMax = (ZwTranMax - ZwEncodeCoreLen),
		ZwCmdMax = ZwContentMax - 1,
	};

#define ZwTransitType(Pack) Pack[0]
#define ZwDecodeDevIP(Pack) Pack->Zip.DevIP

	typedef struct zw_pack
	{
		uint8_t Crypt;
		ZwTransitBinary Zip;
		ExternGetTime Time;
	}ZwEncode;

	void ZwEncodeInit(ZwEncode * Self, uint8_t Crypt, uint8_t EntID, uint8_t DevID[], uint8_t DevIP, ExternGetTime Time);

	bool ZwEncodeGetDevIP(uint8_t Pack[], uint8_t PackLen, uint8_t *DevIP);

	bool ZwEncodeSetDevIP(uint8_t Pack[], uint8_t PackLen, uint8_t DevIP);

	uint8_t ZwEncodeCollect(ZwEncode * Self, uint8_t Pack[], uint8_t SrcLen, uint8_t Src[], uint8_t DataLen, uint8_t Data[]);

	bool ZwEncodeCollectCmpSrc(ZwEncode * Self, uint8_t Pack[], uint8_t PackLen, uint8_t SrcLen, uint8_t *Src);

	uint8_t ZwEncodeCommand(ZwEncode * Self, uint8_t Pack[], uint8_t CmdLen, uint8_t Cmd[]);

	typedef struct zw_unpack
	{
		uint8_t Crypt;
		ZwTransitBinary Zip;
	}ZwDecode;

	// ��ʼ�������������ṹ
	void ZwDecodeInit(ZwDecode *Self, uint8_t Crypt);

	uint8_t* ZwDecodeCore(ZwDecode * Self, uint8_t *Pack, uint8_t PackLen);

	bool ZwDecodeCollect(uint8_t String[], uint8_t *SrcLen, uint8_t Src[], uint8_t *DataLen, uint8_t Data[]);

	bool ZwDecodeCommand(uint8_t String[], uint8_t * CmdLen, uint8_t Cmd[]);

#if defined(_MANAGED)
}
#endif

#endif//ZW_TRANSIT_H
