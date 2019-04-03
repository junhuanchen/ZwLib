
#ifndef TCP_CLIENT_HPP
#define TCP_CLIENT_HPP

#include "../../FrameWork.hpp"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <MSWSock.h>
#pragma comment(lib, "WS2_32.lib")

class TcpClient
{
protected:
	SOCKET Socket;
	static const int RecvMax = 256;
	CHAR Buffer[RecvMax];

private:
	HANDLE Thread, Event;
	SOCKADDR_IN Addr;
	bool ClientExist;
	bool CnctAlive;

	virtual void DealRecv(char data[], int len)
	{
		PutInfo("TcpClient: %p recv data: %.*s\n", this, len, data);
	}

	virtual void DealCnct()
	{
		PutInfo("TcpClient: %p cnct data: %.*s\n", this);
	}

	VOID ConnectEvent(void)
	{
		INT Answer;
		while (ClientExist)
		{
			if (SOCKET_ERROR != Socket)
			{
				if (CnctAlive)
				{
					Answer = recv(Socket, Buffer, sizeof(Buffer), 0);
					// closesocket��TCP��,Զ�˻ᷴ��һ���հ�,���ض˻����һ��socket_error
					if (0 != Answer && SOCKET_ERROR != Answer)
					{
						DealRecv(Buffer, Answer);
					}
					else
					{
						// ����send����
						ResetEvent(Event);
						// �����Ѿ��Ͽ���TCP��socket�޷����ã��պ��ѯ��������TIME_WAIT״̬������DisConnect�������ã�
						closesocket(Socket), Socket = SOCKET_ERROR;
						CnctAlive = false; // ����������
						PutInfo("connceted to break, check error : %d\n", WSAGetLastError());
					}
				}
				else
				{
					PutInfo("conneting...\n");
					if (0 == connect(Socket, (SOCKADDR *) &Addr, sizeof(SOCKADDR)))
					{
						CnctAlive = true;
						// �ͷű�send�����Ĳ���
						SetEvent(Event);
						DealCnct();
					}
				}
			}
			else
			{
				Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			}
		}
		PutInfo("stoping conneted......\n");
		// �ͷű�send�����Ĳ���
		SetEvent(Event);
	}

	static DWORD WINAPI ConnectEventProc(LPVOID lpParam)
	{
		((TcpClient *) lpParam)->ConnectEvent();
		return 0;
	};

public:

	bool ConnectAlive(void)
	{
		return this->CnctAlive;
	}

	bool Startup()
	{
		ClientExist = false;
		Addr.sin_family = AF_INET;
		// ʹ���ֶ�����Ϊ���ź�״̬����ʼ��ʱ���ź�״̬
		return (NULL != (Event = CreateEvent(NULL, TRUE, FALSE, NULL)));
	}

	void Cleanup()
	{
		DisConnect();
		if (Event)
		{
			WaitForSingleObject(Event, INFINITE);
			CloseHandle(Event);
		}
	}

	bool Connect(LPCSTR IPAddr, USHORT Port)
	{
		if (!ClientExist)
		{
			Socket = SOCKET_ERROR, CnctAlive = false;
			Addr.sin_addr.S_un.S_addr = inet_addr(IPAddr), Addr.sin_port = htons(Port);
			ClientExist = (NULL != (Thread = CreateThread(NULL, 0, ConnectEventProc, (LPVOID)this, 0, NULL)));
		}
		return ClientExist;
	}

	bool DisConnect()
	{
		if (ClientExist)
		{
			shutdown(Socket, SD_SEND);
			CnctAlive = ClientExist = false;
			WaitForSingleObject(Thread, INFINITE);
			CloseHandle(Thread);
		}
		return !ClientExist;
	}

	bool Send(char data[], int len, DWORD dwMilliseconds)
	{
		if (WaitForSingleObject(Event, dwMilliseconds/*INFINITE*/) == WAIT_OBJECT_0)
		{
			if (CnctAlive && SOCKET_ERROR != send(Socket, data, len, 0))
			{
				return true;
			}
		}
		return false;
	}

};
#endif

// TODO
#ifdef UNIT_TEST

// �ȴ� processon �����öϵ���������

class UnitTestFileClient : public TcpClient
{
	void DealCnct()
	{
		Send("Request Connect!", sizeof("Request Connect!"), -1);
	}

	void DealRecv(char data[], int len)
	{
		// ȷ�ϴ��ڴ����ļ�����
		if (0 == memcmp(data, "FileRespond", sizeof("FileRespond")) && true == RecvdFileFlag)
		{
			// ȷ�Ͻ����ļ�
			RecvdFileFlag = false;
			// ��ó�ʱ����ֵ
			unsigned EachTimeOut = RecvdFileTimeOut;
			// �õ��ļ���Ϣ���ļ��� + �ļ�����
			
			while (true)
			{
				 
			}
		}
	};

	bool RecvdFileFlag = false;
	unsigned RecvdFileTimeOut = 0;
/*
	~UnitTestFileClient()
	{
		if (FileRecvdFlag) FileRecvdFlag = false;
	}
*/

	bool FileRequest(char * FileNameCmd, unsigned EachTimeOut)
	{
		// RecvdFileFlag Ϊ�ٱ�ʾ�̱߳�ռ�ã������ٽ�
		if (false == RecvdFileFlag)
		{
			RecvdFileFlag = true;
			// ���ô��䳬ʱ����
			RecvdFileTimeOut = EachTimeOut;
			// �����ļ�
			Send(FileNameCmd, strlen(FileNameCmd), -1);
			// �ȴ����պ���ȷ�ϴ��俪ʼ��
			while (RecvdFileFlag);
			// �ȴ�������ɻ򱻴��ÿ�δ��䶼�ᵼ�¼��������á�
			while (RecvdFileTimeOut--)
			{
				//  ��RecvdFileFlag�����Ϊ���ʾ�Ѿ����������
				if (true == RecvdFileFlag)
				{
					// ������ɣ��Ƴ�����ռ�ñ�ǡ�
					RecvdFileFlag = false;
					return true;
				}
			}
		}
		return false;
	}

	void FileRespond()
	{
		ULONG lReadSize = 0;
		HANDLE hFile = CreateFile("�ļ�", GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

		while (hFile != INVALID_HANDLE_VALUE)
		{
			int iSize = recv(Socket, Buffer, RecvMax, 0);

			if (iSize == SOCKET_ERROR || iSize == 0)
			{
				CloseHandle(hFile);
				break;
			}
			else if (iSize < RecvMax)
			{
				WriteFile(hFile, Buffer, iSize, &lReadSize, NULL);
				CloseHandle(hFile);
				break;
			}
		}

	}
};

#endif