
/**
 * @file
 * @brief 控制执行事件模块
 * @author JunHuanChen
 * @date 2018-01-28 21:41:38
 * @version 1.3
 * @remark 计划任务状态机，与多线程模型不同，它需要主动赋予定时和周期执行状态。
 */
#ifndef ZW_CTRL_H
#define ZW_CTRL_H

#include "..\..\Struct\Map\Map.h"

typedef void( *ZwMethod )( void *Param );

typedef struct zw_task
{
	ZwMethod Function;
	void *Param;
	uint32_t Cycle, OnTime;
}ZwTask;

enum
{
	ZwTaskMax = 16, 
};

typedef struct zw_ctrl
{
	Map * TaskPool;
}ZwCtrl;

bool ZwCtrlNew( ZwCtrl *Self );

void ZwCtrlDel( ZwCtrl *Self );

bool ZwCtrlCreateTask( ZwCtrl *Self, MapKey *TaskName, ZwMethod Function, void *Param, uint32_t OnTime, uint32_t Cycle );

MapPair * ZwCtrlGetTask(ZwCtrl *Self, MapKey *TaskName);

bool ZwCtrlRemoveTask( ZwCtrl *Self, MapKey *TaskName );

void ZwCtrlRunning( ZwCtrl *Self, uint32_t Time );

#endif // ZW_CTRL_H
