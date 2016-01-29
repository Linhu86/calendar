/* rtos wrapper for linux OS. */

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "rtos.h"
//#include "rtos_wrapper_linux.h"

/* Now ThreadCreate API only support to create a thread, but leave the interface to be extended other functionalities
in the future, e.g thread name, thread stack size, thread priority management. */

Bool ThreadCreate(pthread_t* thread,     /* OS task reference */
                  const char8_t* name,  /* task name */
                  int32_t priority,     /* task priority */
                  void* stack,          /* pointer to start of task stack area */
                  int32_t stack_size,   /* task size */
                  ThreadFn thread_fn,   /* pointer to thread entry point */
                  void* thread_param)   /* thread argument pointer */
{
  UNUSED(name);
  UNUSED(priority);
  UNUSED(stack);
  UNUSED(stack_size);

  Bool ret = FAILURE;

  if(0 != pthread_create(thread, NULL, thread_fn, (void *)(thread_param)))
  {
    CALENDER_DEBUG("Failed to create thread, error: %s.", strerror(errno));
    ret = FAILURE;
  }
  else
  {
    CALENDER_DEBUG("Succeed to create thread: %s pid: %lu.", name, pthread_self());
    ret = SUCCESS;
  }

  return ret;
}

Bool ThreadDetach(pthread_t* thread)
{
  Bool ret = FAILURE;

  if(0 != pthread_detach(*thread))
  {
    CALENDER_DEBUG("Failed to detach thread %lu, error: %s.", pthread_self(), strerror(errno));
    ret = FAILURE;
  }
  else
  {
    CALENDER_DEBUG("Succeed to detach thread %lu from main thread.", pthread_self());
    ret = SUCCESS;
  }

  return ret;
}



