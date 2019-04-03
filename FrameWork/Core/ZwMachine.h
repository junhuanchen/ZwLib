
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "../HardWare.h"

#include "ZwData.h"
#include "ZwCache.h"
#include "ZwCtrl.h"
#include "ZwTransit.h"

#define ReplyLimit 50*200 // 触发重发临界值
#define ReplyPre 50 // 每次触发值 rand % (0 - ReplyPre)

#include <assert.h>

 #include <stdio.h>
// #define puts(arg)
// #define printf(arg, ...)

typedef void(*ExternSend)(void *SendParam, uint8_t *Data, uint8_t Len);

typedef struct zw_machine
{
	ZwCache Cache;
	ZwEncode En;
	void *SendParam;
	ExternSend Send;
	bool Allow;
	uint32_t Reply;
}ZwMachine;

#ifdef STM32
extern void LockInTime(uint32_t start);
extern bool ZwMachineRecvCommand(ZwMachine *Self, uint8_t * Command, uint8_t CmdLen);
#else
#include <time.h>
time_t ExternLockInTime = 0;
void LockInTime(uint32_t start)
{
	printf("LockInTime %d : %d\r\n", ExternLockInTime, start);
	ExternLockInTime = start;
}
bool ZwMachineRecvCommand(ZwMachine *Self, uint8_t * Command, uint8_t CmdLen)
{
	printf("ZwMachineRecvCommand IP %hhd %.*s\r\n", *Self->En.Zip.DevIP, CmdLen, Command);
	uint8_t key[17], value[21];
	Command[CmdLen] = '\0', sscanf((char *)Command, "%s %s", key, value);
	return true;
}
#endif

void ZwMachineInit(ZwMachine *Self, uint8_t EntID, uint8_t DevID[DevIdLen], uint8_t DevIP, ExternSend Send, void * SendParam, ExternGetTime GetTime)
{
	assert(true == ZwCacheNew(&Self->Cache, 8, 128));
	ZwEncodeInit(&Self->En, 43, EntID, DevID, DevIP, GetTime);
	Self->SendParam = SendParam;
	Self->Send = Send;
	Self->Allow = true;
	Self->Reply = 0;
}

void ZwMachineSend(ZwMachine *Self)
{
	if (true == ZwCacheCheckEvent(&Self->Cache))
	{
		printf( "ZwTaskUploadData Self->Allow %d Self->Reply %d\r\n", Self->Allow, Self->Reply ); 

		if (Self->Allow || (Self->Reply += rand() % ReplyPre, Self->Reply > ReplyLimit)) // 发送失败后随机扰乱时序 
		{
			MapPair *pair = ZwCacheGetEvent(&Self->Cache);
			if (NULL != pair)
			{
				printf("Upload Collect Data IP %hhd %.*s - %.*s \r\n", \
					*Self->En.Zip.DevIP, pair->Key->Len, pair->Key->Data, ((ZwData *)pair->Value)->DataLen, ((ZwData *)pair->Value)->Data);
				uint8_t buffer[ZwTranMax] = { 0 }, send_len = ZwEncodeCollect(&Self->En, buffer, pair->Key->Len, pair->Key->Data, ((ZwData *)pair->Value)->DataLen, ((ZwData *)pair->Value)->Data);
				if (send_len)
				{
					Self->Send(Self->SendParam, buffer, send_len);

					Self->Allow = false, Self->Reply = 0;
				}
			}
		}
	}
}

void ZwMachineRecv(ZwMachine *Self, ZwEncodeType Type, uint8_t *String)
{
	if (Type == ZwTranTypeCollect)
	{
		if (!Self->Allow && true == ZwCacheCheckEvent(&Self->Cache))
		{
			ZwCacheRemoveEvent(&Self->Cache);
			Self->Allow = true;

			//MapPair *pair = ZwCacheGetEvent(&Self->Cache);
			//uint8_t src_len, src_id[ZwSourceMax], data_len, data[ZwDataMax];
			//if (true == ZwDecodeCollect(String, &src_len, src_id, &data_len, data))
			//{
			//	if (pair->Key->Len == src_len && 0 == memcmp(src_id, pair->Key->Data, src_len))
			//	{
			//		ZwCacheRemoveEvent(&Self->Cache);
			//		Self->Upload.Allow = true;
			//	}
			//}
		}

	}
	else if (Type == ZwTranTypeCommand)
	{
		uint8_t command[ZwCmdMax], cmd_len;
		if (ZwDecodeCommand(String, &cmd_len, command) && ZwMachineRecvCommand(Self, command, cmd_len))
		{
			printf("Download Command Data %.*s \r\n", cmd_len, command);
			uint8_t buffer[ZwTranMax] = { 0 }, send_len = ZwEncodeCommand(&Self->En, buffer, cmd_len, command);
			if (send_len)
			{
				Self->Send(Self->SendParam, buffer, send_len);
			}
		}
	}
}

typedef struct zw_runtime
{
	ZwCtrl Ctrl;
	ZwDecode De;
	uint32_t RunTime;
} ZwRunTime;

void ZwRunTimeInit(ZwRunTime *Self)
{
	ZwTranInit();
	ZwDecodeInit(&Self->De, 55);
	Self->RunTime = 0;
	assert(0 == Self->RunTime);
	assert(true == ZwCtrlNew(&Self->Ctrl));
}

void ZwRunTimeAddMachine(ZwRunTime *Self, ZwMachine *Machine)
{
	uint8_t area[16], temp[15];
	sprintf((char *)temp, "DevIP %hhd", *Machine->En.Zip.DevIP);
	MapKey *key = MapKeySet(area, sizeof(area), temp, strlen((char *)temp));
	assert(true == ZwCtrlCreateTask(&Self->Ctrl, key, (ZwMethod)ZwMachineSend, Machine, 0, 1));
}

ZwMachine * ZwRunTimeGetMachine(ZwRunTime *Self, uint8_t DevIP)
{
	uint8_t area[16], temp[15];
	sprintf((char *)temp, "DevIP %hhd", DevIP);
	MapKey *key = MapKeySet(area, sizeof(area), temp, strlen((char *)temp));

	MapPair * pair = ZwCtrlGetTask(&Self->Ctrl, key);
	if (pair)
	{
		return (ZwMachine *)((ZwTask *)pair->Value)->Param;
	}
	return NULL;
}

bool ZwRunTimeRecvPack(ZwRunTime *Self, uint8_t * Buffer, uint8_t BufLen)
{
	uint8_t *String = ZwDecodeCore(&Self->De, Buffer, BufLen);
	if (NULL != String)
	{
		// printf("RecvPack Data IP %hhd - %.*s\r\n", *Self->De.Zip.DevIP, String[0], String + 1);
				
		if (Buffer[0] == ZwTranTypeCommand)
		{
			uint8_t command[ZwCmdMax], cmd_len;
			if (ZwDecodeCommand(String, &cmd_len, command))
			{
				if (0 == memcmp(command, "TimeSysn", cmd_len))
				{
					uint32_t time = *(uint32_t *)(Self->De.Zip.DevTm);
					LockInTime(time);
				}
			}
		}
		ZwMachine * machine = ZwRunTimeGetMachine(Self, *Self->De.Zip.DevIP);
		if (machine)
		{
			ZwMachineRecv(machine, Buffer[0], String);
		}
		return true;
	}
	return false;
}

void ZwRunTimeLoop(ZwRunTime *Self)
{
	ZwCtrlRunning(&Self->Ctrl, Self->RunTime++);
}

#ifdef UNIT_TEST

#include <Windows.h>

void TestRecv(struct ZwMachine *Self, char Buffer[], char BufLen)
{
	if (true == ZwCacheCheckEvent(Self->Cache))
	{
		MapPair *pair = ZwCacheGetEvent(Self->Cache);
		if (true == ZwEncodeCollectCmpSrc(Self->Pack, (uint8_t *)Buffer, BufLen, pair->Key->Len, pair->Key->Data))
		{
			ZwCacheRemoveEvent(Self->Cache);
			puts("TestRecv");
		}
	}
}

void TestSend(void *Self, uint8_t Buffer[], uint8_t BufLen)
{
	puts("TestSend");
	TestRecv((struct ZwMachine *) Self, (char *)Buffer, BufLen);
}

#include <time.h>

void GetTime(uint32_t *sec, uint16_t *ms)
{
	*sec = time(NULL), *ms = GetTickCount64();
}

#include <assert.h>

struct GetDataParam
{
	ZwMaxSource Source;
	const void *Data;
	ZwCache *Cache;
};

void ZwTaskGetData(struct GetDataParam *Self)
{
	char num[6];
	sprintf(num, "%05hd", *(short *)Self->Data);
	printf("Get Data %.*s - %.*s\n", Self->Source.Self.Len, Self->Source.Self.Data, 5, num);
	ZwCacheUpdateEvent(Self->Cache, Self->Source.Self.Len, Self->Source.Self.Data, 5, (uint8_t *)num);
}

void ZwTaskUpdateTemperature(short *data_temperature)
{
	*data_temperature += rand() % 3 - 1;
	printf("ZwTaskUpdata Data T ： %hd\n", *data_temperature);
}

void UnitTestZwTask()
{
	ZwEncode pack;
	ZwEncodeInit(&pack, 4, rand(), (uint8_t *)"0123456789ABCDEF", 0, GetTime);

	ZwCache cache;
	assert(true == ZwCacheNew(&cache, 2, 10));

	ZwCtrl ctrl;
	assert(true == ZwCtrlNew(&ctrl));

	uint8_t area[20];
	MapKey *key = MapKeySet(area, sizeof(area), 8);
	assert(NULL != key);

	short data_temperature = 100; // 模拟温度外部数据源
	struct GetDataParam param_temperature;
	ZwMaxSourceCpy(&param_temperature.Source, "T", 1);
	param_temperature.Data = &data_temperature;
	param_temperature.Cache = &cache;

	MapKeyCopy(key, "GetData");
	assert(true == ZwCtrlCreateTask(&ctrl, key, (ZwMethod)ZwTaskGetData, &param_temperature, GetTickCount64(), 500));

	struct ZwMachine param_upload;
	param_upload.Cache = &cache;
	param_upload.Pack = &pack;
	param_upload.SendParam = &param_upload;
	param_upload.SendPack = TestSend;

	MapKeyCopy(key, "UploadData");
	assert(true == ZwCtrlCreateTask(&ctrl, key, (ZwMethod)ZwTaskUploadData, &param_upload, GetTickCount64(), 100));

	MapKeyCopy(key, "UpdateTemperature");
	assert(true == ZwCtrlCreateTask(&ctrl, key, (ZwMethod)ZwTaskUpdateTemperature, &data_temperature, GetTickCount64(), 1000));

	while (true)
	{
		uint32_t TimeMs = GetTickCount64();

		ZwCtrlRunning(&ctrl, TimeMs);

		Sleep(10);
	}

	ZwCtrlDel(&ctrl);

	ZwCacheDel(&cache);

}

#endif
