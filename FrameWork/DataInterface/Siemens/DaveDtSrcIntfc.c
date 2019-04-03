
#include "DaveInterface.h"
#include "DaveDataSource.h"

extern int32_t daveDebugSwitch;

/*
Universal receive with timeout:
*/
int32_t DAVE_DECL _daveRecvData(DaveDtSrc * DtSrc, uint8_t * BufIn, int32_t RecvLen)
{
#ifdef STM32

	if(ERR_OK == tcp_check_connect(DtSrc->File))
	{
		return tcp_client_recv(DtSrc->File, BufIn, RecvLen, DtSrc->TimeOut);
#else
	// SOCKET
	fd_set FDS;
	FD_ZERO(&FDS);
	FD_SET(DtSrc->File, &FDS);
#ifdef WIN32

	if (select(1, &FDS, NULL, NULL, &DtSrc->TimeOut) > 0)
	{
		return recv(DtSrc->File, (char *)BufIn, RecvLen, 0);
#elif LINUX

	if(select(DtSrc->File + 1, &FDS, NULL, NULL, &DtSrc->TimeOut) > 0)
	{
		return read(DtSrc->File, BufIn, RecvLen);
#endif
#endif
	}
	else
	{
		return 0;   // TimeOut!
	}
}

/*
Universal receive with timeout:
*/
void DAVE_DECL _daveSendData(DaveDtSrc * DtSrc, uint8_t * BufOut, int32_t SendLen)
{
#ifdef WIN32

	if (SOCKET_ERROR == send((SOCKET) (DtSrc->File), (char *)BufOut, SendLen, 0))
	{
		LogExec((daveDebugSwitch & daveDebugPrintErrors),
			LogPut("_daveSendISOPacket WSAGetLastError: %d \n", WSAGetLastError()));
	}

#elif LINUX
	// UNUSED
#elif STM32


	tcp_client_send(DtSrc->File, BufOut, SendLen);

#endif
}

#ifdef WIN32
uint8_t DaveFileCheckConnect(DaveFile file)
{
	int optval, optlen = sizeof(optval);
	return (INVALID_SOCKET == file) ? FALSE : 0 == getsockopt(file, SOL_SOCKET, SO_ERROR, (char *) &optval, &optlen);
}

DaveFile DaveFileOpen(const char * addrIP, uint16_t addrPort)
{
	SOCKADDR_IN addr;               // Connect Addr
	int addrlen = sizeof(addr);
	LogExec((daveDebugSwitch & daveDebugOpen), LogPut("DaveFileOpenTCP start!\n"));
	addr.sin_family = AF_INET;      // IPV4 Socket Type
	addr.sin_port = htons(addrPort);// Set Port Value
	//addr.sin_port = (((addrPort) & 0xff) << 8) | (((addrPort) & 0xff00) >> 8); // Endian change
	LogExec((daveDebugSwitch & daveDebugOpen), LogPut("IP:%s Port:%04X\n", addrIP, addr.sin_port));
#ifdef DAVE_USED_GETHOSTBYNAME
	struct hostent * entry; // get hostname
	entry = gethostbyname(addrIP);

	if(NULL == entry) { return false; }

	memcpy(&addr.sin_addr, entry->h_addr_list[0], sizeof(addr.sin_addr));
#else
	// inet_aton(addrIP, &addr.sin_addr);
	addr.sin_addr.s_addr = inet_addr(addrIP);
#endif
	LogExec((daveDebugSwitch & daveDebugOpen), LogPut("peer:%s = %d\n", addrIP, inet_addr(addrIP)));
	DaveFile file = socket(AF_INET, SOCK_STREAM, AF_UNSPEC);

	if (SOCKET_ERROR != file)
	{
		LogExec((daveDebugSwitch & daveDebugOpen), LogPut("create socket:%p!\n", &file));

		if (SOCKET_ERROR != connect(file, (SOCKADDR *) & addr, sizeof(addr)))
		{
			LogExec((daveDebugSwitch & daveDebugOpen), LogPut("Connected host: %s \n", addrIP));
			// char res = fcntl(file->Object, F_SETFL, O_NONBLOCK);
			// LogExec((file->daveDebugSwitch & daveDebugOpen), LogPut("Set mode to O_NONBLOCK %s %d\n", strerror(errno), res));
			// I thought this might solve Marc's problem with the CP closing a connection after 30 seconds or so, but the Standrad keepalive time on my box is 7200 seconds.
			// char opt = 1;
			// char res = setsockopt(file->Object, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
			// LogPut("setsockopt %s %d\n", strerror(errno),res);
			return file;
		}

		LogExec((daveDebugSwitch & daveDebugOpen), LogPut("connect error: %d \n", WSAGetLastError()));
		closesocket(file), file = DAVE_TRAN_INVALID;
	}

	return file;
}
void DaveFileClose(DaveFile file)
{
	if (file != DAVE_TRAN_INVALID)
	{
		closesocket(file), file = DAVE_TRAN_INVALID;
	}
}

#elif LINUX
#elif STM32
#endif

#ifdef UNIT_TEST
#define _CRTDBG_MAP_ALLOC /**< VS 提供的 malloc 内存泄漏检测宏  */
#include <crtdbg.h>
#include <time.h>

int main()
{
	WSADATA wsadata;

	if (0 == WSAStartup(MAKEWORD(2, 0), &wsadata))
	{
		DaveFile file = INVALID_SOCKET;

		if (INVALID_SOCKET != (file = DaveFileOpen("192.168.2.1", 102)))
		{
            DaveDtSrc * DtSrc = DaveNewDtSrc(malloc, file, daveCommTypePG, 1, 0, daveSpeed9300k, 40000000, daveProtoISOTCP);

			if (DtSrc)
			{
				daveDebugSwitch = daveDebugOFF;
				DaveIntfc * Intfc = DaveNewIntfc(malloc, free, DtSrc);

				if (Intfc)
                {
					if (daveResOK == DaveIntfcCnctPLC(Intfc))
					{
						DaveIntfcStartPLC(Intfc);
						int res;
                        printf("Connected.\n");
                        while (1)
                        {
							Sleep(1000);

							//uint16_t VW2100;
							//VW2100 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 2100, sizeof(VW2100), false, &VW2100);
							//printf("res:%d VW2100:%x\n", res, daveGetTwoBytefrom(&VW2100));
							//uint16_t VW750;
							//VW750 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 750, sizeof(VW750), false, &VW750);
							//printf("res:%d VW750:%x\n", res, daveGetTwoBytefrom(&VW750));
							// 变量读取测试
							//system("pause");
							//DaveIntfcStopPLC(Intfc);
							//system("pause");
							//DaveIntfcStartPLC(Intfc);
							//system("pause");

							//// 以下为测试值
							//for ( uint8_t I0[9] = { 0 }, i = 0; i < sizeof(I0); i++ )
							//{
							//    res = DaveIntfcReadBytes(Intfc, daveInputs, 0, i, sizeof(I0[i]), true, &I0[i]);
							//    printf("res:%d I0.%d:%d\n", res, i, I0[i]);
							//}

							// uint8_t MB0;
							// MB0 = 50, res = DaveIntfcWriteBytes(Intfc, daveFlags, 0, 0, sizeof(MB0), &MB0);

							//printf("res:%d MB0:%d\n", res, MB0);
							// MB0 = 0, res = DaveIntfcReadBytes(Intfc, daveFlags, 0, 0, sizeof(MB0), false, &MB0);
							//printf("res:%d MB0:%d\n", res, MB0);

							//uint8_t Q00;
							//res = DaveIntfcReadBytes(Intfc, daveOutputs, 0, 0, sizeof(Q00), true, &Q00);
							//printf("res:%d Q0.0:%d\n", res, Q00);

							//uint8_t QB1;
							//QB1 = 100, res = DaveIntfcWriteBytes(Intfc, daveOutputs, 0, 0, sizeof(QB1), &QB1);
							//printf("res:%d QB1:%d\n", res, QB1);
							//QB1 = 0, res = DaveIntfcReadBytes(Intfc, daveOutputs, 0, 0, sizeof(QB1), false, &QB1);
							//printf("res:%d QB1:%d\n", res, QB1);

							//uint8_t VB100[20] = "\x11Hello Wrold!";
							//res = DaveIntfcWriteBytes(Intfc, daveDB, 1, 100, strlen(VB100), &VB100);
							//printf("res:%d VB100:%.*s\n", res, VB100[0], VB100 + 1);
							//memset(VB100, '\0', sizeof(VB100));
							//res = DaveIntfcReadBytes(Intfc, daveDB, 1, 100, sizeof(VB100), false, &VB100);
							//printf("res:%d VB100:%.*s\n", res, VB100[0], VB100 + 1);

							//uint8_t VB120[20] = "\x5Wrold!";
							//res = DaveIntfcWriteBytes(Intfc, daveDB, 1, 120, strlen(VB120), &VB120);
							//printf("res:%d VB120:%.*s\n", res, VB120[0], VB120 + 1);
							//memset(VB120, '\0', sizeof(VB120));
							//res = DaveIntfcReadBytes(Intfc, daveDB, 1, 120, sizeof(VB120), false, &VB120);
							//printf("res:%d VB120:%.*s\n", res, VB120[0], VB120 + 1);

							uint8_t T37[5];
							// for ( int i = 0; i < 0xFFFF; i++ )
							{
							    // T0 T1 T5 T32 T34 T37
							    res = DaveIntfcReadBytes(Intfc, daveTimer200, 0, 37, 1, false, T37);
							    printf("T37: ");
							    for ( int i = 0; i < sizeof(T37); i++ ) printf("%d ", T37[i]);
							    putchar('\n');
							    res = DaveIntfcGetLastRecvData(Intfc, T37, sizeof(T37));
							    printf("res:%d\n", res);
							}

							//uint8_t C1[3];
							//res = DaveIntfcReadBytes(Intfc, daveCounter200, 0, 1, 1, false, NULL);
							//res = DaveIntfcGetLastRecvData(Intfc, C1, sizeof(C1));
							//printf("res:%d C1:%d%d%d\n", res, C1[0], C1[1], C1[2]);

							//uint16_t VW1100;
							//VW1100 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 1100, sizeof(VW1100), false, &VW1100);
							//printf("res:%d VW1100:%d\n", res, daveGetTwoBytefrom(&VW1100));

							//uint16_t VW1102;
							//VW1102 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 1102, sizeof(VW1102), false, &VW1102);
							//printf("res:%d VW1102:%d\n", res, daveGetTwoBytefrom(&VW1102));

							//uint16_t VW1104;
							//VW1104 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 1104, sizeof(VW1104), false, &VW1104);
							//printf("res:%d VW1104:%d\n", res, daveGetTwoBytefrom(&VW1104));

							//uint16_t VW2140;
							//VW2140 = 1234567890, res = DaveIntfcWriteBytes(Intfc, daveDB, 1, 2140, sizeof(VW2140), &VW2140);
							////printf("res:%d VW2140:%d\n", res, VW2140);
							//VW2140 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 2140, sizeof(VW2140), false, &VW2140);
							//printf("res:%d VW2140:%d\n", res, daveGetTwoBytefrom(&VW2140));

							// Name##No 编译器拒绝 000832 参数为不合法的八进制。
							// #define GetPlcData(Type, Name, No, SwapIed, Res)\
									Type Name = 0; Res = DaveIntfcReadBytes(Intfc, daveDB, 1, No, sizeof(Name), false, &Name), Name = SwapIed(Name);

							// #define LogPutData(Type, Data, Result)\
											LogPut("Result: %d LogPutData "#Data":"#Type"\n", Result, Data)

							// #define SaveZwCtrl(StrNum, Name, StrLen)\
							//		sprintf(StrNum, "%0"#StrLen"u", Name), zw_cache_update((uint8_t *)StrNum, StrLen, (ZwSrc *)#Name)

							// int result = 0;
							// GetPlcData(uint32_t, VD000828, 828, DaveSwapIedFourByte, result);

							// LogPutData(%u, VD000828, result);

							// GetPlcData(uint16_t, VW001100, 1100, DaveSwapIedFourByte, result);

							// LogPutData(%hu, VW001100, result);

							{

								uint32_t VD100;
								VD100 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 100, sizeof(VD100), false, &VD100);
								VD100 = daveGetFourBytefrom(&VD100);
								float temp = VD100;
								memcpy(&temp, &VD100, 4);
								printf("res:%d VD100:%lf\n", res, temp);

							}
							//{
							//	uint32_t VD110;
							//	float temp;

							//	temp = 60;
							//	VD110 = daveGetFourBytefrom(&temp);
							//	res = DaveIntfcWriteBytes(Intfc, daveDB, 1, 110, sizeof(VD110), &VD110);
							//	printf("res:%d VD110:%lf\n", res, temp);

							//	VD110 = 0, res = DaveIntfcReadBytes(Intfc, daveDB, 1, 110, sizeof(VD110), false, &VD110);
							//	VD110 = daveGetFourBytefrom(&VD110);
							//	memcpy(&temp, &VD110, 4);
							//	printf("res:%d VD110:%lf\n", res, temp);
							//}
                        }
						system("pause");
					}
					DaveDel(free, Intfc);
				}
				DaveDel(free, DtSrc);
			}
			DaveFileClose(file);
		}
		WSACleanup();
	}
	return 0;
}
#endif
