#ifndef BUDDY_H
#define BUDDY_H

#include <stdint.h>
#include <stddef.h>

typedef void *(*buddy_new)(uint32_t); /** < 内存申请函数声明  */

typedef void( *buddy_del )(void *); /** < 内存释放函数声明  */

// 伙伴算法柔性数组 当前算法复杂度分别为,空间复杂度 O（N^2）,时间复杂度 O（LogN）
typedef struct buddy
{
    buddy_new New;
    buddy_del Del;
    uint32_t Size;      // 标记内存总大小
    int8_t MemPow[];  // 二叉树数组和其结点权值为可分配内存大小的幂值（藉由Log还原）
}Buddy;

// 标记结点权值,表示内存被使用。（负数的意图是小于所有正整数的幂指数值）
enum
{
    BuddyMemOut = -1,
};

Buddy* BuddyNew( uint32_t Size, buddy_new New, buddy_del Del );

void BuddyDel( Buddy* Self );

uint32_t BuddyAlloc( Buddy* Self, uint32_t Size );

void BuddyFree( Buddy* Self, uint32_t Offset );

uint32_t BuddySize( Buddy* Self, uint32_t Offset );

uint32_t BuddyUsable( Buddy* Self );

#endif//BUDDY_H
