
// 1.0 原始版本来源 buddy2.h
// 1.1 改进版本，增加了静态二次幂值的映射，原结点存储幂指数间接获取，减少内存的占用。
// 1.2 重构版本，接口统一到规范。

#include "Buddy.h"

// ======================公共通用函数实现======================

static uint32_t MulByTwo( uint32_t Num )
{
	return Num * 2; // << 1
}

static uint32_t DivByTwo( uint32_t Num )
{
	return Num / 2; // >> 1
}

// 数组二叉树索引访问 树根结点索引
enum
{
	TreeRoot = 0,
};

// 数组二叉树索引访问 叶子父结点索引
static uint32_t LeafParent( uint32_t index )
{
	return DivByTwo( index + 1 ) - 1;
}

// 数组二叉树索引访问 叶子左子结点索引
static uint32_t LeafLeft( uint32_t index )
{
	return MulByTwo( index ) + 1;
}

// 数组二叉树索引访问 叶子右子结点索引
static uint32_t LeafRight( uint32_t index )
{
	return MulByTwo( index ) + 2;
}

// 判断是否为二次幂数值
static uint8_t IsPowOfTwo( uint32_t Num )
{
	return !( Num & ( Num - 1 ) );
}

// 将值调整为二次幂值（52 => 64, 67 => 128）
static uint32_t FixToPowOfTwo( uint32_t Num )
{
	static const int TreeMaxLayer = ( 8 * sizeof( uint32_t ) );
	for ( int i = 1; i != TreeMaxLayer; i = MulByTwo( i ) )
	{
		Num |= Num >> i;
	}
	return Num + 1;
}

// 返回较大值（有符号数能够区分被占用的块标记（-1被看作最小故可以不被选择））
static int8_t TreeMax( int8_t a, int8_t b )
{
	return a > b ? a : b;
}

//// 当需要2^64层的内存索引时
//static const char debruijn_64[ ] = { 
//    0, 1, 48, 2, 57, 49, 28, 3, 61, 58, 50, 42, 38, 29, 17, 4, 62, 55, 59, 36, 53, 51,
//    43, 22, 45, 39, 33, 30, 24, 18, 12, 5, 63, 47, 56, 27, 60, 41, 37, 16, 54, 35, 52, 
//    21, 44, 32, 23, 11, 46, 26, 40, 15, 34, 20, 31, 10, 25, 14, 19, 9, 13, 8, 7, 6 
//};
//#define LogTwo64(v) debruijn_64[(uint64_t)((v) * 0x3f79d71b4cb0a89UL) >> 58]

// deBruijn序列
static const int8_t debruijn[] = {
	0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
	31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9,
};

// 特殊log2 （查找二进制串右起的串到1为止的0的个数，例如：0100 => 3，101 = > 0，仅用于根据N = 2^Y中N的求Y值）
static int8_t LogTwo32( uint32_t N )
{
	return debruijn[( ( uint32_t ) ( ( ( N ) & -( N ) ) * 0x077CB531U ) ) >> 27];
}

// 改进存储原始二次幂值的幂指数后的增加一个获取二次幂树索引操作
// 可以降低空间复杂度四至八（64位）个常数，但时间复杂度将增加一个常数
// 根据索引求二次幂数值映射表（改进存储数据结构）
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

// ======================伙伴算法函数实现======================

Buddy* BuddyNew( uint32_t Size, buddy_new New, buddy_del Del )
{
	if ( Size > 0 && IsPowOfTwo( Size ) )
	{
		// 深度为 n 的完全二叉树结点总数 s 满足 s = n^2-1
		uint32_t tree_size = MulByTwo( Size ) - 1;
		Buddy* buddy = ( Buddy* ) New( sizeof( Buddy ) + tree_size * sizeof( int8_t ) );
		if ( NULL != buddy )
		{
			buddy->Size = Size;
			int8_t tree_pow = LogTwo32( Size ) + 1;
			for ( uint32_t i = 0; i != tree_size; i++ )
			{
				if ( IsPowOfTwo( i + 1 ) ) // 与深度有关，取决于tree_pow初始值
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
	// 判断数据不合法或内存已不足
	if ( Size == 0 || Self == NULL || PowTwo32( Self->MemPow[TreeRoot] ) < Size ) return BuddyMemOut;

	// 内存需求值修正回二次幂数值
	if ( 0 == IsPowOfTwo( Size ) ) Size = FixToPowOfTwo( Size );

	// 前序遍历查找二叉树数组中结点权值与需求内存大小相等的索引
	uint32_t index = TreeRoot;
	for ( uint8_t mem_pow = LogTwo32( Self->Size ); Size != PowTwo32( mem_pow ); mem_pow-- )
	{
		uint32_t mem_left = LeafLeft( index );
		index = ( PowTwo32( Self->MemPow[mem_left] ) >= Size ) ? mem_left : LeafRight( index );
	}

	Self->MemPow[index] = BuddyMemOut; // 标记该索引的内存被使用

	// 返回转换后的真实内存索引
	uint32_t Offset = ( index + 1 ) * Size - Self->Size;

	// 回溯修正父结点的剩余内存根权值(权值折半)
	while ( index )
	{
		index = LeafParent( index ); // 获取父结点索引
		Self->MemPow[index] = TreeMax( Self->MemPow[LeafLeft( index )], Self->MemPow[LeafRight( index )] );
	}

	return Offset;
}

void BuddyFree( Buddy* Self, uint32_t Offset )
{
	// 检查参数合法性
	if ( Offset >= Self->Size ) return;

	// 检查内存是否有被分配出去
	if ( Self->Size == PowTwo32( Self->MemPow[TreeRoot] ) ) return;

	// 根据内存索引转换为二叉树索引。
	// 由于不知道二叉树索引的内存块大小，故设 L = 1 则将从最小子结点向上回溯查找权值为BuddyMemOut的父节点，即可得到偏移量对应的块大小。
	// 因 MemPow = 0 => mem_size = 1 则 (Offset + Self->Size) / mem_size = (Offset + Self->Size)，结果等同于 BuddySize
	uint32_t mem_pow = 0, index = Offset + Self->Size - 1;

	// 根据最小子结点二叉树索引查找被标记的结点所在的内存块大小（即结点对应的层数）
	while ( BuddyMemOut != Self->MemPow[index] )
	{
		// 已到达树根索引仍查找不到被使用的内存修改后的BuddyMemOut,故退出函数
		if ( index == TreeRoot ) return;
		mem_pow++, index = LeafParent( index );
	}

	// 恢复该二叉树索引原本的权值
	Self->MemPow[index] = mem_pow;

	// 回溯修正二叉树索引父结点的原权值
	for ( uint32_t mem_left, mem_right; index; )
	{
		mem_pow++, index = LeafParent( index ); // 获取父结点索引
		mem_left = Self->MemPow[LeafLeft( index )];
		mem_right = Self->MemPow[LeafRight( index )];

		// 若左右内存大小和满足此层内存块大小即可合并置为当前父结点权值
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
	// 检查参数合法性
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

	// 假设是 四片 四字节 内存片
	static uint8_t GlobalBlock[4 * BlockSize];

	for ( int i = 0; i < sizeof( GlobalBlock ); i++ ) GlobalBlock[i] = i;

	Buddy *buddy = BuddyNew( ( sizeof( GlobalBlock ) / BlockSize ), malloc, free );

	if ( NULL != buddy )
	{
		int * value, pos, len;
		// 更换两者的内存的测试

		PoolStatic MemMap;
		PoolStatic MemQueue;

		// 要一片
		len = 1;
		pos = BuddyAlloc( buddy, len );
		assert( 0 == pos );
		PoolStaticInit( &MemMap, GlobalBlock + pos, BlockSize * len );

		for ( int i = 0; i < BlockSize * len + 1; i++ )
		{
			value = PoolStaticNew( &MemMap, BlockSize * 1 );
			if ( value ) printf( "i:%d value %d\n", i, *value );
		}

		// 要两片
		len = 2;
		pos = BuddyAlloc( buddy, len );
		assert( 2 == pos );
		PoolStaticInit( &MemQueue, GlobalBlock + pos, BlockSize * len );

		for ( int i = 0; i < BlockSize * len + 1; i++ )
		{
			value = PoolStaticNew( &MemQueue, BlockSize * 1 );
			if ( value ) printf( "i:%d value %d\n", i, *value );
		}

		// 再次尝试要两片会失败
		pos = BuddyAlloc( buddy, 2 );
		assert( BuddyMemOut == pos );
		
		// 查看剩余有一片
		assert( 1 == BuddyUsable( buddy ) );

		// 释放自己拥有的一片就有可能拿到两片
		// 或者将其他人的内存收缩，这样就可以提取到其他内存。

		// 比如将旁边（如果有链表做统计即可得知）的队列内存收缩到一片。
		// 如果为了完整实现动态内存回收，这时候应该先给队列转移内存，但这需要一个共享指针所有指向该区域的指针均转移
		// 该例子目前仍然不可以使用，主要问题出在各类容器需要建立在二级的托管指针之上才能变更其拥有的内存，这类似C#的语言架构。
		// 但目前可以调整两者拥有的内存，不需要借助该模块也可以完成。
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
		// 从队列那边转移给映射容器

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
//#define _CRTDBG_MAP_ALLOC /**< VS 提供的 malloc 内存泄漏检测宏  */
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
