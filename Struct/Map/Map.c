
#include "Map.h"

MapKey *MapKeySet( uint8_t Area[], uint8_t AreaLen, uint8_t Key[], uint8_t Len )
{
	if ( Area != NULL && AreaLen >= Len + sizeof( MapKey ) )
	{
		MapKey *key = ( MapKey * ) Area;
		key->Len = Len;
		memcpy(key->Data, Key, Len);
		return key;
	}
	return NULL;
}

MapKey *MapKeyNew( map_new New, uint8_t Len )
{
	MapKey *key = ( MapKey * ) ( New( sizeof( MapKey ) + Len ) );
	if ( NULL != key )
	{
		key->Len = Len;
		memset( key->Data, '\0', key->Len );
	}
	return key;
}

MapHash MapKeyHash( MapKey *Key )
{
	MapHash result = 0;
	for ( uint8_t i = 0; i != Key->Len; i++ )
	{
		result += Key->Data[i];
	}
	return result;
}

bool MapKeyEqual( MapKey *Left, MapKey *Right )
{
	return ( Left->Len != Right->Len ) ? false : 0 == memcmp( Left->Data, Right->Data, Left->Len );
}

MapPair *MapListFind( MapList *List, MapKey *Key )
{
	MapNode *end = &List->Head, *pos = end->Next;
	ListOneWayForeach( pos, end )
	{
		MapPair *target = StructOf( pos, MapPair, Node );
		if ( MapKeyEqual( target->Key, Key ) )
		{
			return target;
		}
	}
	return NULL;
}

#define MapPairNew(Self) ((MapPair *)Self->New(sizeof(MapPair)))

#define MapPairDel(Self, Pair) Self->Del(Pair)

#define MapGetList(Self, Pos) ((MapList *)((Self)->ListSet + Pos))

Map *MapNew( MapHash ListSum, map_new New, map_del Del, map_value_new NewValue, map_value_del DelValue )
{
	Map *map = ( Map* ) New( sizeof( Map ) + ListSum *sizeof( MapList ) );
	if ( NULL != map )
	{
		map->New = New, map->Del = Del;
		map->NewValue = NewValue, map->DelValue = DelValue;
		map->ListSum = ListSum;
		while ( ListSum-- )
		{
			ListOneWayInit( map->ListSet + ListSum );
		}

#ifdef StartupMapIteratorPair
		map->Iterator = 0;
		map->IteratorEnd = map->IteratorPos = NULL;
		map->IteratorResult = NULL;
#endif

	}
	return map;
}

void MapDel( Map *Self )
{
	while ( Self->ListSum-- )
	{
		MapNode *bak, *end = &( MapGetList( Self, Self->ListSum )->Head ), *pos = end->Next;
		ListOneWayForeachSafe( pos, end, bak )
		{
			MapPair *pair = StructOf( pos, MapPair, Node );
			Self->DelValue( pair->Value ), MapKeyDel( Self->Del, pair->Key ), Self->Del( pair );
		}
	}
	Self->Del( Self );
}

MapPair *MapFindPair( Map *Self, MapKey *Key )
{
	return MapListFind( MapGetList( Self, ( MapKeyHash( Key ) % Self->ListSum ) ), Key );
}

MapPair *MapGetPair( Map *Self, MapKey *Key, void *NewValueParam )
{
	MapList *list = MapGetList( Self, ( MapKeyHash( Key ) % Self->ListSum ) );
	MapPair *pair = MapListFind( list, Key );
	if ( NULL == pair )
	{
		if ( ( NULL != ( pair = MapPairNew( Self ) ) ) )
		{
			if ( NULL != ( pair->Key = MapKeyNew( Self->New, Key->Len ) ) )
			{
				MapKeyCopy( pair->Key, Key->Data );
				if ( NULL != ( pair->Value = Self->NewValue( NewValueParam ) ) )
				{
					ListOneWayTailAdd( list, &pair->Node );
					return pair;
				}
				MapKeyDel( Self->Del, Key );
			}
			MapPairDel( Self, pair );
		}
	}
	return pair;
}

bool MapRemovePair( Map *Self, MapKey *Key )
{
	MapList *list = MapGetList( Self, ( MapKeyHash( Key ) % Self->ListSum ) );
	MapNode *prev, *end = &list->Head, *pos = end->Next;
	ListOneWayForeachPrev( pos, end, prev )
	{
		MapPair *pair = StructOf( pos, MapPair, Node );
		if ( MapKeyEqual( pair->Key, Key ) )
		{
#ifdef StartupMapIteratorPair
			// 将其与先迭代指针比较后执行跌代函数
			if ( Self->IteratorPos == pos )
			{
				MapIteratorPair( Self );
			}
#endif			
			// 核对是否将删除链表末端元素（维护 Tail 指针）
			if ( list->Tail == pos )
			{
				list->Tail = prev;
			}
			NodeOneWayDel( prev ); // 从链表中移除
			Self->DelValue( pair->Value ), MapKeyDel( Self->Del, pair->Key ), Self->Del( pair );
			// 注：在并行环境中对删除后的数据须校验是否成功，例如：return prev->Next != pos;
			return true;
		}
	}
	return false;
}

#ifdef StartupMapIteratorPair
MapPair *MapIteratorPair( Map *Self )
{
	while ( Self->IteratorPos == Self->IteratorEnd )
	{
		Self->IteratorEnd = &Self->ListSet[Self->Iterator].Head, Self->IteratorPos = Self->IteratorEnd->Next;
		Self->Iterator++, Self->Iterator %= Self->ListSum;
	}
	Self->IteratorResult = StructOf( Self->IteratorPos, MapPair, Node );
	Self->IteratorPos = Self->IteratorPos->Next;
	return Self->IteratorResult;
}
#endif

#ifdef UNIT_TEST

#define _CRTDBG_MAP_ALLOC /**< VS 提供的 malloc 内存泄漏检测宏  */

#include <crtdbg.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

struct Test
{
	char a;
	short b;
	int c;
};

void *TestNew()
{
	struct Test *Test = (struct Test *)malloc(sizeof(struct Test));
	Test->a = rand(), Test->b = rand(), Test->c = rand();
	printf("tmp : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	return Test;
}

void TestDel(struct Test *Test)
{
	printf("tmp : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	free(Test);
}

/**
 * @brief map 单元测试
 * @remark 单元测试中采用assert（断言）处理测试结果。
 */
void UnitTestMap(void)
{
	_CrtDumpMemoryLeaks();

#define MapKeyInit(Key) MapKey *Key = MapKeyNew(malloc, sizeof(#Key) - 1); memcpy(Key->Data, #Key, Key->Len);

	MapKeyInit(DM12345);
	MapKeyInit(DM12344);
	MapKeyInit(D03000);
	MapKeyInit(D03001);

	Map *map = MapNew(16, malloc, free, TestNew, (map_value_del)TestDel);
	assert(map != NULL);

	MapPair *pair;
	struct Test *Test;

	assert(NULL == MapFindPair(map, DM12345));

	pair = MapGetPair(map, DM12345);
	assert(NULL != pair && true == MapKeyEqual(DM12345, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);

	assert(pair == MapFindPair(map, DM12345));
	assert(true == MapRemovePair(map, DM12345));
	assert(false == MapRemovePair(map, DM12345));

	pair = MapGetPair(map, DM12345);
	assert(NULL != pair && true == MapKeyEqual(DM12345, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	pair = MapGetPair(map, DM12344);
	assert(NULL != pair && true == MapKeyEqual(DM12344, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	pair = MapGetPair(map, D03000);
	assert(NULL != pair && true == MapKeyEqual(D03000, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	pair = MapGetPair(map, D03001);
	assert(NULL != pair && true == MapKeyEqual(D03001, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);

#ifdef StartupMapIteratorPair // 内部哈希链表序
	pair = MapIteratorPair(map);
	assert(NULL != pair && true == MapKeyEqual(DM12345, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	pair = MapIteratorPair(map);
	assert(NULL != pair && true == MapKeyEqual(D03000, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	pair = MapIteratorPair(map);
	assert(NULL != pair && true == MapKeyEqual(D03001, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
	pair = MapIteratorPair(map);
	assert(NULL != pair && true == MapKeyEqual(DM12344, pair->Key) && NULL != pair->Value);
	Test = (struct Test *)pair->Value, printf("Test : a %c, b %06hd, c %06d\n", Test->a, Test->b, Test->c);
#endif

	assert(true == MapRemovePair(map, DM12345));
	assert(false == MapRemovePair(map, DM12345));

	assert(true == MapRemovePair(map, DM12344));
	assert(false == MapRemovePair(map, DM12344));

	assert(true == MapRemovePair(map, D03000));
	assert(false == MapRemovePair(map, D03000));

	assert(true == MapRemovePair(map, D03001));
	assert(false == MapRemovePair(map, D03001));

	MapKeyDel(free, DM12345);
	MapKeyDel(free, DM12344);
	MapKeyDel(free, D03000);
	MapKeyDel(free, D03001);

	MapDel(map);
	//// 检测内存是否泄漏，例如注释掉上一行的map_del

	_CrtDumpMemoryLeaks();
}
#endif 
