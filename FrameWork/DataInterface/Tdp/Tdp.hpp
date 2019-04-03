
#ifndef TDP_H
#define TDP_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Winsock2.h"
#pragma comment(lib, "WS2_32.lib")

#ifdef _TDPCRITICALSECTION_
#include <windows.h>
#endif

#include "../../FrameWork.hpp"

#include "../../../Struct/Queue/QueueList.hpp"

template <class Pack> class Tdp
{
public:
	
	struct TdpFrame
	{
		SOCKADDR_IN Addr;
		int Len;
		Pack Var;
		TdpFrame() :Len(0)
		{
			;
		}
	};

private:

#ifdef _TDPCRITICALSECTION_
    CRITICAL_SECTION Cs;
    int ThreadSum;
    HANDLE AllotThread, *ExecThread;
#else
    HANDLE AllotThread, ExecThread;
#endif;

    SOCKET Listen;

	TemplateQueueList<TdpFrame> FrameQueue;

	void AllotEvent()
	{
		static INT Len = sizeof(SOCKADDR_IN);
		static fd_set readfds;
		static struct timeval tv = { 0, 1000 }; // 轮询频率 1000 microseconds
		TdpFrame RecvPack;
		FD_ZERO(&readfds);
		FD_SET(Listen, &readfds);
		select(Listen, &readfds, NULL, NULL, &tv);
		if (FD_ISSET(Listen, &readfds))
		{
			RecvPack.Len = recvfrom(Listen, (char *) &RecvPack.Var, sizeof(RecvPack.Var), 0, (SOCKADDR*) &RecvPack.Addr, &Len);
			// 非阻塞接收UDP包,中途发生错误则返回SOCKET_ERROR
			if (SOCKET_ERROR != RecvPack.Len)
			{
				// 接收到的UDP数据包转移到队列缓冲中
				FrameQueue.Push(RecvPack);
			}
			else
			{
				PutError("Tdp Tx Error!");
			}
		}
	}

	static DWORD WINAPI ExecProc(LPVOID lpParam)
	{
		Tdp *Self = (Tdp*) lpParam;
		TdpFrame frame;
		while (INVALID_SOCKET != Self->Listen)
		{
#ifdef _TDPCRITICALSECTION_
			EnterCriticalSection(&Self->Cs);
#endif
			if (Self->FrameQueue.Exist())
			{
				frame = *Self->FrameQueue.GetFront();
				Self->FrameQueue.Pop();
			}
#ifdef _TDPCRITICALSECTION_
			LeaveCriticalSection(&Self->Cs);
#endif
			if (frame.Len > 0)
			{
				Self->TdpRecvProcess(frame);
				frame.Len = 0;
			}

			Sleep(1);
		}
		return 0;
	}

	static DWORD WINAPI AllotProc(LPVOID lpParam)
	{
		Tdp *Self = (Tdp*) lpParam;
		while (INVALID_SOCKET != Self->Listen)
		{
			Self->AllotEvent();
		}
		return 0;
	}

public:

	// 模板类的纯虚函数才可被继承，或虚函数实体，可被覆盖。
	virtual void TdpRecvProcess(TdpFrame & Packet) = 0;

#ifdef _TDPCRITICALSECTION_
	bool StartServer(USHORT port, INT sum)
#else
	bool StartServer(USHORT port)
#endif
	{
		// 初始化UDP监听端口
		SOCKADDR_IN Addr;
		Addr.sin_family = AF_INET;						// 绑定服务端地址族
		Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);	// 绑定服务端IP地址 
		Addr.sin_port = htons(port);					// 绑定服务端端口号
		// 创建Tdp数据链
		if (FrameQueue.New())
		{
			// 创建监听套接字
			if (INVALID_SOCKET != (Listen = socket(AF_INET, SOCK_DGRAM, 0)))
			{
				if (SOCKET_ERROR != bind(Listen, (SOCKADDR*) &Addr, sizeof(SOCKADDR)))
				{
					if (NULL != (AllotThread = CreateThread(NULL, 0, AllotProc, (LPVOID)this, 0, NULL)))
					{
#ifdef _TDPCRITICALSECTION_
						InitializeCriticalSection(&Cs);
						if (NULL != (ExecThread = new HANDLE[sum]()))
						{
							for (ThreadSum = 0; ThreadSum != sum; ThreadSum++)
							{
								if (NULL == (ExecThread[ThreadSum] = CreateThread(NULL, 0, ExecProc, (LPVOID)this, 0, NULL)))
								{
									break;
								}
							}
							if (ThreadSum == sum)
							{
								return true;
							}
							else
							{
								while (ThreadSum--)
								{
									CloseHandle(ExecThread[ThreadSum]);
								}
							}
							delete[] ExecThread;
						}
						DeleteCriticalSection(&Cs);
#else
						if (NULL != (ExecThread = CreateThread(NULL, 0, ExecProc, (LPVOID)this, 0, NULL)))
						{
							return true;
						}
#endif
						CloseHandle(AllotThread);
					}
				}
				// 关闭监听套接字
				closesocket(Listen), Listen = INVALID_SOCKET;
			}
			FrameQueue.Del();
		}
		return false;
	}

	void StopServer()
	{
		if (INVALID_SOCKET != Listen)
		{
			// 关闭监听套接字
			closesocket(Listen), Listen = INVALID_SOCKET;
			WaitForSingleObject(AllotThread, INFINITE);
			CloseHandle(AllotThread);
#ifdef _TDPCRITICALSECTION_
			// 等待所有线程结束
			WaitForMultipleObjects(ThreadSum, ExecThread, TRUE, INFINITE);
			while (ThreadSum--)
			{
				CloseHandle(ExecThread[ThreadSum]);
			}
			delete[] ExecThread;
			DeleteCriticalSection(&Cs);
#else
			WaitForSingleObject(ExecThread, INFINITE);
			CloseHandle(ExecThread);
			FrameQueue.Del();
#endif
		}
	}

	int SendTo(SOCKADDR_IN & Addr, Pack & Data, int Len)
	{
		return Len == sendto(Listen, (char *) &Data, Len, 0, (SOCKADDR*) &Addr, sizeof(SOCKADDR));
	}

};

#endif // TDP_H

#ifdef UNIT_TEST
#include <stdlib.h>
#include <assert.h>

namespace UnitTestTdp
{
	static void ParseDomain(const char * Name)
	{
		HOSTENT *iphost;
		if (NULL != (iphost = gethostbyname(Name)))
		{
			for (int i = 0; iphost->h_addr_list[i]; i++)
			{
				printf("%d : %.*s\n", i, 20, inet_ntoa(*((struct in_addr *)iphost->h_addr_list[i])));
			}
		}
	}

	struct TestBuf
	{
		CHAR Buf[64];
		TestBuf()
		{
			memset(Buf, 0, sizeof(Buf));
		}
		TestBuf(char * str)
		{
			strcpy(Buf, str);
		}
	};

	class TestTdp : public Tdp<TestBuf>
	{
		void TdpRecvProcess(TdpFrame & Packet)
		{
			// assert(0 == memcmp(Packet.obj, TestServer::TestStr, sizeof(Packet.obj)));
			printf("%.*s\n", Packet.Len, Packet.Var.Buf);
		}
	};

	static int main()
	{
		WSADATA wsaData;
		if (0 == WSAStartup(MAKEWORD(0x02, 0x02), &wsaData))
		{
			TestTdp tdp;
			while(1)
			{
			
		#ifdef _TDPCRITICALSECTION_
				assert(true == tdp.StartServer(Port, 2));
		#else
				assert(true == tdp.StartServer(9955));
		#endif // _TDPCRITICALSECTION_

				SOCKADDR_IN Addr;
				Addr.sin_family = AF_INET;				// 服务端地址族
				Addr.sin_addr.s_addr = inet_addr("127.0.0.1");	// 绑定服务端IP地址
				Addr.sin_port = htons(9955);			// 绑定服务端端口号
				TestBuf ts("this is a data!");
				assert(SOCKET_ERROR != tdp.SendTo(Addr, ts, strlen("this is a data!")));
				system("pause");
				tdp.StopServer();
			}
			WSACleanup();
		}
		return 0;
	}
}

#endif 