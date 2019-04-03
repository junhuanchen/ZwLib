
// 1.0 ԭʼ�汾��Դ buddy2.h
// 1.1 �Ľ��汾�������˾�̬������ֵ��ӳ�䣬ԭ���洢��ָ����ӻ�ȡ�������ڴ��ռ�á�
// 1.2 �ع��汾���ӿ�ͳһ���淶��

#include "Buddy.h"

// ======================����ͨ�ú���ʵ��======================

static uint32_t MulByTwo( uint32_t Num )
{
	return Num * 2; // << 1
}

static uint32_t DivByTwo( uint32_t Num )
{
	return Num / 2; // >> 1
}

// ����������������� �����������
enum
{
	TreeRoot = 0,
};

// ����������������� Ҷ�Ӹ��������
static uint32_t LeafParent( uint32_t index )
{
	return DivByTwo( index + 1 ) - 1;
}

// ����������������� Ҷ�����ӽ������
static uint32_t LeafLeft( uint32_t index )
{
	return MulByTwo( index ) + 1;
}

// ����������������� Ҷ�����ӽ������
static uint32_t LeafRight( uint32_t index )
{
	return MulByTwo( index ) + 2;
}

// �ж��Ƿ�Ϊ��������ֵ
static uint8_t IsPowOfTwo( uint32_t Num )
{
	return !( Num & ( Num - 1 ) );
}

// ��ֵ����Ϊ������ֵ��52 => 64, 67 => 128��
static uint32_t FixToPowOfTwo( uint32_t Num )
{
	static const int TreeMaxLayer = ( 8 * sizeof( uint32_t ) );
	for ( int i = 1; i != TreeMaxLayer; i = MulByTwo( i ) )
	{
		Num |= Num >> i;
	}
	return Num + 1;
}

// ���ؽϴ�ֵ���з������ܹ����ֱ�ռ�õĿ��ǣ�-1��������С�ʿ��Բ���ѡ�񣩣�
static int8_t TreeMax( int8_t a, int8_t b )
{
	return a > b ? a : b;
}

//// ����Ҫ2^64����ڴ�����ʱ
//static const char debruijn_64[ ] = { 
//    0, 1, 48, 2, 57, 49, 28, 3, 61, 58, 50, 42, 38, 29, 17, 4, 62, 55, 59, 36, 53, 51,
//    43, 22, 45, 39, 33, 30, 24, 18, 12, 5, 63, 47, 56, 27, 60, 41, 37, 16, 54, 35, 52, 
//    21, 44, 32, 23, 11, 46, 26, 40, 15, 34, 20, 31, 10, 25, 14, 19, 9, 13, 8, 7, 6 
//};
//#define LogTwo64(v) debruijn_64[(uint64_t)((v) * 0x3f79d71b4cb0a89UL) >> 58]

// deBruijn����
static const int8_t debruijn[] = {
	0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9,
};

// ����log2 �����Ҷ����ƴ�����Ĵ���1Ϊֹ��0�ĸ��������磺0100 => 3��101 = > 0�������ڸ���N = 2^Y��N����Yֵ��
static int8_t LogTwo32( uint32_t N )
{
	return debruijn[( ( uint32_t ) ( ( ( N ) & -( N ) ) * 0x077CB531U ) ) >> 27];
}

// �Ľ��洢ԭʼ������ֵ����ָ���������һ����ȡ����������������
// ���Խ��Ϳռ临�Ӷ������ˣ�64λ������������ʱ�临�ӶȽ�����һ������
// �����������������ֵӳ����Ľ��洢���ݽṹ��
static const uint32_t PowTwo[] = {
	0x00000001, 0x00000002, 0x00000004, 0x00000008,
	0x00000010, 0x00000020, 0x00000040, 0x00000080,
	0x00000100, 0x00000200, 0x00000400, 0x00000800,
	0x00001000, 0x00002000, 0x00004000, 0x00008000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
}; // 2^N{N+}

static uint32_t PowTwo32( int8_t Y )
{
	return 0 <= Y && Y < ( sizeof( PowTwo ) / sizeof( *PowTwo ) ) ? PowTwo[Y] : 0;
}

// ======================����㷨����ʵ��======================

Buddy* BuddyNew( uint32_t Size, buddy_new New, buddy_del Del )
{
	if ( Size > 0 && IsPowOfTwo( Size ) )
	{
		// ���Ϊ n ����ȫ������������� s ���� s = n^2-1
		uint32_t tree_size = MulByTwo( Size ) - 1;
		Buddy* buddy = ( Buddy* ) New( sizeof( Buddy ) + tree_size * sizeof( int8_t ) );
		if ( NULL != buddy )
		{
			buddy->Size = Size;
			int8_t tree_pow = LogTwo32( Size ) + 1;
			for ( uint32_t i = 0; i != tree_size; i++ )
			{
				if ( IsPowOfTwo( i + 1 ) ) // ������йأ�ȡ����tree_pow��ʼֵ
				{
					tree_pow--;
				}
				buddy->MemPow[i] = tree_pow;
			}
			buddy->New = New, buddy->Del = Del;
			return buddy;
		}
	}
	return NULL;
}

void BuddyDel( Buddy* Self )
{
	Self->Del( Self );
}

uint32_t BuddyAlloc( Buddy* Self, uint32_t Size )
{
	// �ж����ݲ��Ϸ����ڴ��Ѳ���
	if ( Size == 0 || Self == NULL || PowTwo32( Self->MemPow[TreeRoot] ) < Size ) return BuddyMemOut;

	// �ڴ�����ֵ�����ض�������ֵ
	if ( 0 == IsPowOfTwo( Size ) ) Size = FixToPowOfTwo( Size );

	// ǰ��������Ҷ����������н��Ȩֵ�������ڴ��С��ȵ�����
	uint32_t index = TreeRoot;
	for ( uint8_t mem_pow = LogTwo32( Self->Size ); Size != PowTwo32( mem_pow ); mem_pow-- )
	{
		uint32_t mem_left = LeafLeft( index );
		index = ( PowTwo32( Self->MemPow[mem_left] ) >= Size ) ? mem_left : LeafRight( index );
	}

	Self->MemPow[index] = BuddyMemOut; // ��Ǹ��������ڴ汻ʹ��

	// ����ת�������ʵ�ڴ�����
	uint32_t Offset = ( index + 1 ) * Size - Self->Size;

	// ��������������ʣ���ڴ��Ȩֵ(Ȩֵ�۰�)
	while ( index )
	{
		index = LeafParent( index ); // ��ȡ���������
		Self->MemPow[index] = TreeMax( Self->MemPow[LeafLeft( index )], Self->MemPow[LeafRight( index )] );
	}

	return Offset;
}

void BuddyFree( Buddy* Self, uint32_t Offset )
{
	// �������Ϸ���
	if ( Offset >= Self->Size ) return;

	// ����ڴ��Ƿ��б������ȥ
	if ( Self->Size == PowTwo32( Self->MemPow[TreeRoot] ) ) return;

	// �����ڴ�����ת��Ϊ������������
	// ���ڲ�֪���������������ڴ���С������ L = 1 �򽫴���С�ӽ�����ϻ��ݲ���ȨֵΪBuddyMemOut�ĸ��ڵ㣬���ɵõ�ƫ������Ӧ�Ŀ��С��
	// �� MemPow = 0 => mem_size = 1 �� (Offset + Self->Size) / mem_size = (Offset + Self->Size)�������ͬ�� BuddySize
	uint32_t mem_pow = 0, index = Offset + Self->Size - 1;

	// ������С�ӽ��������������ұ���ǵĽ�����ڵ��ڴ���С��������Ӧ�Ĳ�����
	while ( BuddyMemOut != Self->MemPow[index] )
	{
		// �ѵ������������Բ��Ҳ�����ʹ�õ��ڴ��޸ĺ��BuddyMemOut,���˳�����
		if ( index == TreeRoot ) return;
		mem_pow++, index = LeafParent( index );
	}

	// �ָ��ö���������ԭ����Ȩֵ
	Self->MemPow[index] = mem_pow;

	// ������������������������ԭȨֵ
	for ( uint32_t mem_left, mem_right; index; )
	{
		mem_pow++, index = LeafParent( index ); // ��ȡ���������
		mem_left = Self->MemPow[LeafLeft( index )];
		mem_right = Self->MemPow[LeafRight( index )];

		// �������ڴ��С������˲��ڴ���С���ɺϲ���Ϊ��ǰ�����Ȩֵ
		if ( PowTwo32( mem_left ) + PowTwo32( mem_right ) == PowTwo32( mem_pow ) )
		{
			Self->MemPow[index] = mem_pow;
		}
		else
		{
			Self->MemPow[index] = TreeMax( mem_left, mem_right );
		}
	}
}

uint32_t BuddyUsable( Buddy* Self )
{
	return Self->Size - PowTwo32( Self->MemPow[TreeRoot] );
}

uint32_t BuddySize( Buddy* Self, uint32_t Offset )
{
	// �������Ϸ���
	if ( Offset >= Self->Size ) return -1;

	uint32_t mem_size = 1, index = Offset + Self->Size - 1;
	for ( ; BuddyMemOut != Self->MemPow[index]; index = LeafParent( index ) )
	{
		if ( index == TreeRoot ) return 0;
		mem_size = MulByTwo( mem_size );
	}

	return mem_size;
}

#ifdef UNIT_TEST

#include "../../Struct/Pool/PoolStatic.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void UnitTestSwapMemory()
{
#define BlockSize 4

	// ������ ��Ƭ ���ֽ� �ڴ�Ƭ
	static uint8_t GlobalBlock[4 * BlockSize];

	for ( int i = 0; i < sizeof( GlobalBlock ); i++ ) GlobalBlock[i] = i;

	Buddy *buddy = BuddyNew( ( sizeof( GlobalBlock ) / BlockSize ), malloc, free );

	if ( NULL != buddy )
	{
		int * value, pos, len;
		// �������ߵ��ڴ�Ĳ���

		PoolStatic MemMap;
		PoolStatic MemQueue;

		// ҪһƬ
		len = 1;
		pos = BuddyAlloc( buddy, len );
		assert( 0 == pos );
		PoolStaticInit( &MemMap, GlobalBlock + pos, BlockSize * len );

		for ( int i = 0; i < BlockSize * len + 1; i++ )
		{
			value = PoolStaticNew( &MemMap, BlockSize * 1 );
			if ( value ) printf( "i:%d value %d\n", i, *value );
		}

		// Ҫ��Ƭ
		len = 2;
		pos = BuddyAlloc( buddy, len );
		assert( 2 == pos );
		PoolStaticInit( &MemQueue, GlobalBlock + pos, BlockSize * len );

		for ( int i = 0; i < BlockSize * len + 1; i++ )
		{
			value = PoolStaticNew( &MemQueue, BlockSize * 1 );
			if ( value ) printf( "i:%d value %d\n", i, *value );
		}

		// �ٴγ���Ҫ��Ƭ��ʧ��
		pos = BuddyAlloc( buddy, 2 );
		assert( BuddyMemOut == pos );
		
		// �鿴ʣ����һƬ
		assert( 1 == BuddyUsable( buddy ) );

		// �ͷ��Լ�ӵ�е�һƬ���п����õ���Ƭ
		// ���߽������˵��ڴ������������Ϳ�����ȡ�������ڴ档

		// ���罫�Աߣ������������ͳ�Ƽ��ɵ�֪���Ķ����ڴ�������һƬ��
		// ���Ϊ������ʵ�ֶ�̬�ڴ���գ���ʱ��Ӧ���ȸ�����ת���ڴ棬������Ҫһ������ָ������ָ��������ָ���ת��
		// ������Ŀǰ��Ȼ������ʹ�ã���Ҫ������ڸ���������Ҫ�����ڶ������й�ָ��֮�ϲ��ܱ����ӵ�е��ڴ棬������C#�����Լܹ���
		// ��Ŀǰ���Ե�������ӵ�е��ڴ棬����Ҫ������ģ��Ҳ������ɡ�
		len = 1;
		pos = BuddyAlloc( buddy, len );
		assert( 1 == pos );
		PoolStatic tmp;
		PoolStaticInit( &tmp, GlobalBlock + pos, BlockSize * len );

		BuddyFree( buddy, 2 );

		len = 2;
		pos = BuddyAlloc( buddy, len );
		assert( 2 == pos );
		PoolStaticInit( &MemMap, GlobalBlock + pos, BlockSize * len );
		// �Ӷ����Ǳ�ת�Ƹ�ӳ������

		for ( int i = 0; i < BlockSize * len + 1; i++ )
		{
			value = PoolStaticNew( &MemQueue, BlockSize * 1 );
			if ( value ) printf( "i:%d value %d\n", i, *value );
		}



		BuddyDel( buddy );
	}

}

int main()
{
	UnitTestSwapMemory();
	return 0;
}

//#include <string.h>
//
//void BuddyDump(Buddy* Self, char * canvas, size_t canvas_len)
//{
//    if (Self == BuddyMemOut)
//    {
//        strncpy(canvas, "BuddyDump: (Buddy*)Self == Error",
//            sizeof("BuddyDump: (Buddy*)Self == Error"));
//        return;
//    }
//
//    if (Self->Size > canvas_len)
//    {
//        strncpy(canvas, "BuddyDump: (Buddy*)Self is too big to dump",
//            sizeof("BuddyDump: (Buddy*)Self is too big to dump"));
//        return;
//    }
//
//    memset(canvas, '_', canvas_len);
//    uint32_t MemPow = LogTwo32(Self->Size) + 1, tree_size = MulByTwo(Self->Size) - 1;
//    for (uint32_t i = 0; i < tree_size; i++)
//    {
//        if (IsPowOfTwo(i + 1))
//        {
//            MemPow--;
//        }
//        if (Self->MemPow[i] == BuddyMemOut)
//        {
//            if (i >= Self->Size - 1)
//            {
//                canvas[i - Self->Size + 1] = '*';
//            }
//            else if (BuddyMemOut != Self->MemPow[LeafLeft(i)] && BuddyMemOut != Self->MemPow[LeafRight(i)])
//            {
//                uint32_t Offset = (i + 1) * PowTwo32(MemPow) - Self->Size;
//                for (uint32_t j = Offset; j < Offset + PowTwo32(MemPow); ++j)
//                {
//                    canvas[j] = '*';
//                }
//            }
//        }
//    }
//    canvas[Self->Size] = '\0';
//}
//
//#define _CRTDBG_MAP_ALLOC /**< VS �ṩ�� malloc �ڴ�й©����  */
//#include <crtdbg.h>
//#include <stdlib.h>
//#include <assert.h>
//#include <stdio.h>
//#include <string.h>
//
//void UnitTestBuddy()
//{
//    _CrtDumpMemoryLeaks();
//    static char cmd[0xFFF] = { '\0' };
//    int max = 1024;
//    Buddy* buddy = BuddyNew(max, malloc, free);
//    uint32_t arg;
//    BuddyDump(buddy, cmd, sizeof(cmd));
//    while (1)
//    {
//        puts(cmd);
//        int res = scanf("%s %u", cmd, &arg);
//        if (strcmp(cmd, "alloc") == 0)
//        {
//            printf("allocated@%u\n", BuddyAlloc(buddy, arg));
//            BuddyDump(buddy, cmd, sizeof(cmd));
//        }
//        else if (strcmp(cmd, "free") == 0)
//        {
//            BuddyFree(buddy, arg);
//            BuddyDump(buddy, cmd, sizeof(cmd));
//        }
//        else if (strcmp(cmd, "size") == 0)
//        {
//            printf("Size: %u\n", BuddySize(buddy, arg));
//            BuddyDump(buddy, cmd, sizeof(cmd));
//        }
//        else if (strcmp(cmd, "sum") == 0)
//        {
//            printf("Size: %u\n", BuddyUsable(buddy));
//        }
//        else if (strcmp(cmd, "exit") == 0)
//        {
//            break;
//        }
//        else
//        {
//            BuddyDump(buddy, cmd, sizeof(cmd));
//        }
//    }
//    // BuddyDel(buddy);
//    _CrtDumpMemoryLeaks();
//}

#endif
