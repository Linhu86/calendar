#ifndef __UCOM_RTOS_YOCTO_H__
#define __UCOM_RTOS_YOCTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <sys/ioctl.h>       /* For ioctl calls */
#include <sys/types.h>

#include "ucom_types.h"
#include "ucom_rtos.h"

#define UCOM_RTOS_DEBUG

#ifdef UCOM_RTOS_DEBUG
    #define _RTOS_MEM_DUMP  _rtos_mem_dump
    #define UCOM_RTOS_TRACE ucom_log
#else
    #define _RTOS_MEM_DUMP
    #define UCOM_RTOS_TRACE
#endif

#define _RTOS_ERROR_SUCCESS	 0
#define _RTOS_ERROR_FAILURE	-1

void  _rtos_mem_dump(const void* mem_addr, size_t dump_len);
UComInt32 UComOsConvertSysStatus(int );



#endif

