
#ifndef _WIN_USB_TO_UDP_
#define _WIN_USB_TO_UDP_

#include "../DataInterface/usb/usbhid.hpp"
#include "../DataInterface/Tdp/Tdp.hpp"
#include "WinTran.hpp"

struct WinTranHid : public UsbHid
{
    USHORT PID, VID, PVN;	// USB 设备连接标识
	void InitCnct( USHORT PID, USHORT VID, USHORT PVN )
	{
		this->PID = PID, this->VID = VID, this->PVN = PVN;
	}

	bool CheckCnct()
	{
		return UsbHid::DevState & UsbHid::State::Open;
	}

	bool TryCnct()
	{
		// 搜索HID设备
		if ( ( UsbHid::State::Find ) & UsbHid::FindDev( PID, VID, PVN ) )
		{
			// 重置驱动
			UsbHid::ReSetDev();
			// 连接HID设备
			if ( ( UsbHid::State::Open | UsbHid::State::Read | UsbHid::State::Write ) & UsbHid::OpenDev() )
			{
				PutInfo( "指定的USB设备已连接成功\n" );
			}
			else
			{
				PutInfo( "无法打开指定的USB设备，请拔插重试或更换USB设备\n" );
			}
		}
		else
		{
			PutInfo( "搜索不到指定的设备，请检查设备驱动是否已安装或设备是否已插上\n" );
		}
		return false;
	}
};

struct WinTranTdp : public Tdp<WinTranPack>
{
    SOCKADDR_IN WTAddr;
    USHORT RemotePort;
    // 初始化主机地址
    void InitAddr( char *RemoteIP, USHORT RemotePort )
	{
		memset( &WTAddr, 0, sizeof( WTAddr ) );
		this->RemotePort = RemotePort;
		WTAddr.sin_family = AF_INET;						// 选择地址族
		WTAddr.sin_addr.S_un.S_addr = inet_addr( RemoteIP );	// 服务端的IP地址
		WTAddr.sin_port = htons( RemotePort );				// 服务端的端口号
	}

	// 确认是否是主机信息
	bool ConfirmAddr( SOCKADDR_IN & Addr )
	{
		return ( 0 == memcmp( &Addr, &WTAddr, sizeof( WTAddr ) ) );
	}
};

struct WinTranUsbToTdp : public WinTranTask, public WinTranHid, public WinTranTdp
{
	struct usb_pack : public WinTranPack
	{
		uint32_t Len;
	};

    int WinTranStartupState;		// 采集服务启动请求（默认值：禁止）
	TemplateQueueList<usb_pack> WinTranBuffer;	// 数据采集缓冲区
    bool RelayFlag;		// 缓冲区发送标记（默认值：允许）
	USHORT LocalPort;			// Tdp 服务端口

	WinTranUsbToTdp::WinTranUsbToTdp()
	{
		ZwTranInit();
		RelayFlag = true, WinTranStartupState = true;
	}

	// 回调处理Tdp接收的数据
	void WinTranUsbToTdp::TdpRecvProcess( TdpFrame & Packet )
	{
		PutInfo( "tdp - ip : %s port : %hu\n",
			inet_ntoa( Packet.Addr.sin_addr ), ntohs( Packet.Addr.sin_port ) );
		// 锁定双方的IP与端口号 
		if ( 0 == memcmp( &Packet.Addr, &WTAddr, sizeof( WTAddr ) ) )
		{
			WinTranPack *Data = ( WinTranPack * ) &Packet.Var;
			// 确认是否是本机的上传数据时请求的应答包
			if ( false == RelayFlag && WinTranBuffer.Exist())
			{
				// usb_pack * pack = WinTranBuffer.GetFront();
				// if ( 0 == memcmp( pack->buf, Packet.Var.buf, pack->Len ) )
				{
					WinTranBuffer.Pop(), RelayFlag = true;
				}

				if (UsbHid::WriteData(Packet.Var.buf, Packet.Len))
				{
					// 缓冲数据成功并返回应答
				}
				else
				{
					if (UsbHid::ErrWriteTmOut == UsbHid::DevError)
					{
						// 终止程序（TODO：分离 USB 重启）
						exit(TRUE);
					}
					else
					{
						//  其他错误
					}
				}
			}
			else
			{
				// 异常外来包，危害等级中
			}
		}
		else
		{
			// 外来数据源，记录，危害等级低
		}
	}

	// 取代默认的处理USB接收的数据
	void WinTranUsbToTdp::UsbRecvProcess( UCHAR buf[] )
	{
		UCHAR *bufpos = buf;
		PutInfo( "Recv Usb : %.*s\n", 1 /*USB_MAX*/, bufpos );

		// 校验 USB 数据
		usb_pack pack;
		pack.Len = strnlen( ( char * ) bufpos, USB_MAX );
		memcpy( pack.buf, bufpos, pack.Len );
		if ( 0 == ZwTranCheck( pack.buf, pack.Len ) )
		{
			if ( NULL != WinTranBuffer.Push( pack ) )
			{
				;
			}
			else
			{
				// 内存不足，此时可能的原因有Usb超出Tdp传输速度太多导致的缓冲不足，此时Tdp速度也有可能为零，即断开。
				PutError( "Usb To Tdp MemError!" );
				// 转移到外存缓冲
			}
		}
	}

	// 回调处理程序后台任务
	void WinTranUsbToTdp::BackgroundProcess()
	{
		// 重试计数器
		static uint8_t time;

		// 检查采集服务启动情况
		if ( WinTranStartupState )
		{
			// 上传USB数据
			if ( WinTranBuffer.Exist() && ( 0 == time || RelayFlag ) )
			{
				usb_pack * pack = WinTranBuffer.GetFront();
				if ( true == WinTranTdp::SendTo( WTAddr, *pack, pack->Len ) )
				{
					// 移除发送标记后发送数据
					RelayFlag = false;
				}
				else
				{
					PutError( "Tdp Send Error!" );

					static UCHAR TdpErrorSendCount = 1;

					// 达到十次发送错误重启Tdp服务
					if ( 0 == TdpErrorSendCount++ )
					{
						// 重启 Tdp 服务
						WinTranTdp::StopServer();
						WinTranTdp::StartServer( LocalPort );
					}
				}
			}
		}
		else
		{
			// 未请求则进行请求，请求失败过多则退出程序，并删除配置文件
			WinTranStartupState = true;
		}

		// 检测 USB 设备的状态
		if ( 0 == time )
		{
			if ( WinTranHid::CheckCnct() )
			{
				;
			}
			else
			{
				// 未连接USB 设备
				WinTranHid::TryCnct();
			}
		}

		// 检查外存数据

		// 后台任务执行间隔
		time++, Sleep( 5 );
	}

	bool WinTranUsbToTdp::StartUp( CHAR *RemoteIP, USHORT RemotePort, USHORT LocalPort, USHORT PID, USHORT VID, USHORT PVN )
	{
		// 初始化HID设备标识
		WinTranHid::InitCnct( PID, VID, PVN );
		// 初始化连接主机类
		WinTranTdp::InitAddr( RemoteIP, RemotePort );
		// 初始化缓冲队列
		if ( WinTranBuffer.New() )
		{
			// 启动Tdp服务
			this->LocalPort = LocalPort;
			if ( WinTranTdp::StartServer( LocalPort ) )
			{
				// 启动UsbHid环境
				if ( WinTranHid::Startup() )
				{
					// 启动后台线程服务
					if ( WinTranTask::Start() )
					{
						return true;
					}
					else
					{
						PutError( "后台线程服务启动失败，无法在系统中创建更多的线程\n" );
					}
					WinTranHid::Cleanup();
				}
				else
				{
					PutError( "USB-HID服务启动失败\n" );
				}
				WinTranTdp::StopServer();
			}
			else
			{
				PutError( "UDP网络服务启动失败\n" );
			}
		}
		else
		{
			PutError( "数据采集程序申请内存失败\n" );
		}
		return false;
	}

	void WinTranUsbToTdp::CleanUp()
	{
		WinTranTask::Stop();
		WinTranHid::Cleanup();
		WinTranTdp::StopServer();
	}

};

#ifdef UNIT_TEST
EXTERN_C
{
#include "../ZwLib/ZwTransit.h"
}

#include <time.h>

void GetTime( uint32_t *sec, uint16_t *ms )
{
	*sec = time( NULL ), *ms = GetTickCount() % 1000;
}

int CollectServer()
{
	// 启动WINDOWS网络环境
	WSADATA wsaData;
	if ( 0 == WSAStartup( MAKEWORD( 0x02, 0x02 ), &wsaData ) )
	{
		WinTranUsbToTdp Sys;
		if ( Sys.StartUp( "127.0.0.1", 9954, 9955, 0x9990, 0x0666, 0x0200 ) )
		{
			PutInfo( "数据采集服务启动成功，按任意键停止服务\n" );
			/*
			const uint8_t RsaDe = 43, RsaEn = 55;
			ZwTranInit();
			ZwEncode zp;
			ZwEncodeInit(&zp, RsaDe, 0xFF, (uint8_t *)"0123456789ABCDEF", 0, GetTime);
			ZwDecode zup;
			ZwDecodeInit(&zup, RsaDe);

			while (0)
			{
			WinTranPack wtp;
			uint8_t data[6], packlen;
			sprintf((char *) data, "%05d", rand());
			packlen = ZwEncodeCollect(&zp, wtp.buffer, 8, (uint8_t *)"DM123456", 5, data);
			// Sys.WriteData(wtp.buffer);
			Sys.SendTo(Sys.WTAddr, wtp, packlen);
			Sleep(100);
			}
			*/
			system( "pause > nul" );
			Sys.CleanUp();
		}
		else
		{
			PutError( "数据采集服务启动失败" );
		}
		WSACleanup();
	}
	else
	{
		PutError( "网络环境启动失败，正尝试重置网络" );
		system( "netsh winsock reset" );
	}
	return 0;
}

#include <crtdbg.h> // 检测内存泄漏

int main()
{
	_CrtDumpMemoryLeaks();
	// 启动采集服务
	CollectServer();
	// 检测内存是否泄漏，请放在程序最终执行位置而不是函数
	_CrtDumpMemoryLeaks();
	return 0;
}

#endif // !UNIT_TEST

#endif