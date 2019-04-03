#ifndef POOL_STATIC_H
#define POOL_STATIC_H

#include <stdint.h>
#include <string.h>

typedef void *BlockStatic;

typedef struct pool_static
{
	uint32_t Usage, Len;
	uint8_t *Area;
}PoolStatic;

void PoolStaticInit( PoolStatic *Pool, uint8_t *Area, uint32_t Len )
{
	if ( Pool != NULL && Area != NULL && Len > 0 )
	{
		// memset(Area, 0, Len);// ³õÊ¼»¯
		Pool->Area = Area;
		Pool->Len = Len, Pool->Usage = 0;
	}
}

void PoolStaticCopy( PoolStatic * Target, PoolStatic * Source )
{
	if ( Target->Len >= Source->Len )
	{
		memcpy( Target->Area, Source->Area, Source->Len );
		Target->Usage = Source->Usage;
	}
}

BlockStatic PoolStaticNew( PoolStatic *Pool, uint32_t Size )
{
	BlockStatic Type = NULL;
	if ( Size && Pool->Usage + Size <= Pool->Len )
	{
		Type = Pool->Area + Pool->Usage;
		Pool->Usage += Size;
	}
	return Type;
}

__inline void PoolStaticDel( BlockStatic Data )
{
	// Do Not Need;
}

#ifdef UNIT_TEST

#include <stdio.h>
static void PoolStaticDump(PoolStatic *Self)
{
	printf("ScPool Dump: ");
	for (uint32_t i = 0; i < Self->Len; i++)
	{
		printf("%X ", Self->Area[i]);
	}
	putchar('\n');
}

static void UnitTestPoolStatic()
{
	uint8_t MemArea[32];
	PoolStatic Pool;
	PoolScInit(&Pool, MemArea, sizeof(MemArea));
	int * i = (int *)PoolStaticNew(&Pool, 4);
	*i = 0x12345678;
	int * j = (int *)PoolStaticNew(&Pool, 4);
	*j = 0x87654321;
	int * k = (int *)PoolStaticNew(&Pool, 4);
	*k = 0x90909090;
	printf("%f\n", (float)(Pool.Len - Pool.Usage) / Pool.Len * 100);
	PoolStaticDump(&Pool);
}
#endif

#endif // POOL_STATIC_H
