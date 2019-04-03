
extern "C"
{
#include "ZwMachine.h"
extern time_t ExternLockInTime;
}

#include <stdio.h>
#include <assert.h>
#include "..\DataInterface\Tdp\Tdp.hpp"
#include <time.h>


namespace UnitTestZwVirtual
{
	static const char IdasClientDevId[] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	struct UdpPack
	{
		CHAR Buffer[64];
		UdpPack()
		{
			memset(Buffer, 0, sizeof(Buffer));
		}
		UdpPack(char * String)
		{
			strcpy(Buffer, String);
		}
		UdpPack(uint8_t * Buffer, uint8_t BufLen)
		{
			memcpy(this->Buffer, Buffer, BufLen);
		}
	};

	struct TdpTranModel : public Tdp<UdpPack>
	{
		SOCKADDR_IN Addr;

		static void GetTime(uint32_t *Sec, uint16_t *Ms)
		{
			static time_t last_time = time(NULL);
			SYSTEMTIME st;
			GetLocalTime(&st);
			//tm temptm = { st.wSecond, st.wMinute, st.wHour, st.wDay,
			//	st.wMonth - 1, st.wYear - 1900, st.wDayOfWeek, 0, 0 };
			*Sec = /*mktime(&temptm) + ExternLockInTime - */last_time, *Ms = st.wMilliseconds;
		}

		TdpTranModel(uint16_t LocalPort, uint16_t SendPort)
		{
			if (this->StartServer(LocalPort))
			{
				Addr.sin_family = AF_INET;				// 服务端地址族
				Addr.sin_addr.s_addr = inet_addr("127.0.0.1");	// 绑定服务端IP地址
				Addr.sin_port = htons(SendPort);			// 绑定服务端端口号
			}
		}

		~TdpTranModel()
		{
			this->StopServer();
			WSACleanup();
		}

		void Send(UdpPack & buf, uint8_t len)
		{
			SendTo(Addr, buf, len);
		}
	};

	struct VirtualMachine : public TdpTranModel
	{
		ZwRunTime RunTime;

		void TdpRecvProcess(TdpFrame & Packet)
		{
			PutInfo("VirtualMachine TdpRecvProcess : %.*s\n", Packet.Len, Packet.Var.Buffer);

			ZwRunTimeRecvPack(&RunTime, (uint8_t *)Packet.Var.Buffer, Packet.Len);
		}

		static void SendTo(TdpTranModel *Tdp, uint8_t *buf, uint8_t len)
		{
			Tdp->Send(UdpPack(buf, len), len);
		}

		VirtualMachine() :TdpTranModel(9955, 9954)
		{
			ZwRunTimeInit(&RunTime);

			PutInfo("VirtualMachine Start!\n");
		}

		~VirtualMachine()
		{
			TdpTranModel::StopServer();
		}

		struct GetDataParam
		{
			ZwMaxSource Source;
			const void *Data;
			ZwCache *Cache;
		};

		static void ZwTaskGetData(struct GetDataParam *Self)
		{
			char data[5];
			sprintf(data, "%04hx", *(short *)Self->Data);
			// PutInfo("VirtualMachine GetData %.*s - %.*s\n", Self->Source.Self.Len, Self->Source.Self.Data, 4, data);
			ZwCacheUpdateEvent(Self->Cache, Self->Source.Self.Len, Self->Source.Self.Data, 4, (uint8_t *)data);
		}

		static void ZwTaskUpdateTemperature(short *data_temperature)
		{
			*data_temperature += rand() % 3 - 1;
			PutInfo("VirtualMachine ZwTaskUpdateTemperature ： %hd\n", *data_temperature);
		}

		static void ZwTaskUpdateSpeed(short *data_speed)
		{
			static enum {
				Add = +1, Sub = -1, Min = 0, Max = 300
			} state = Add;
			switch (state)
			{
				case Add:
					(*data_speed < Max) ? *data_speed += state : state = Sub;
					break;
				case Sub:
					(*data_speed > Min) ? *data_speed += state : state = Add;
					break;
			}
			PutInfo("VirtualMachine ZwTaskUpdateSpeed ： %hd\n", *data_speed);
		}

#define Config(DevID)\
		void Config##DevID()\
		{\
			static ZwMachine dev00##DevID;\
			ZwMachineInit(&dev00##DevID, 0x01, (uint8_t *)"dev0"#DevID, DevID, (ExternSend)SendTo, this, TdpTranModel::GetTime);\
			ZwRunTimeAddMachine(&RunTime, &dev00##DevID);\
			ZwMachine * machine = ZwRunTimeGetMachine(&RunTime, DevID);\
			assert(machine != NULL);\
			uint8_t area[20];\
			MapKey *key = NULL;\
			static short data_speed = 0;\
			assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"UploadSpeed "#DevID, strlen("UploadSpeed "#DevID))));\
			assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskUpdateSpeed, &data_speed, 50, 1000));\
			static struct GetDataParam param_speed;\
			ZwMaxSourceCpy(&param_speed.Source, "speed", strlen("speed"));\
			param_speed.Data = &data_speed, param_speed.Cache = &machine->Cache;\
			assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"GetSpeed "#DevID, strlen("GetSpeed "#DevID))));\
			assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskGetData, &param_speed, 25, 500));\
		}

		//Config(03);
		//Config(04);
		//Config(05);
		//Config(06);
		//Config(07);
		//Config(10);
		//Config(11);
		//Config(12);
		//Config(13);
		//Config(14);
		//Config(15);
		//Config(16);
		//Config(17);
		//Config(18);
		//Config(19);
		//Config(20);
		//Config(21);
		//Config(22);
		//Config(23);
		//Config(24);
		//Config(25);
		//Config(26);
		//Config(27);
		//Config(28);
		//Config(29);
		//Config(30);
		//Config(31);
		//Config(32);
		//Config(33);
		//Config(34);
		//Config(35);
		//Config(36);
		//Config(37);
		//Config(38);
		//Config(39);
		//Config(40);
		//Config(41);
		//Config(42);
		//Config(43);
		//Config(44);
		//Config(45);
		//Config(46);
		//Config(47);
		//Config(48);
		//Config(49);
		//Config(50);
		//Config(51);
		//Config(52);
		//Config(53);
		//Config(54);
		//Config(55);
		//Config(56);
		//Config(57);
		//Config(58);
		//Config(59);
		//Config(60);
		//Config(61);
		//Config(62);
		//Config(63);
		//Config(64);
		//Config(65);
		//Config(66);
		//Config(67);
		//Config(68);
		//Config(69);
		//Config(70);
		//Config(71);

		void Config01()
		{
			static ZwMachine dev001;

			ZwMachineInit(&dev001, 0x01, (uint8_t *)"1P-01-L1-DGLB", 1, (ExternSend)SendTo, this, TdpTranModel::GetTime);

			ZwRunTimeAddMachine(&RunTime, &dev001);

			ZwMachine * machine = ZwRunTimeGetMachine(&RunTime, 1);
			assert(machine != NULL);

			uint8_t area[20];
			MapKey *key = NULL;

			//static short data_temperature = 100; // 模拟temp外部数据源
			//assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"UploadTemp 1", strlen("UploadTemp 1"))));
			//assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskUpdateTemperature, &data_temperature, 500, 1000));

			//static struct GetDataParam param_temperature;
			//ZwMaxSourceCpy(&param_temperature.Source, "temp", strlen("temp"));
			//param_temperature.Data = &data_temperature, param_temperature.Cache = &machine->Cache;

			//assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"GetTemp 1", strlen("GetTemp 1"))));
			//assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskGetData, &param_temperature, 25, 50));
			
			{
				static short data_speed = 0; // 模拟speed外部数据源
				assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"UploadSpeed 1", strlen("UploadSpeed 1"))));
				assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskUpdateSpeed, &data_speed, 50, 500));

				static struct GetDataParam param_speed;
				ZwMaxSourceCpy(&param_speed.Source, "speed", strlen("speed"));
				param_speed.Data = &data_speed, param_speed.Cache = &machine->Cache;

				assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"GetSpeed 1", strlen("GetSpeed 1"))));
				assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskGetData, &param_speed, 25, 1000));
			}

#define TestCase(Name)\
			{\
				static short data_##Name = 0;\
				assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"Upload"#Name" 1", strlen("Upload"#Name" 1"))));\
				assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskUpdateSpeed, &data_##Name, 50, 500));\
				static struct GetDataParam param_##Name;\
				ZwMaxSourceCpy(&param_##Name.Source, #Name, strlen(#Name));\
				param_##Name.Data = &data_##Name, param_##Name.Cache = &machine->Cache;\
				assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"Get"#Name" 1", strlen("Get"#Name" 1"))));\
				assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskGetData, &param_##Name, 25, 1000));\
			}

			//TestCase(aa);
			//TestCase(at);
			//TestCase(ba);
			//TestCase(bt);
			//TestCase(ca);
			//TestCase(ct);
			//TestCase(la);
			//TestCase(nt);
			//TestCase(s0);

		}

		void Config02()
		{
			static ZwMachine dev002;

			ZwMachineInit(&dev002, 0x01, (uint8_t *)"dev002", 2, (ExternSend)SendTo, this, TdpTranModel::GetTime);

			ZwRunTimeAddMachine(&RunTime, &dev002);

			ZwMachine * machine = ZwRunTimeGetMachine(&RunTime, 2);
			assert(machine != NULL);

			uint8_t area[20];
			MapKey *key = NULL;

			static short data_temperature = 100; // 模拟temp外部数据源
			assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"UploadTemp 2", strlen("UploadTemp 2"))));
			assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskUpdateTemperature, &data_temperature, 0, 1000));

			static struct GetDataParam param_temperature;
			ZwMaxSourceCpy(&param_temperature.Source, "temp", strlen("temp"));
			param_temperature.Data = &data_temperature, param_temperature.Cache = &machine->Cache;

			assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"GetTemp 2", strlen("GetTemp 2"))));
			assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskGetData, &param_temperature, 0, 500));

			//static short data_speed = 0; // 模拟speed外部数据源
			//assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"UploadSpeed 2", strlen("UploadSpeed 2"))));
			//assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskUpdateSpeed, &data_speed, 0, 100));

			//static struct GetDataParam param_speed;
			//ZwMaxSourceCpy(&param_speed.Source, "speed", strlen("speed"));
			//param_speed.Data = &data_speed, param_speed.Cache = &machine->Cache;

			//assert(NULL != (key = MapKeySet(area, sizeof(area), (uint8_t *)"GetSpeed 2", strlen("GetSpeed 2"))));
			//assert(true == ZwCtrlCreateTask(&RunTime.Ctrl, key, (ZwMethod)ZwTaskGetData, &param_speed, 0, 50));
		}

		void Execute()
		{
			while (true)
			{
				uint32_t TimeMs = GetTickCount64();

				// ZwCtrlRunning( &Ctrl, TimeMs );

				ZwRunTimeLoop(&RunTime);

				Sleep(1);
			}
		}

	};

	struct VirtualServer : public TdpTranModel
	{
		ZwEncode En;
		ZwDecode PackRecv;

		VirtualServer() : TdpTranModel(9954, 9955)
		{
			ZwEncodeInit(&En, 7, 0xFF, (uint8_t *)IdasClientDevId, 0, GetTime);
			ZwDecodeInit(&PackRecv, 3);

			PutInfo("VirtualServer Start!\n");
		}

		void TdpRecvProcess(TdpFrame & Packet)
		{
			PutInfo("VirtualServer TdpRecvProcess : %.*s\t", Packet.Len, Packet.Var.Buffer);
			//switch (Packet.Var.Buffer[0])
			//{
			//case ZwTranTypeCollect:
			//{
			//	ZwMaxSource source;
			//	ZwMaxData data;
			//	TdpFrame temp(Packet);
			//	if (true == ZwDecodeCollect(&PackRecv, (uint8_t *)temp.Var.Buffer, temp.Len, &source.Self.Len, source.Self.Data, &data.Self.Len, data.Self.Data))
			//	{
			//		SendTo(Packet.Addr, Packet.Var, Packet.Len);
			//		PutInfo("Collect Source:%.*s Data:%.*s\n", source.Self.Len, source.Self.Data, data.Self.Len, data.Self.Data);
			//	}
			//	break;
			//}
			//case ZwTranTypeCommand:
			//{
			//	uint8_t Cmd[ZwTranMax - ZwEncodeCoreLen], CmdLen;
			//	if (true == ZwDecodeCommand(&this->PackRecv, (uint8_t *)Packet.Var.Buffer, Packet.Len, &CmdLen, Cmd))
			//	{
			//		if (0 == memcmp(Cmd, "TimeSysn", CmdLen))
			//		{
			//			uint8_t pack_len = ZwEncodeCommand(&this->En, (uint8_t *)Packet.Var.Buffer, CmdLen, Cmd);
			//			if (0 != pack_len)
			//			{
			//				uint32_t time = *(uint32_t *)En.Zip.DevTm;
			//				PutInfo("TimeSysn:%u\n", time);
			//				SendTo(Packet.Addr, Packet.Var, pack_len);
			//			}
			//		}
			//	}
			//}
			//default:
			//	break;
			//}
		}
	};

	void main()
	{
		ZwTranInit();
		WSADATA wsaData;
		if (0 == WSAStartup(MAKEWORD(0x02, 0x02), &wsaData))
		{
			// VirtualServer server;
			VirtualMachine machine;
			machine.Config02();
			/*
			machine.Config01();
			machine.Config03();
			machine.Config04();
			machine.Config05();
			machine.Config06();
			machine.Config07();
			machine.Config10();
			machine.Config11();
			machine.Config12();
			machine.Config13();
			machine.Config14();
			machine.Config15();
			machine.Config16();
			machine.Config17();
			machine.Config18();
			machine.Config19();
			machine.Config20();
			machine.Config21();
			machine.Config22();
			machine.Config23();
			machine.Config24();
			machine.Config25();
			machine.Config26();
			machine.Config27();
			machine.Config28();
			machine.Config29();
			machine.Config30();
			machine.Config31();
			machine.Config32();
			machine.Config33();
			machine.Config34();
			machine.Config35();
			machine.Config36();
			machine.Config37();
			machine.Config38();
			machine.Config39();
			machine.Config40();
			machine.Config41();
			machine.Config42();
			machine.Config43();
			machine.Config44();
			machine.Config45();
			machine.Config46();
			machine.Config47();
			machine.Config48();
			machine.Config49();
			machine.Config50();
			machine.Config51();
			machine.Config52();
			machine.Config53();
			machine.Config54();
			machine.Config55();
			machine.Config56();
			machine.Config57();
			machine.Config58();
			machine.Config59();
			machine.Config60();
			machine.Config61();
			machine.Config62();
			machine.Config63();
			machine.Config64();
			machine.Config65();
			machine.Config66();
			machine.Config67();
			machine.Config68();
			machine.Config69();
			machine.Config70();
			machine.Config71();
			*/
			machine.Execute();
			system("pause");
			WSACleanup();
		}
	}

}

