// #pragma once

#ifndef IoCp_H
#define IoCp_H

#include "iodata.h"

#include "../../../Struct/Queue/QueueList.hpp"

// ��־���
#include "../../FrameWork.hpp"

class IoCp
{
public:
	enum
	{
		PreBufClient = 4,   // ÿ�λ��崦��ͻ��˵��׽���
		FlagExit = -'E',    // IoCp ģ���˳����
		BusinessCmdLen = 1, // Ԥ���ҵ���߼����յ���ָ���
	};

	// ����IoCpģ��
	BOOL StartUp(LONG ThreadSum, USHORT port);

	// ����IoCpģ��
	void CleanUp();

protected:

	// ��֤���Ӽ̳нӿ�
	virtual BOOL CheckCnct(IoData * Data);

	// ���䴦��̳нӿ�
	virtual void Business(IoData * Data);

	LONG ClientNum, ClientSum;	    // ���ӵĿͻ�����Ŀ���׽������ֵ
    TemplateQueueList< IoData > ClientQueue;// �ͻ����׽��ֶ���
	LONG ThreadSum;				    // ����ͻ����߳���Ŀ
	HANDLE ProducerEvent;		    // �����߻������¼�
	HANDLE ComplePort;			    // ��ɶ˿ھ��
	SOCKET ListenPort;			    // �����˿ھ��
	sockaddr_in Addr;			    // �洢������ַЭ�������Χ
	HANDLE * Threads;			    // IoCp�����µ������߳�

	// �������̣߳��׽��ּ������ɲ���
	void IoCp::Producer(void);

	// IoData �����߳�
	static inline DWORD WINAPI TaskProducer(LPVOID lpParam)
	{
		((IoCp *)lpParam)->Producer();
		return 0;
	}

	// �������̣߳��׽����첽��Ӧ����
	void IoCp::Worker(void);

	// IoData �����߳�
	static inline DWORD WINAPI TaskWorker(LPVOID lpParam)
	{
		((IoCp *)lpParam)->Worker();
		return 0;
	}

};
#endif // IoCp_H
