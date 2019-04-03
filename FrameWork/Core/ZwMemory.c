
#include "ZwMemory.h"

static uint8_t StaticArea[10 * 1024];
static PoolStatic Queue;
static PoolStatic Map;
static PoolStatic ZwData;

void ZwMemoryReInit(uint32_t QueueAreaLen, uint32_t MapAreaLen, uint32_t ZwDataAreaLen)
{
    uint8_t *Area = StaticArea;
    // Queue Init
    PoolStaticInit(&Queue, Area, QueueAreaLen), Area += QueueAreaLen;
    // Map Init
    PoolStaticInit(&Map, Area, MapAreaLen), Area += MapAreaLen;
    // ZwData Init
    PoolStaticInit(&ZwData, Area, ZwDataAreaLen), Area += ZwDataAreaLen;
}

// ªÒ»° £”‡¬ 
float ZwMemoryGetUsageRate(enum ZwMemoryFlag Flag)
{
    float max, usage;
    switch (Flag)
    {
        case ZwMemoryQueue:
            max = Queue.Len, usage = Queue.Usage;
            break;
        case ZwMemoryMap:
            max = Map.Len, usage = Map.Usage;
            break;
        case ZwMemoryZwData:
            max = ZwData.Len, usage = ZwData.Usage;
            break;
        default:
            max = usage = 0;
            break;
    }
    return (float)(usage / max * 100.0);
}

void ZwMemoryFree(void * Data)
{
    PoolStaticDel(Data);
}

void * ZwMemoryQueueAlloc(uint32_t Size)
{
    return PoolStaticNew(&Queue, Size);
}

void * ZwMemoryMapAlloc(uint32_t Size)
{
    return PoolStaticNew(&Map, Size);
}

void * ZwMemoryZwDataAlloc(uint32_t Size)
{
    return PoolStaticNew(&ZwData, Size);
}

#ifdef UNIT_TEST

#include <stdio.h>
#include <assert.h>

void UnitTestZwMemory( )
{
    assert(NULL == ZwMemoryQueueAlloc(1));
    assert(NULL == ZwMemoryMapAlloc(1));
    assert(NULL == ZwMemoryZwDataAlloc(1));

    ZwMemoryReInit(1024, 2048, 4096);

    assert(NULL != ZwMemoryQueueAlloc(64));
    assert(NULL != ZwMemoryMapAlloc(64));
    assert(NULL != ZwMemoryZwDataAlloc(64));

    printf("ZwMemoryGetUsageRate : %f\n", ZwMemoryGetUsageRate(ZwMemoryQueue));
    printf("ZwMemoryGetUsageRate : %f\n", ZwMemoryGetUsageRate(ZwMemoryMap));
    printf("ZwMemoryGetUsageRate : %f\n", ZwMemoryGetUsageRate(ZwMemoryZwData));

    ZwMemoryReInit(1024, 2048, 4096);

    assert(NULL != ZwMemoryQueueAlloc(64));
    assert(NULL != ZwMemoryMapAlloc(64));
    assert(NULL != ZwMemoryZwDataAlloc(64));

    printf("ZwMemoryGetUsageRate : %f\n", ZwMemoryGetUsageRate(ZwMemoryQueue));
    printf("ZwMemoryGetUsageRate : %f\n", ZwMemoryGetUsageRate(ZwMemoryMap));
    printf("ZwMemoryGetUsageRate : %f\n", ZwMemoryGetUsageRate(ZwMemoryZwData));
}

#endif
