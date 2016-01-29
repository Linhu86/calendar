#ifndef __TYPES_H__
#define __TYPES_H__

enum {
    FALSE = 0,
    TRUE = 1
};

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





