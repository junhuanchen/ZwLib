#include "ZwCache.h"

#include <stdlib.h>

bool ZwCacheNew(ZwCache *Self, uint32_t MapSize, uint32_t QueueSize)
{
    Self->Map = MapNew(MapSize, malloc, free, (map_value_new)ZwDataNew, (map_value_del)ZwDataDel);
    if (NULL != Self->Map)
    {
        Self->Queue = QueueArrayNew(QueueSize, malloc, free);
        if (NULL != Self->Queue)
        {
            return true;
        }
        MapDel(Self->Map);
    }
    return false;
}

void ZwCacheDel(ZwCache *Self)
{
    if (NULL != Self->Map) MapDel(Self->Map);
    if (NULL != Self->Queue) QueueArrayDel(Self->Queue);
}

enum ZwCacheFlag ZwCacheUpdateEvent( ZwCache *Self, uint8_t SourceLen, uint8_t *Source, uint8_t DataLen, uint8_t *Data )
{
	ZwMaxSource temp;
	ZwMaxSourceCpy( &temp, Source, SourceLen );
    MapPair *pair = MapGetPair(Self->Map, &temp.Self, &DataLen);
    if (NULL != pair)
    {
        ZwData *data = (ZwData *)pair->Value;
        // �˴�����Ӹ���ZwData������С�Ĳ��������Դ��ⲿ���Ƴ���Ԫ���������Ԫ�ء�����Ϊ�������һ�㲻���ڳ�������ʱ���֣�
				if (data->DataLen == DataLen)
        {
            // �����ݱ仯�˺�򾭹���������һ����������ط�
            if ((0 != ZwDataCmp(data, Data) || (0 == ++data->Count)) && false == data->Used)
            {
                ZwDataCpy(data, Data), data->Used = true;
                return QueueArrayPush(Self->Queue, pair) ? ZwCacheOK : ZwCacheMem;
            }
            return ZwCacheNo;
        }
				MapRemovePair(Self->Map, &temp.Self);
        return ZwCacheErr;
    }
    return ZwCacheMem;
}

bool ZwCacheCheckEvent(ZwCache *Self)
{
    return (Self->Queue && QueueArrayExist(Self->Queue));
}

MapPair *ZwCacheGetEvent(ZwCache *Self)
{
    return (MapPair *)QueueArrayFront(Self->Queue);
}

void ZwCacheRemoveEvent(ZwCache *Self)
{
    if (true == ZwCacheCheckEvent(Self))
    {
        // �Ƴ���ͷ�Ԫ��
        ((ZwData *)ZwCacheGetEvent(Self)->Value)->Used = false;
        QueueArrayPop(Self->Queue);
    }
}

uint32_t ZwCacheSizeEvent(ZwCache *Self)
{
    return QueueArraySize(Self->Queue);
}

#ifdef UNIT_TEST

#define _CRTDBG_MAP_ALLOC /**< VS �ṩ�� malloc �ڴ�й©����  */

#include <crtdbg.h>
#include <assert.h>

void UnitTestZwCache()
{
    ZwCache cache = { NULL, NULL };
    MapPair *pair = NULL;
    ZwData *data = NULL;

    assert(true == ZwCacheNew(&cache, 2u, 16u));

    assert(0 == ZwCacheSizeEvent(&cache));

    assert(false == ZwCacheCheckEvent(&cache));

	ZwCacheUpdateEvent(&cache, 5, (uint8_t *)"test", 5, (uint8_t *)"12345");

    assert(1 == ZwCacheSizeEvent(&cache));

    assert(true == ZwCacheCheckEvent(&cache));
    assert(1 == ZwCacheSizeEvent(&cache));

    pair = ZwCacheGetEvent(&cache);

    assert(NULL != pair);
    data = (ZwData *)pair->Value;

    ZwCacheRemoveEvent(&cache);

    assert(0 == ZwCacheSizeEvent(&cache));

    for (uint16_t i = 0; i != UINT16_MAX; i++)
    {
		ZwCacheUpdateEvent(&cache, 5, (uint8_t *)"test", 5, (uint8_t *)"12345" );
        assert(0 == ZwCacheSizeEvent(&cache) && false == ZwCacheCheckEvent(&cache));
    }

	ZwCacheUpdateEvent(&cache, 5, (uint8_t *)"nihao", 5, (uint8_t *)"12345" );

    assert(true == ZwCacheCheckEvent(&cache));

    assert(1 == ZwCacheSizeEvent(&cache));

    pair = ZwCacheGetEvent(&cache);

    assert(NULL != pair);

    data = (ZwData *)pair->Value;

    ZwCacheRemoveEvent(&cache);

    assert(0 == ZwCacheSizeEvent(&cache));

    ZwCacheDel(&cache);

    _CrtDumpMemoryLeaks();

}

#endif
