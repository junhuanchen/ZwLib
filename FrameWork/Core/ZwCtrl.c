#include "ZwCtrl.h"

#ifdef LOG_OUT 
#include <stdio.h>
#else 
#define printf(Format, ...)
#endif 

void ZwTaskSet(ZwTask *Self, ZwMethod Function, void *Param, uint32_t OnTime, uint32_t Cycle)
{
	Self->Function = Function, Self->Param = Param, Self->OnTime = OnTime, Self->Cycle = Cycle;
}

void ZwTaskInit(ZwTask *Self)
{
	Self->Function = NULL, Self->Param = NULL, Self->OnTime = UINT32_MAX, Self->Cycle = 0;
}

void ZwTaskExecute(ZwTask *Self, uint32_t Time)
{
	if (Time >= Self->OnTime) // 时间已到
	{
		Self->Function(Self->Param); // 执行任务
		if (0 != Self->Cycle) // 添加周期数
		{
			Self->OnTime += Self->Cycle;
		}
	}
}

#include <stdlib.h>

void *ZwTaskNew(void * Param)
{
	ZwTask *task = (ZwTask *)malloc(sizeof(ZwTask));
	if (NULL != task)
	{
		ZwTaskInit(task);
	}
	return task;
}

void ZwTaskDel(ZwTask *Self)
{
	free(Self);
}

bool ZwCtrlNew(ZwCtrl *Self)
{
	Self->TaskPool = MapNew(1, malloc, free, ZwTaskNew, ZwTaskDel);
	return (NULL != Self->TaskPool);
}

void ZwCtrlDel(ZwCtrl *Self)
{
	if (NULL != Self->TaskPool)
	{
		MapDel(Self->TaskPool);
	}
}

bool ZwCtrlCreateTask(ZwCtrl *Self, MapKey *TaskName, ZwMethod Function, void *Param, uint32_t OnTime, uint32_t Cycle)
{
	if (NULL != Self->TaskPool)
	{
		MapPair *pair = MapGetPair(Self->TaskPool, TaskName, NULL);
		if (NULL != pair)
		{
			ZwTask * task = (ZwTask *)pair->Value;
			ZwTaskSet(task, Function, Param, OnTime, Cycle);
			printf("ZwCtrl Create Task : %.*s Time : %u Cycle : %u\n", pair->Key->Len, pair->Key->Data, task->OnTime, task->Cycle);
			return true;
		}
	}
	return false;
}

MapPair * ZwCtrlGetTask(ZwCtrl *Self, MapKey *TaskName)
{
	if (NULL != Self->TaskPool)
	{
		return MapFindPair(Self->TaskPool, TaskName);
	}
	return NULL;
}

bool ZwCtrlRemoveTask(ZwCtrl *Self, MapKey *TaskName)
{
	printf("ZwCtrl Remove Task : %.*s\n", TaskName->Len, TaskName->Data);

	return MapRemovePair(Self->TaskPool, TaskName);
}

void ZwCtrlRunning(ZwCtrl *Self, uint32_t Time)
{
	MapPair *pair = MapIteratorPair(Self->TaskPool);

	printf("ZwCtrl Execute Task : %.*s\n", pair->Key->Len, pair->Key->Data);

	ZwTaskExecute((ZwTask *)pair->Value, Time);
}

#ifdef UNIT_TEST

#undef printf

#define _CRTDBG_MAP_ALLOC /**< VS 提供的 malloc 内存泄漏检测宏 */
#include <crtdbg.h>

#include <time.h>

#include <Windows.h>

#include <stdio.h>

#include "..\..\FrameWork\Core\ZwCache.h"

static void TaskUploadRepond(ZwCache *Self)
{
	if (true == ZwCacheCheckEvent(Self))
	{
		ZwCacheRemoveEvent(Self);
		printf("\tTaskUploadRepond\n");
	}
}

static void TaskUploadRequest(ZwCache *Self)
{
	if (ZwCacheCheckEvent(Self))
	{
		MapPair *pair = ZwCacheGetEvent(Self);
		printf("\tTaskUploadRequest : %.*s - %.*s\n",
			pair->Key->Len, pair->Key->Data, ((ZwData *)pair->Value)->DataLen, ((ZwData *)pair->Value)->Data);
		TaskUploadRepond(Self);
	}
}

struct CollectConfig
{
	ZwCache *Cache;
	ZwMaxSource Source;
};

static void TaskCollectRandData(struct CollectConfig *Self)
{
	uint8_t data[7] = { 0 };
	sprintf((char *)data, "%0.*hd", 6, rand());
	printf("\tTaskCollectRandData ： %.*s - %.*s\n", Self->Source.Self.Len, Self->Source.Self.Data, 6, data);
	ZwCacheUpdateEvent(Self->Cache, Self->Source.Self.Len, Self->Source.Self.Data, 6, data);
}

#include "..\..\Struct\Pool\PoolStatic.h"

uint8_t ZwSourceArea[sizeof(struct CollectConfig) * 3];

static void UnitTestZwCtrl()
{
	srand(time(NULL));

	PoolStatic ZwSourcePool;

	PoolStaticInit(&ZwSourcePool, ZwSourceArea, sizeof(ZwSourceArea));

	ZwCache cache;

	if (true == ZwCacheNew(&cache, 10, 10))
	{
		ZwCtrl ctrl;

		if (true == ZwCtrlNew(&ctrl))
		{
			char task_name[30];
			uint8_t area[30 + 1];

			for (size_t i = 0; i < 3; i++)
			{
				struct CollectConfig * config = (struct CollectConfig *) PoolStaticNew(&ZwSourcePool, sizeof(struct CollectConfig));

				if (NULL != config)
				{
					char tmp_str[9];
					sprintf(tmp_str, "Test%u", i);
					ZwMaxSourceCpy(&config->Source, tmp_str, strlen(tmp_str));
					config->Cache = &cache;

					sprintf(task_name, "Task Collect %s", tmp_str);
					MapKey *key = MapKeySet(area, sizeof(area), strlen(task_name));
					MapKeyCopy(key, task_name);

					ZwCtrlCreateTask(&ctrl, key, (ZwMethod)TaskCollectRandData, config, rand() % 8 + 2, rand() % 8 + 2);
				}
			}

			sprintf(task_name, "Task Upload RandData");
			MapKey *key = MapKeySet(area, sizeof(area), strlen(task_name));
			MapKeyCopy(key, task_name);

			ZwCtrlCreateTask(&ctrl, key, (ZwMethod)TaskUploadRequest, &cache, 0, 1);

			uint32_t start_time = time(NULL);
			for (size_t i = 0; i < 40; i++)
			{
				ZwCtrlRunning(&ctrl, time(NULL) - start_time);
				Sleep(250);
				/*
				if ( 6 == i ) // 测试迭代器指针函数
				{
					ZwCtrlRemoveTask( &ctrl, key );
				}
				*/
			}

			ZwCtrlDel(&ctrl);
		}

		ZwCacheDel(&cache);
	}

	_CrtDumpMemoryLeaks();

	system("pause");
}

#endif
