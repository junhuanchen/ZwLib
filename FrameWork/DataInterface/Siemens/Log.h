#ifndef LOG_H
#define LOG_H

#ifdef LINUX
#define LOG_OUT
#elif WIN32
#include <stdio.h>
#elif STM32
#define LOG_OUT
#include "usart.h"
#elif DOS
// NO NEED
#endif

#ifdef LOG_OUT
#define LogPut(format, ...) printf(format, ##__VA_ARGS__)
#define LogExec(level, func) if ( level ) (func)
#else
#define LogPut(format, ...)
#define LogExec(level, func)
#endif /* LOG_OUT */

#endif /* LOG_H */
