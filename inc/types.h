#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdio.h>

typedef enum {
    FAILURE = 0,
    SUCCESS = 1
} BOOLEAN;

#define UNUSED(x) { x = 0;}

#ifdef DEBUG_INFO_ON
#define CALENDER_DEBUG(fmt, ...) { \
  printf("[TimeStamp: %d] [Pid:%lu] [Func:%s]: " #fmt "\n", (int)time(NULL), pthread_self(), __func__, ##__VA_ARGS__); \
}
#else
#define CALENDER_DEBUG
#endif



#ifdef OS_LINUX
typedef int Bool;
typedef char char8_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef float float32_t;
typedef double double64_t;
#else
typedef int Bool;
typedef char char8_t;
typedef signed int int_t;
typedef unsigned int uint_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef float float32_t;
typedef double double64_t;
#endif


#endif





