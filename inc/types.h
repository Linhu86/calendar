#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdio.h>
#include <pthread.h>
#include <mqueue.h>

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
typedef int int32_t;
typedef unsigned int uint32_t;
typedef char char8_t;
typedef unsigned char uchar8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef float float32_t;
typedef double double64_t;
typedef pthread_t thread_hdl;
typedef mqd_t mqd_hdl;
typedef FILE* file_hdl;
#else
typedef int Bool;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef char char8_t;
typedef unsigned char uchar8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef float float32_t;
typedef double double64_t;
typedef pthread_t thread_hdl;
typedef mqd_t mqd_hdl;
#endif


#endif





