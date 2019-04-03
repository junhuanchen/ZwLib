
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
					// closesocket在TCP下,远端会反馈一个空包,本地端会产生一个socket_error
					if (0 != Answer && SOCKET_ERROR != Answer)
					{
						DealRecv(Buffer, Answer);
					}
					else
					{
						// 阻塞send操作
						ResetEvent(Event);
						// 连接已经断开，TCP的socket无法重用（日后查询快速消除TIME_WAIT状态在利用DisConnect即可重用）
						closesocket(Socket), Socket = SOCKET_ERROR;
						CnctAlive = false; // 连接已死亡
						PutInfo("connceted to break, check error : %d\n", WSAGetLastError());
					}
				}
				else
				{
					PutInfo("conneting...\n");
					if (0 == connect(Socket, (SOCKADDR *) &Addr, sizeof(SOCKADDR)))
					{
						CnctAlive = true;
						// 释放被send阻塞的操作
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
		// 释放被send阻塞的操作
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
		// 使用手动重置为无信号状态，初始化时无信号状态
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

// 等待 processon 描述好断点续传功能

class UnitTestFileClient : public TcpClient
{
	void DealCnct()
	{
		Send("Request Connect!", sizeof("Request Connect!"), -1);
	}

	void DealRecv(char data[], int len)
	{
		// 确认存在传输文件请求
		if (0 == memcmp(data, "FileRespond", sizeof("FileRespond")) && true == RecvdFileFlag)
		{
			// 确认接收文件
			RecvdFileFlag = false;
			// 获得超时计数值
			unsigned EachTimeOut = RecvdFileTimeOut;
			// 得到文件信息：文件名 + 文件长度
			
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
		// RecvdFileFlag 为假表示线程被占用，简易临界
		if (false == RecvdFileFlag)
		{
			RecvdFileFlag = true;
			// 设置传输超时计数
			RecvdFileTimeOut = EachTimeOut;
			// 请求文件
			Send(FileNameCmd, strlen(FileNameCmd), -1);
			// 等待接收函数确认传输开始。
			while (RecvdFileFlag);
			// 等待接收完成或被打断每次传输都会导致计数器重置。
			while (RecvdFileTimeOut--)
			{
				//  当RecvdFileFlag被标记为真表示已经传输结束。
				if (true == RecvdFileFlag)
				{
					// 传输完成，移除函数占用标记。
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
		HANDLE hFile = CreateFile("文件", GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

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