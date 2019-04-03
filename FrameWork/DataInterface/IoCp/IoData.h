
#ifndef IODATA_H
#define IODATA_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>

#include <MSWSock.h>

#pragma comment(lib, "WS2_32.lib")

struct IoData
{
	// IoData ����
	enum
	{
		SockAddrSize = sizeof(SOCKADDR_IN) + 16,
		IoDataBufSize = 128,   // IoData ��Ч����������(�̶�)
		IoDataSize = (IoDataBufSize + (2 * (sizeof(SOCKADDR_IN) + 16))),
		StateAccept = 'A',      // IoData ��ʼ̬���ȴ��ͻ��˽��룩
		StateRecv = 'R',        // IoData ����̬������������ݽ�����
		AliveCycle = 1000,     // IoData �� Client ����������(ms)
	};

	// ����Accept�ü����˿ڽ��ܿͻ�����������,����Listen�����׽���,������Ĳ�������Ϊ0�����̷�����IoCp��������Ӧ
	static LPFN_ACCEPTEX Accept;
	// ����DisconnectEx�ֱ�ִ��shutdown��disconnect������socket�Թ�Accept��������
	static LPFN_DISCONNECTEX DisconnectEx;
	// ����GetSocketAddrs��ȡ�׽���ĩ�˴洢��˫����ַ��Ϣ
	static LPFN_GETACCEPTEXSOCKADDRS GetAcceptSockAddrs;

	SOCKET Socket;				// �׽���SOCKET
	DWORD RecvByte;				// ���յ����ֽ���
	DWORD RecvFlag;				// modify the behavior of the WSARecv function call
	time_t LastExecTime;		// �ϴβ���ʱ��
	WSABUF Buffer;				// ���ݻ�����
	OVERLAPPED OverlapIO;		// �ص�IO�ṹ
	UCHAR State;				// Socket״̬
	CHAR DATA[IoDataSize];		// �̶�������

	// ��ȡIoData�����˵ĵ�ַ��Ϣ��ip��port�ȣ�
	inline void GetAddr(PSOCKADDR_IN & clientAddr, PSOCKADDR_IN & localAddr)
	{
		static INT clientLen = sizeof(SOCKADDR_IN), localLen = sizeof(SOCKADDR_IN);
		GetAcceptSockAddrs(Buffer.buf, Buffer.len, SockAddrSize, SockAddrSize,
			(LPSOCKADDR *)&localAddr, &localLen, (LPSOCKADDR *)&clientAddr, &clientLen);
	}

	// ��ʼ��IoData
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
	// ����IoData
	inline void Reuse(SOCKET & listenPort)
	{
		// ��¼��ǰ������ִ��ʱ��
		LastExecTime = GetTickCount64();
		State = StateAccept, RecvFlag = 0, DisconnectEx(Socket, NULL, TF_REUSE_SOCKET, 0);
		Accept(listenPort, Socket, Buffer.buf, Buffer.len, SockAddrSize, SockAddrSize, &RecvByte, &OverlapIO);
	}
	// �첽����IoData�������ݣ�RecvLen : ������ն����ֽ���(�ⲻ��ζ��һ���յ�ָ�����ȣ�
	inline int Recv(DWORD RecvLen)
	{
		// ��¼��ǰ������ִ��ʱ��
		LastExecTime = GetTickCount64();
		return WSARecv(Socket, &Buffer, RecvLen, &RecvByte, &RecvFlag, &OverlapIO, NULL);
	}
	// ���ٹ��������Եģ�
	~IoData()
	{
		UnInit();
	}
	// ����ʼ��IoData
	inline void UnInit()
	{
		shutdown(Socket, SD_SEND), closesocket(Socket);
	}
	// �������ʱ���Ƿ�ʱ
	inline bool OverTimeConnect()
	{
		DWORD nSecs = 0;
		static int nBytes = sizeof(nBytes);
		getsockopt(Socket, SOL_SOCKET, SO_CONNECT_TIME, (char*)&nSecs, &nBytes);
		return -1 != nSecs ? (1000 * nSecs) > AliveCycle : false;
	}
	// ����ϴβ�������Ƿ�ʱ
	inline bool OverTimeLastExec()
	{
		return (GetTickCount64() - LastExecTime) > AliveCycle;
	}

};
#endif // IODATA_H