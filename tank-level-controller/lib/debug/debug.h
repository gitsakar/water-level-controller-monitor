#ifndef MY_DEBUG_H
#define MY_DEBUG_H

#if ENABLE_DEBUG == 1
#include <stdio.h>
#define TRACE(val, ...) printf(val"\n", ##__VA_ARGS__)
#else
#define TRACE(...)
#endif /*end if ENABLE_DEBUG == 1*/

#endif /*end ifndef MY_DEBUG_H*/