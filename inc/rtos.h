#ifndef __RTOS_H__
#define __RTOS_H__

#include "types.h"

#define THREAD_DEFAULT_STACK_SIZE 4096
#define THREAD_DEFAULT_PRIORITY   10

typedef void *(*ThreadFn)(void*);

/* Create a thread. */
Bool ThreadCreate(pthread_t* thread,     /* OS task reference */
                  const char8_t* name,  /* task name */
                  int32_t priority,     /* task priority */
                  void* stack,          /* pointer to start of task stack area */
                  int32_t stack_size,   /* task size */
                  ThreadFn thread_fn,   /* pointer to thread entry point */
                  void* thread_param);  /* thread argument pointer */


/* Detach thread from main thread. */
Bool ThreadDetach(pthread_t* thread);


#endif



