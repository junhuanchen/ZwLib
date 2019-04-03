#ifndef ZW_MEMORY_H
#define ZW_MEMORY_H

#include "../../Struct/Pool/PoolStatic.h"

// ��ȡ�ڴ�ռ���ʱ��
enum ZwMemoryFlag
{
    ZwMemoryQueue,
    ZwMemoryMap,
    ZwMemoryZwData,
};

//�ڴ��������
void ZwMemoryReInit(uint32_t QueueAreaLen, uint32_t MapAreaLen, uint32_t ZwDataAreaLen);
// ��ȡʣ����
float ZwMemoryGetUsageRate(enum ZwMemoryFlag Flag);
// �����ڴ�����ӿ�
void * ZwMemoryQueueAlloc(uint32_t Size);
// �����ڴ�����ӿ�
void * ZwMemoryMapAlloc(uint32_t Size);
// �����ڴ�����ӿ�
void * ZwMemoryZwDataAlloc(uint32_t Size);
// �ͷ��ڴ� // �˴�Ϊͳһ�ͷŽӿ�
void ZwMemoryFree(void * Data);

#endif // ZW_MEMORY_H
