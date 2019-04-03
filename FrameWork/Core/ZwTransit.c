#include "ZwTransit.h"

void ZwTranInit(void)
{
	CrcInit();
}

#include "../../Algorithm/RSA/Rsa.h"
#define		CIPHER_MAX		0xFF // ����ԭʼ���ݳ���

static void ZwTranCrypt(uint8_t String[], uint8_t Crypt)
{
    for (; *String; String++)
    {
        if (CIPHER_MAX != *String)
        {
            *String = RsaCrypt(*String, Crypt, CIPHER_MAX);
        }
    }
}

uint8_t ZwTranCheck(uint8_t Buf[], uint8_t Len)
{
	return CrcFast(Buf, Len);
}

static void ZwTranSetCheck(uint8_t Buf[], uint8_t Len)
{
	Buf[Len] = ZwTranCheck(Buf, Len);
}

// ��ʼ�����������ֵ��̬���ݡ�
void ZwEncodeInit(ZwEncode * Self, uint8_t Crypt, uint8_t EntID, uint8_t DevID[], uint8_t DevIP, ExternGetTime Time)
{
	// ��ʼ��Ϊ��
	memset(Self, '\0', sizeof(*Self));
	// ��������̬����
	memcpy(Self->Zip.EntID, &EntID, sizeof(Self->Zip.EntID));
	memcpy(Self->Zip.DevID, DevID, sizeof(Self->Zip.DevID));
	memcpy(Self->Zip.DevIP, &DevIP, sizeof(Self->Zip.DevIP));
	// �����Լ�������ݽӿ�
	Self->Time = Time;
	Self->Crypt = Crypt;
}

static void ZwEncodeCore(ZwEncode * Self, ZwEncodeType Type, uint8_t Pack[])
{
	// �������ݰ�����
	Pack[0] = Type;
	// ��ȡʱ��ֵ�����õ������Ʊ�����
	Self->Time((uint32_t *) Self->Zip.DevTm, (uint16_t *) Self->Zip.DevMs);
	// �Զ��������ݽ��б���
	BaseSevenEncode(Pack + sizeof(Type), ZwBinaryLen, (uint8_t *)&Self->Zip, sizeof(Self->Zip));
}

bool ZwEncodeGetDevIP(uint8_t Pack[], uint8_t PackLen, uint8_t *DevIP)
{
	if (((PackLen < ZwTranMax) && (0 == ZwTranCheck(Pack, PackLen))))
	{
		// ���벿�ֺ�������
		uint8_t tmp[BsDecodeLen];
		// ���ݽṹ���豸IP�ڱ�������һ���֡�
		if (BaseSevenDecode(Pack + ZwTranTypeLen, BsDecodeLen, tmp, sizeof(tmp)))
		{
			// ���DevIP����
			memcpy(DevIP, tmp, sizeof(*DevIP));
			return true;
		}
	}
	return false;
}

bool ZwEncodeSetDevIP(uint8_t Pack[], uint8_t PackLen, uint8_t DevIP)
{
	if (((PackLen < ZwTranMax) && (0 == ZwTranCheck(Pack, PackLen))))
	{
		// ���뻺����
		uint8_t tmp[BsEncodeLen];
		// ����Э�飬����IP�ڱ�������һ���֣����ݽ��롣
		if (BaseSevenDecode(Pack + ZwTranTypeLen, BsDecodeLen, tmp, sizeof(tmp)))
		{
			// �޸�DevIP����
			memcpy(tmp, &DevIP, sizeof(DevIP));
			// ����д���������
			if (BaseSevenEncode(Pack + ZwTranTypeLen, BsDecodeLen, tmp, sizeof(tmp)))
			{
				// �������ð�β��У����
				ZwTranSetCheck(Pack, PackLen - ZwTranCrcLen);
				return true;
			}
		}
	}
	return false;
}

uint8_t ZwEncodeCollect(ZwEncode * Self, uint8_t Pack[], uint8_t SrcLen, uint8_t Src[], uint8_t DataLen, uint8_t Data[])
{
	uint8_t pack_len = (ZwEncodeCoreLen + sizeof(SrcLen) + sizeof(DataLen)) + SrcLen + DataLen - 1;
	if (pack_len < ZwTranMax && DataLen <= ZwDataMax)
	{
		// �����������
		ZwEncodeCore(Self, ZwTranTypeCollect, Pack);
		// ���ܷ������
		uint8_t * pack = Pack + ZwTranTypeLen + ZwBinaryLen;
		*pack = SrcLen, pack += sizeof(SrcLen);
		memcpy(pack, Src, SrcLen), pack += SrcLen;
		*pack = DataLen, pack += sizeof(DataLen);
		memcpy(pack, Data, DataLen), pack[DataLen] = '\0';
		ZwTranCrypt(Pack + ZwTranTypeLen + ZwBinaryLen, Self->Crypt);
		// ���㵱ǰ����У���벢�����ĩλ
		ZwTranSetCheck(Pack, pack_len - ZwTranCrcLen);
		Pack[pack_len] = '\0';
		return pack_len;
	}
	return 0;
}

uint8_t ZwEncodeCommand(ZwEncode * Self, uint8_t Pack[], uint8_t CmdLen, uint8_t Cmd[])
{
	uint8_t pack_len = ZwEncodeCoreLen + sizeof(CmdLen) + CmdLen - 1;
	if (pack_len < ZwTranMax)
	{
		// �����������
		ZwEncodeCore(Self, ZwTranTypeCommand, Pack);
		// ���ܷ������
		uint8_t * pack = Pack + ZwTranTypeLen + ZwBinaryLen;
		*pack = CmdLen, pack += sizeof(CmdLen);
		memcpy(pack, Cmd, CmdLen), pack[CmdLen] = '\0';
		ZwTranCrypt(Pack + ZwTranTypeLen + ZwBinaryLen, Self->Crypt);
		// ���㵱ǰ����У���벢�����ĩλ
		ZwTranSetCheck(Pack, pack_len - ZwTranCrcLen);
		Pack[pack_len] = '\0';
		return pack_len;
	}
	return 0;
}

// ��ʼ�������������ṹ
void ZwDecodeInit(ZwDecode *Self, uint8_t Crypt)
{
	memset(Self, '\0', sizeof(*Self));
	Self->Crypt = Crypt;
}

// ����У�顢���롢���ܿ�ܣ�����Crypt��������
uint8_t* ZwDecodeCore(ZwDecode * Self, uint8_t *Pack, uint8_t PackLen)
{
	if (PackLen < ZwTranMax)
	{
		if (0 == ZwTranCheck(Pack, PackLen))
		{
			Pack[PackLen - 1] = '\0'; // �Ƴ�У����
			if (BaseSevenDecode(Pack + ZwTranTypeLen, ZwBinaryLen, (uint8_t *)&Self->Zip, sizeof(Self->Zip)))
			{
				Pack += ZwTranTypeLen + ZwBinaryLen;
				ZwTranCrypt(Pack, Self->Crypt);
				return Pack;
			}
			else
			{
				// LogOut:�����������쳣��
				PutInfo("UnPackCoreCheck : �����������쳣\n");
			}
		}
		else
		{
			// LogOut:У��ֵ�����쳣��
			PutInfo("UnPackCoreCheck : У��ֵ�����쳣\n");
		}
	}
	else
	{
		// LogOut:���ݳ�����������
		PutInfo("UnPackCoreCheck : ���ݳ���������\n");
	}
	return NULL;
}

bool ZwDecodeCollect(uint8_t String[], uint8_t *SrcLen, uint8_t Src[], uint8_t *DataLen, uint8_t Data[])
{
	// ȡ��Crypt������
	if (String[0] <= ZwSourceMax)
	{
		*SrcLen = String[0];
		memcpy(Src, String + sizeof(*SrcLen), *SrcLen);
		String += *SrcLen + sizeof(*SrcLen);
		if (String[0] <= ZwDataMax)
		{
			*DataLen = String[0];
			memcpy(Data, String + sizeof(*DataLen), *DataLen);
			return true;
		}
	}
	return false;
}

bool ZwDecodeCommand(uint8_t String[], uint8_t * CmdLen, uint8_t Cmd[])
{
	if (String[0] <= ZwCmdMax)
	{
		*CmdLen = String[0];
		memcpy((char *) Cmd, (char *)String + sizeof(*CmdLen), *CmdLen);
		return true;
	}
	return false;
}

#ifdef UNIT_TEST
#include <assert.h>

#include <time.h>
#include <stdlib.h>

void GetTime(uint32_t *sec, uint16_t *ms)
{
	*sec = time(NULL), *ms = rand();
}

void UnitTestZwTransit()
{
	const uint8_t RsaDe = 43, RsaEn = 3;
	ZwTranInit();
	ZwEncode En;
	ZwEncodeInit(&En, RsaDe, 123, "0123456789ABCDEF", 21, GetTime);
	ZwDecode De;
	ZwDecodeInit(&De, RsaEn);
	time_t NTimeTm, LTimeTm;
	uint16_t NTimeMs, LTimeMs;

	// ���²���ָ���

	uint8_t buffer[ZwTranMax], buflen, *String;

#define __command "0123456789ABCDEF0123456789ABCDEF0123456789AB"

	assert(strlen(__command) == ZwCmdMax);
	buflen = ZwEncodeCommand(&En, buffer, ZwCmdMax, __command);
	assert(buflen == ZwTranMax - 1);

	LTimeTm = *(uint32_t *) En.Zip.DevTm;
	LTimeMs = *(uint32_t *) En.Zip.DevMs;

	uint8_t Command[ZwContentMax], CmdLen;

	String = ZwDecodeCore(&De, buffer, buflen);
	assert(NULL != String);

	assert(ZwDecodeCommand(String, &CmdLen, Command));
	assert(CmdLen == ZwCmdMax && !memcmp(Command, __command, CmdLen));

	NTimeTm = *(uint32_t *) De.Zip.DevTm;
	NTimeMs = *(uint32_t *) De.Zip.DevMs;
	assert(NTimeTm == LTimeTm && NTimeMs == LTimeMs);

	// ���²��Բɼ���
	uint8_t pack_len = ZwEncodeCollect(&En, buffer, 8, "DM000000", 20, "12345123451234512345");
	assert(pack_len == strlen(buffer)); // ��ζ������һ���ַ���

	LTimeTm = *(uint32_t *) En.Zip.DevTm;
	LTimeMs = *(uint32_t *) En.Zip.DevMs;

	// δ���ʱ���úͻ�ȡ�豸��ַ
	uint8_t DevIP;
	ZwEncodeGetDevIP(buffer, pack_len, &DevIP);
	assert(DevIP == 21);
	ZwEncodeSetDevIP(buffer, pack_len, 45);
	ZwEncodeGetDevIP(buffer, pack_len, &DevIP);
	assert(DevIP == 45);

	uint8_t Src[8] = "DM000000", SrcLen = 8;
	uint8_t Data[20], DataLen;

	// ���ʱ��ȡ��������
	String = ZwDecodeCore(&De, buffer, pack_len);
	assert(NULL != String);

	// assert(true == ZwEncodeCollectCmpSrc(&En, buffer, pack_len, SrcLen, Src));

	assert(ZwDecodeCollect(String, &SrcLen, Src, &DataLen, Data));
	assert(SrcLen == 8 && !memcmp(Src, "DM000000", SrcLen));
	assert(DataLen == 20 && !memcmp(Data, "12345123451234512345", DataLen));
	assert(*((uint8_t *) De.Zip.DevIP) == 45);
	assert(*((uint8_t *) De.Zip.EntID) == 123);
	assert(!memcmp(De.Zip.DevID, "0123456789ACBDEFG", sizeof(De.Zip.DevID)));

	NTimeTm = *(uint32_t *) De.Zip.DevTm;
	NTimeMs = *(uint32_t *) De.Zip.DevMs;
	assert(NTimeTm == LTimeTm && NTimeMs == LTimeMs);

	// char TimeStr[20];
	// strftime(TimeStr, sizeof(TimeStr), "%Y-%m-%d %H:%M:%S", localtime(&NTimeTm));
}

int main()
{
	UnitTestZwTransit();
}

#endif
