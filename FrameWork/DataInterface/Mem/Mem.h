#ifndef MEM_H
#define MEM_H

typedef struct mem_mngr MemMngr;

#define MemError NULL

void MemMngrNew( MemMngr *Self, uint8_t *area, uint32_t size );

uint8_t MemMngrUsageRate( MemMngr *Self );

void MemMngrDel( MemMngr *Self );

void * MemMngrAlloc( MemMngr *Self, uint32_t size );

void MemMngrFree( MemMngr *Self, void * MemAddr );

#endif // MEM_H