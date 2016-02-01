#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdio.h>

typedef enum {
    FAILURE = 0,
    SUCCESS = 1
} BOOLEAN;

#define IN
#define OUT

#define UNUSED(x) { x = 0;}

#ifdef OS_LINUX
#include <pthread.h>
#include <mqueue.h>

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
typedef pthread_mutex_t mutex_hdl;
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
#endif


#endif





