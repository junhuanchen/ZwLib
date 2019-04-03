
#include "../../../Algorithm/Buddy/Buddy.h"

#include "Mem.h"

#include <stdlib.h>

typedef struct mem_mngr
{
    uint8_t * mem_area;
    Buddy * mem_mngr;
}MemMngr;

void MemMngrNew( MemMngr *Self, uint8_t *area, uint32_t size )
{
    Self->mem_mngr = BuddyNew( size, malloc, free );
    if ( MemError != Self->mem_mngr )
    {
        Self->mem_area = area;
    }
}

uint8_t MemMngrUsageRate( MemMngr *Self )
{
    return (MemError == Self->mem_mngr) ? 0 : BuddyUsable( Self->mem_mngr ) * 100.0 / Self->mem_mngr->Size;
}

void MemMngrDel( MemMngr *Self )
{
    if ( MemError == Self || MemError == Self->mem_mngr ) return;
    BuddyDel( Self->mem_mngr );
}

void * MemMngrAlloc( MemMngr *Self, uint32_t size )
{
    if ( MemError == Self || MemError == Self->mem_mngr ) return MemError;
    uint32_t offset = BuddyAlloc( Self->mem_mngr, size );
    void * MemAddr = (BuddyMemOut == offset) ? MemError : Self->mem_area + offset;
    return MemAddr;
}

void MemMngrFree( MemMngr *Self, void * MemAddr )
{
    if ( MemError == Self || MemError == Self->mem_mngr || MemError == MemAddr ) return;
    BuddyFree( Self->mem_mngr, (uint8_t*) MemAddr - Self->mem_area );
}

#ifdef UNIT_TEST
#include <assert.h>
#include <stdio.h>
static uint8_t Mem[64 * 1024];

void * testbuf[sizeof( Mem )];

int main( )
{
    MemMngr Stm32;
    MemMngrNew( &Stm32, Mem, sizeof( Mem ) );

    for ( int i = 0; i < sizeof( Mem ); i++ )
    {
        printf( "%d:\n", i );
        printf( "%hhu\n", MemMngrUsageRate( &Stm32 ) );
        testbuf[i] = MemMngrAlloc( &Stm32, 1 );
        assert( MemError != testbuf[i] );
        (*(uint8_t*) testbuf[i]) = i;
        printf( "%hhu\n", MemMngrUsageRate( &Stm32 ) );
    }
    for ( int i = 0; i < sizeof( Mem ); i++ )
    {
        printf( "%d:\n", i );
        printf( "%hhu\n", MemMngrUsageRate( &Stm32 ) );
        assert( (i % 0x100) == *(uint8_t*) testbuf[i] );
        MemMngrFree( &Stm32, testbuf[i] );
        printf( "%hhu\n", MemMngrUsageRate( &Stm32 ) );
    }
    system( "pause" );
    return 0;
}

#endif 