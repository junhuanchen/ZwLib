
#ifndef ZW_CACHE_H
#define ZW_CACHE_H

#include "ZwData.h"

typedef void *( *zw_cache_new )( unsigned int ); /** < 内存申请函数声明  */

typedef void( *zw_cache_del )( void * ); /** < 内存释放函数声明  */

#include "../../Struct/Queue/QueueArray.h"

typedef struct zw_cache
{
	QueueArray * Queue;
	Map * Map;
}ZwCache;

enum ZwCacheFlag
{
	ZwCacheOK,  // 缓存 成功 并 触发
	ZwCacheNo,  // 缓存 成功 但 未触发
	ZwCacheMem, // 缓存 内存不足，和容器容量有关
	ZwCacheErr, // 缓存 数据不符合存储要求，和数据长度有关
};

bool ZwCacheNew( ZwCache *Self, uint32_t MapSize, uint32_t QueueSize );

void ZwCacheDel( ZwCache *Self );

enum ZwCacheFlag ZwCacheUpdateEvent( ZwCache *Self, uint8_t SourceLen, uint8_t *Source, uint8_t DataLen, uint8_t *Data );

bool ZwCacheCheckEvent( ZwCache *Self );

MapPair *ZwCacheGetEvent( ZwCache *Self );

void ZwCacheRemoveEvent( ZwCache *Self );

uint32_t ZwCacheSizeEvent( ZwCache *Self );

#endif
