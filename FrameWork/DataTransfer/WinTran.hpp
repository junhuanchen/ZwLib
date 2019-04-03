#ifndef _WIN_TRAN_H_
#define _WIN_TRAN_H_

#include <Windows.h>
#include "../FrameWork.hpp"
#include <stdint.h>
EXTERN_C
{
#include "../Core\ZwTransit.h"
}

class WinTranTask
{
    HANDLE Thread;
public:
    // 继承接口
	virtual void BackgroundProcess()
	{
		// 后台线程执行任务
	}
	
	static DWORD WINAPI WinTranTask::RetryProc(LPVOID lpParam)
	{
		WinTranTask *Self = (WinTranTask *) lpParam;
		while ( Self->Thread ) Self->BackgroundProcess( );
		return 0;
	}

	WinTranTask::WinTranTask( )
	{
		Thread = NULL;
	}

	bool WinTranTask::Start( )
	{
		return (NULL != Thread) ? false : NULL != (Thread = CreateThread(NULL, 0, RetryProc, (LPVOID)this, 0, NULL));
	}

	void WinTranTask::Stop( )
	{
		HANDLE Tmp = Thread;
		Thread = NULL;
		WaitForSingleObject(Tmp, INFINITE);
		CloseHandle(Tmp);
	}
};

struct WinTranPack
{
    UCHAR buf[ZwTranMax];
	WinTranPack()
	{
		memset(buf, 0, sizeof(buf));
	}
	WinTranPack(char * str)
	{
		strcpy((char *) buf, str);
	}
	WinTranPack(uint8_t * buffer, uint8_t len)
	{
		memcpy(buf, buffer, len);
	}
};

#endif