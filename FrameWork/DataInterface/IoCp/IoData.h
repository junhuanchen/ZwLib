
#ifndef IODATA_H
#define IODATA_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>

#include <MSWSock.h>

#pragma comment(lib, "WS2_32.lib")

struct IoData
{
	// IoData 常量
	enum
	{
		SockAddrSize = sizeof(SOCKADDR_IN) + 16,
		IoDataBufSize = 128,   // IoData 有效数据区长度(固定)
		IoDataSize = (IoDataBufSize + (2 * (sizeof(SOCKADDR_IN) + 16))),
		StateAccept = 'A',      // IoData 初始态（等待客户端接入）
		StateRecv = 'R',        // IoData 接收态（允许进行数据交互）
		AliveCycle = 1000,     // IoData 中 Client 的生存周期(ms)
	};

	// 函数Accept让监听端口接受客户的连接请求,依赖Listen监听套接字,如果第四参数设置为0则立刻返回在IoCp队列中响应
	static LPFN_ACCEPTEX Accept;
	// 函数DisconnectEx分别执行shutdown和disconnect后重置socket以供Accept函数重用
	static LPFN_DISCONNECTEX DisconnectEx;
	// 函数GetSocketAddrs获取套接字末端存储的双方地址信息
	static LPFN_GETACCEPTEXSOCKADDRS GetAcceptSockAddrs;

	SOCKET Socket;				// 套接字SOCKET
	DWORD RecvByte;				// 接收到的字节数
	DWORD RecvFlag;				// modify the behavior of the WSARecv function call
	time_t LastExecTime;		// 上次操作时间
	WSABUF Buffer;				// 数据缓冲区
	OVERLAPPED OverlapIO;		// 重叠IO结构
	UCHAR State;				// Socket状态
	CHAR DATA[IoDataSize];		// 固定数据区

	// 获取IoData中两端的地址信息（ip或port等）
	inline void GetAddr(PSOCKADDR_IN & clientAddr, PSOCKADDR_IN & localAddr)
	{
		static INT clientLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
		GetAcceptSockAddrs(Buffer.buf, Buffer.len, SockAddrSize, SockAddrSize,
			(LPSOCKADDR *)&localAddr, &localLen, (LPSOCKADDR *)&clientAddr, &clientLen);
	}

	// 初始化IoData
	inline BOOL Init(SOCKET & listenPort)
	{
		Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET != Socket)
		{
			RecvByte = RecvFlag = 0, Buffer.buf = DATA, Buffer.len = IoDataBufSize;
			ZeroMemory(&OverlapIO, sizeof(OVERLAPPED));
			if (TRUE == Accept(listenPort, Socket, Buffer.buf, Buffer.len, SockAddrSize, SockAddrSize, &RecvByte, &OverlapIO)
				|| ERROR_IO_PENDING == WSAGetLastError())
			{
				LastExecTime = GetTickCount64();
				State = StateAccept;
				return TRUE;
			}
			closesocket(Socket);
		}
		return FALSE;
	}
	// 重用IoData
	inline void Reuse(SOCKET & listenPort)
	{
		// 记录当前操作的执行时间
		LastExecTime = GetTickCount64();
		State = StateAccept, RecvFlag = 0, DisconnectEx(Socket, NULL, TF_REUSE_SOCKET, 0);
		Accept(listenPort, Socket, Buffer.buf, Buffer.len, SockAddrSize, SockAddrSize, &RecvByte, &OverlapIO);
	}
	// 异步请求IoData接收数据（RecvLen : 请求接收多少字节数(这不意味着一定收到指定长度）
	inline int Recv(DWORD RecvLen)
	{
		// 记录当前操作的执行时间
		LastExecTime = GetTickCount64();
		return WSARecv(Socket, &Buffer, RecvLen, &RecvByte, &RecvFlag, &OverlapIO, NULL);
	}
	// 销毁工作（绝对的）
	~IoData()
	{
		UnInit();
	}
	// 反初始化IoData
	inline void UnInit()
	{
		shutdown(Socket, SD_SEND), closesocket(Socket);
	}
	// 检查连接时长是否超时
	inline bool OverTimeConnect()
	{
		DWORD nSecs = 0;
		static int nBytes = sizeof(nBytes);
		getsockopt(Socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&nSecs, &nBytes);
		return -1 != nSecs ? (1000 * nSecs) > AliveCycle : false;
	}
	// 检查上次操作间隔是否超时
	inline bool OverTimeLastExec()
	{
		return (GetTickCount64() - LastExecTime) > AliveCycle;
	}

};
#endif // IODATA_H