// #pragma once

#ifndef IoCp_H
#define IoCp_H

#include "iodata.h"

#include "../../../Struct/Queue/QueueList.hpp"

// 日志输出
#include "../../FrameWork.hpp"

class IoCp
{
public:
	enum
	{
		PreBufClient = 4,   // 每次缓冲处理客户端的套接字
		FlagExit = -'E',    // IoCp 模型退出标记
		BusinessCmdLen = 1, // 预设的业务逻辑接收到的指令长度
	};

	// 创建IoCp模型
	BOOL StartUp(LONG ThreadSum, USHORT port);

	// 销毁IoCp模型
	void CleanUp();

protected:

	// 验证连接继承接口
	virtual BOOL CheckCnct(IoData * Data);

	// 传输处理继承接口
	virtual void Business(IoData * Data);

	LONG ClientNum, ClientSum;	    // 连接的客户端数目和套接字最大值
    TemplateQueueList< IoData > ClientQueue;// 客户端套接字队列
	LONG ThreadSum;				    // 处理客户端线程数目
	HANDLE ProducerEvent;		    // 生产者缓冲区事件
	HANDLE ComplePort;			    // 完成端口句柄
	SOCKET ListenPort;			    // 监听端口句柄
	sockaddr_in Addr;			    // 存储主机地址协议监听范围
	HANDLE * Threads;			    // IoCp服务下的所有线程

	// 生产者线程：套接字检查和生成操作
	void IoCp::Producer(void);

	// IoData 生产线程
	static inline DWORD WINAPI TaskProducer(LPVOID lpParam)
	{
		((IoCp *)lpParam)->Producer();
		return 0;
	}

	// 消费者线程：套接字异步响应操作
	void IoCp::Worker(void);

	// IoData 工作线程
	static inline DWORD WINAPI TaskWorker(LPVOID lpParam)
	{
		((IoCp *)lpParam)->Worker();
		return 0;
	}

};
#endif // IoCp_H
