#include "ZwData.h"

// 内存获取接口
#include <stdlib.h>
#define New malloc
#define Del free

//#include "ZwMemory.h"
//#define New ZwMemoryQueueAlloc
//#define Del ZwMemoryFree

ZwData *ZwDataNew(uint8_t *DataLen)
{
    ZwData *data = (ZwData *)New(sizeof(ZwData) + *DataLen);
    if (NULL != data)
    {
        memset(data, 0, sizeof(ZwData) + *DataLen), data->DataLen = *DataLen, data->Used = false;
    }
    return data;
}

void ZwDataDel(ZwData *Self)
{
    Del(Self);
}
