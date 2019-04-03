
#ifndef _WIN_USB_TO_UDP_
#define _WIN_USB_TO_UDP_

#include "../DataInterface/usb/usbhid.hpp"
#include "../DataInterface/Tdp/Tdp.hpp"
#include "WinTran.hpp"

struct WinTranHid : public UsbHid
{
    USHORT PID, VID, PVN;	// USB �豸���ӱ�ʶ
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
		// ����HID�豸
		if ( ( UsbHid::State::Find ) & UsbHid::FindDev( PID, VID, PVN ) )
		{
			// ��������
			UsbHid::ReSetDev();
			// ����HID�豸
			if ( ( UsbHid::State::Open | UsbHid::State::Read | UsbHid::State::Write ) & UsbHid::OpenDev() )
			{
				PutInfo( "ָ����USB�豸�����ӳɹ�\n" );
			}
			else
			{
				PutInfo( "�޷���ָ����USB�豸����β����Ի����USB�豸\n" );
			}
		}
		else
		{
			PutInfo( "��������ָ�����豸�������豸�����Ƿ��Ѱ�װ���豸�Ƿ��Ѳ���\n" );
		}
		return false;
	}
};

struct WinTranTdp : public Tdp<WinTranPack>
{
    SOCKADDR_IN WTAddr;
    USHORT RemotePort;
    // ��ʼ��������ַ
    void InitAddr( char *RemoteIP, USHORT RemotePort )
	{
		memset( &WTAddr, 0, sizeof( WTAddr ) );
		this->RemotePort = RemotePort;
		WTAddr.sin_family = AF_INET;						// ѡ���ַ��
		WTAddr.sin_addr.S_un.S_addr = inet_addr( RemoteIP );	// ����˵�IP��ַ
		WTAddr.sin_port = htons( RemotePort );				// ����˵Ķ˿ں�
	}

	// ȷ���Ƿ���������Ϣ
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

    int WinTranStartupState;		// �ɼ�������������Ĭ��ֵ����ֹ��
	TemplateQueueList<usb_pack> WinTranBuffer;	// ���ݲɼ�������
    bool RelayFlag;		// ���������ͱ�ǣ�Ĭ��ֵ������
	USHORT LocalPort;			// Tdp ����˿�

	WinTranUsbToTdp::WinTranUsbToTdp()
	{
		ZwTranInit();
		RelayFlag = true, WinTranStartupState = true;
	}

	// �ص�����Tdp���յ�����
	void WinTranUsbToTdp::TdpRecvProcess( TdpFrame & Packet )
	{
		PutInfo( "tdp - ip : %s port : %hu\n",
			inet_ntoa( Packet.Addr.sin_addr ), ntohs( Packet.Addr.sin_port ) );
		// ����˫����IP��˿ں� 
		if ( 0 == memcmp( &Packet.Addr, &WTAddr, sizeof( WTAddr ) ) )
		{
			WinTranPack *Data = ( WinTranPack * ) &Packet.Var;
			// ȷ���Ƿ��Ǳ������ϴ�����ʱ�����Ӧ���
			if ( false == RelayFlag && WinTranBuffer.Exist())
			{
				// usb_pack * pack = WinTranBuffer.GetFront();
				// if ( 0 == memcmp( pack->buf, Packet.Var.buf, pack->Len ) )
				{
					WinTranBuffer.Pop(), RelayFlag = true;
				}

				if (UsbHid::WriteData(Packet.Var.buf, Packet.Len))
				{
					// �������ݳɹ�������Ӧ��
				}
				else
				{
					if (UsbHid::ErrWriteTmOut == UsbHid::DevError)
					{
						// ��ֹ����TODO������ USB ������
						exit(TRUE);
					}
					else
					{
						//  ��������
					}
				}
			}
			else
			{
				// �쳣��������Σ���ȼ���
			}
		}
		else
		{
			// ��������Դ����¼��Σ���ȼ���
		}
	}

	// ȡ��Ĭ�ϵĴ���USB���յ�����
	void WinTranUsbToTdp::UsbRecvProcess( UCHAR buf[] )
	{
		UCHAR *bufpos = buf;
		PutInfo( "Recv Usb : %.*s\n", 1 /*USB_MAX*/, bufpos );

		// У�� USB ����
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
				// �ڴ治�㣬��ʱ���ܵ�ԭ����Usb����Tdp�����ٶ�̫�ർ�µĻ��岻�㣬��ʱTdp�ٶ�Ҳ�п���Ϊ�㣬���Ͽ���
				PutError( "Usb To Tdp MemError!" );
				// ת�Ƶ���滺��
			}
		}
	}

	// �ص���������̨����
	void WinTranUsbToTdp::BackgroundProcess()
	{
		// ���Լ�����
		static uint8_t time;

		// ���ɼ������������
		if ( WinTranStartupState )
		{
			// �ϴ�USB����
			if ( WinTranBuffer.Exist() && ( 0 == time || RelayFlag ) )
			{
				usb_pack * pack = WinTranBuffer.GetFront();
				if ( true == WinTranTdp::SendTo( WTAddr, *pack, pack->Len ) )
				{
					// �Ƴ����ͱ�Ǻ�������
					RelayFlag = false;
				}
				else
				{
					PutError( "Tdp Send Error!" );

					static UCHAR TdpErrorSendCount = 1;

					// �ﵽʮ�η��ʹ�������Tdp����
					if ( 0 == TdpErrorSendCount++ )
					{
						// ���� Tdp ����
						WinTranTdp::StopServer();
						WinTranTdp::StartServer( LocalPort );
					}
				}
			}
		}
		else
		{
			// δ�����������������ʧ�ܹ������˳����򣬲�ɾ�������ļ�
			WinTranStartupState = true;
		}

		// ��� USB �豸��״̬
		if ( 0 == time )
		{
			if ( WinTranHid::CheckCnct() )
			{
				;
			}
			else
			{
				// δ����USB �豸
				WinTranHid::TryCnct();
			}
		}

		// ����������

		// ��̨����ִ�м��
		time++, Sleep( 5 );
	}

	bool WinTranUsbToTdp::StartUp( CHAR *RemoteIP, USHORT RemotePort, USHORT LocalPort, USHORT PID, USHORT VID, USHORT PVN )
	{
		// ��ʼ��HID�豸��ʶ
		WinTranHid::InitCnct( PID, VID, PVN );
		// ��ʼ������������
		WinTranTdp::InitAddr( RemoteIP, RemotePort );
		// ��ʼ���������
		if ( WinTranBuffer.New() )
		{
			// ����Tdp����
			this->LocalPort = LocalPort;
			if ( WinTranTdp::StartServer( LocalPort ) )
			{
				// ����UsbHid����
				if ( WinTranHid::Startup() )
				{
					// ������̨�̷߳���
					if ( WinTranTask::Start() )
					{
						return true;
					}
					else
					{
						PutError( "��̨�̷߳�������ʧ�ܣ��޷���ϵͳ�д���������߳�\n" );
					}
					WinTranHid::Cleanup();
				}
				else
				{
					PutError( "USB-HID��������ʧ��\n" );
				}
				WinTranTdp::StopServer();
			}
			else
			{
				PutError( "UDP�����������ʧ��\n" );
			}
		}
		else
		{
			PutError( "���ݲɼ����������ڴ�ʧ��\n" );
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
	// ����WINDOWS���绷��
	WSADATA wsaData;
	if ( 0 == WSAStartup( MAKEWORD( 0x02, 0x02 ), &wsaData ) )
	{
		WinTranUsbToTdp Sys;
		if ( Sys.StartUp( "127.0.0.1", 9954, 9955, 0x9990, 0x0666, 0x0200 ) )
		{
			PutInfo( "���ݲɼ����������ɹ����������ֹͣ����\n" );
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
			PutError( "���ݲɼ���������ʧ��" );
		}
		WSACleanup();
	}
	else
	{
		PutError( "���绷������ʧ�ܣ���������������" );
		system( "netsh winsock reset" );
	}
	return 0;
}

#include <crtdbg.h> // ����ڴ�й©

int main()
{
	_CrtDumpMemoryLeaks();
	// �����ɼ�����
	CollectServer();
	// ����ڴ��Ƿ�й©������ڳ�������ִ��λ�ö����Ǻ���
	_CrtDumpMemoryLeaks();
	return 0;
}

#endif // !UNIT_TEST

#endif