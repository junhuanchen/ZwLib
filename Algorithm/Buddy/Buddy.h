#ifndef BUDDY_H
#define BUDDY_H

#include <stdint.h>
#include <stddef.h>

typedef void *(*buddy_new)(uint32_t); /** < �ڴ����뺯������  */

typedef void( *buddy_del )(void *); /** < �ڴ��ͷź�������  */

// ����㷨�������� ��ǰ�㷨���Ӷȷֱ�Ϊ,�ռ临�Ӷ� O��N^2��,ʱ�临�Ӷ� O��LogN��
typedef struct buddy
{
    buddy_new New;
    buddy_del Del;
    uint32_t Size;      // ����ڴ��ܴ�С
    int8_t MemPow[];  // ���������������ȨֵΪ�ɷ����ڴ��С����ֵ������Log��ԭ��
}Buddy;

// ��ǽ��Ȩֵ,��ʾ�ڴ汻ʹ�á�����������ͼ��С����������������ָ��ֵ��
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
