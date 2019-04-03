#ifndef ZW_MEMORY_H
#define ZW_MEMORY_H

#include "../../Struct/Pool/PoolStatic.h"

// 获取内存占用率标记
enum ZwMemoryFlag
{
    ZwMemoryQueue,
    ZwMemoryMap,
    ZwMemoryZwData,
};

//内存管理重置
void ZwMemoryReInit(uint32_t QueueAreaLen, uint32_t MapAreaLen, uint32_t ZwDataAreaLen);
// 获取剩余率
float ZwMemoryGetUsageRate(enum ZwMemoryFlag Flag);
// 队列内存申请接口
void * ZwMemoryQueueAlloc(uint32_t Size);
// 容器内存申请接口
void * ZwMemoryMapAlloc(uint32_t Size);
// 数据内存申请接口
void * ZwMemoryZwDataAlloc(uint32_t Size);
// 释放内存 // 此处为统一释放接口
void ZwMemoryFree(void * Data);

#endif // ZW_MEMORY_H
