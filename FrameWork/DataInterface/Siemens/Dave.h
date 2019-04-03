#ifndef DAVE_H
#define DAVE_H

#include "DaveInterface.h"

uint8_t DAVE_DECL daveGetOneBytefrom(void *buffer);

uint16_t DAVE_DECL daveGetTwoBytefrom(void *buffer);

uint32_t DAVE_DECL daveGetFourBytefrom(void *buffer);

/*
    error code to message string conversion:
*/
char * DAVE_DECL daveStrerror(int32_t code);

/*
    Hex dump:
*/
void DAVE_DECL _daveDumpHex(const char *name, uint8_t *buf, int32_t len);

char * DAVE_DECL daveAreaName(uint8_t n);

#endif // DAVE_H
