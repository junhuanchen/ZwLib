
#ifndef ZW_CACHE_H
#define ZW_CACHE_H

#include "ZwData.h"

typedef void *( *zw_cache_new )( unsigned int ); /** < �ڴ����뺯������  */

typedef void( *zw_cache_del )( void * ); /** < �ڴ��ͷź�������  */

#include "../../Struct/Queue/QueueArray.h"

typedef struct zw_cache
{
	QueueArray * Queue;
	Map * Map;
}ZwCache;

enum ZwCacheFlag
{
	ZwCacheOK,  // ���� �ɹ� �� ����
	ZwCacheNo,  // ���� �ɹ� �� δ����
	ZwCacheMem, // ���� �ڴ治�㣬�����������й�
	ZwCacheErr, // ���� ���ݲ����ϴ洢Ҫ�󣬺����ݳ����й�
};

bool ZwCacheNew( ZwCache *Self, uint32_t MapSize, uint32_t QueueSize );

void ZwCacheDel( ZwCache *Self );

enum ZwCacheFlag ZwCacheUpdateEvent( ZwCache *Self, uint8_t SourceLen, uint8_t *Source, uint8_t DataLen, uint8_t *Data );

bool ZwCacheCheckEvent( ZwCache *Self );

MapPair *ZwCacheGetEvent( ZwCache *Self );

void ZwCacheRemoveEvent( ZwCache *Self );

uint32_t ZwCacheSizeEvent( ZwCache *Self );

#endif
