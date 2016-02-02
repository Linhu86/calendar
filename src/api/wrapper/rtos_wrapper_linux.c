/* rtos_wrapper_linux.c
 rtos wrapper for linux OS. */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <mqueue.h>
#include <signal.h>

#include "rtos.h"
#include "common_include.h"

extern int32_t calendar_exit;

extern mutex_hdl calendar_lock;

/* signal handler */
static void signal_handler()
{
  calendar_quit();
}


/* Now ThreadCreate API only support to create a thread, but leave the interface to be extended other functionalities
in the future, e.g thread name, thread stack size, thread priority management. */
Bool ThreadCreate(thread_hdl* thread,   /* OS thread reference */
                  const char8_t* name,  /* thread name */
                  uint32_t priority,     /* thread priority */
                  void* stack,          /* pointer to start of task stack area */
                  uint32_t stack_size,   /* thread size */
                  ThreadFn thread_fn,   /* pointer to thread entry point */
                  void* thread_param)   /* thread argument pointer */
{
  UNUSED(name);
  UNUSED(priority);
  UNUSED(stack);
  UNUSED(stack_size);

  if(NULL == thread)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }

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


/* Thread detach API*/
Bool ThreadDetach(thread_hdl* thread)
{
  Bool ret = FAILURE;

  if(NULL == thread)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }

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

/** Create a queue.
    @param  queue       Pointer to the queue.
    @param  name        queue name
    @param  msg_size    queue size
    @param  msg_count   max queue num
    @return             Result value out of  */

Bool QueueCreate( mqd_hdl*         queue,
                  const char8_t*   name,
                  uint32_t         msg_size,
                  uint32_t         msg_count )
{
  Bool ret = FAILURE;
  mqd_t mq;
  struct mq_attr attr;

  if(NULL == queue || NULL == name)
  {
     CALENDER_DEBUG("Invalid parameters.");
     return FAILURE;
  }

  /* initialize the queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = msg_count;
  attr.mq_msgsize = msg_size;
  attr.mq_curmsgs = 0;

  /* create the message queue */
  mq = mq_open(name, O_CREAT | O_RDWR, 0644, &attr);

  if(-1  == mq)
  {
    CALENDER_DEBUG("Failed to create message queue, error: %s.", strerror(errno));
    ret = FAILURE;
  }
  else
  {
    CALENDER_DEBUG("Succeed to create message queue.");
    ret = SUCCESS;
  }

  *queue = mq;

  return ret;
}

/** Deletes a queue.
    @param  queue       Pointer to the queue.
    @param  name        queue name
    @return             Result value out of  */
void QueueDelete(mqd_hdl* queue, const char8_t *name)
{
  if(NULL == queue || NULL == name)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return;
  }

  mq_close(*queue);
  mq_unlink(name);
}


/** Sends a message to the specified queue.
    @param  queue       Pointer to the queue.
    @param  message     Message to be sent to the queue
    @param  suspend     Suspend mode. Specifies whether or not to suspend the calling task if the queue is full.
    @return             Result value */
Bool QueueSend(mqd_hdl* queue, const void* message, Bool suspend)
{
  Bool ret = FAILURE;
  ssize_t bytes_write = 0;
  UNUSED(suspend);

  if(NULL == queue)
  {
     CALENDER_DEBUG("Invalid parameters.");
     return FAILURE;
  }

  bytes_write = mq_send(*queue, message, MAX_MSG_QUEUE_SIZE, 0);

  if(-1 == bytes_write)
  {
    CALENDER_DEBUG("Failed to send message, error: %s.", strerror(errno));
    ret = FAILURE;
  }
  else
  {
    CALENDER_DEBUG("Succeed to send message.");
    ret = SUCCESS;
  }
  return ret;
}


/** Receives a message from give queue.
    @param  queue       Pointer to the queue.
    @param  message     Message to be received from the queue. Must be provided by caller.
    @param  suspend     Suspend mode. #UCOM_TRUE means to suspend the current thread until message is retrieved from the queue.
    @return             Result value */
Bool QueueReceive(mqd_hdl* queue, void* message, Bool suspend)
{
  Bool ret = FAILURE;
  ssize_t bytes_read = 0;
  UNUSED(suspend);

  if(NULL == queue)
  {
     CALENDER_DEBUG("Invalid parameters.");
     return FAILURE;
  }

  bytes_read = mq_receive(*queue, message, MAX_MSG_QUEUE_SIZE, NULL);

  if(-1 == bytes_read)
  {
    CALENDER_DEBUG("Failed to receive data, error: %s.", strerror(errno));
    ret = FAILURE;
  }
  else
  {
    CALENDER_DEBUG("Success to receive data.");
    ret = SUCCESS;
  }

  return ret;
}

/** Create a mutex lock.
    @param  mutex       Pointer to the mutex.
    @return             Result value */

Bool MutexCreate(mutex_hdl *lock)
{
  if(NULL == lock)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }

  if(pthread_mutex_init(lock, NULL) != 0)
  {
    CALENDER_DEBUG("mutex init failed :%s", strerror(errno));
    return FAILURE;
  }

  return SUCCESS;
}

/** Block a mutex lock.
    @param  mutex       Pointer to the mutex.
    @return             Result value */

Bool MutexLock(mutex_hdl *lock)
{
  if(NULL == lock)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }
  
  pthread_mutex_lock(lock);

  return SUCCESS;
}

/** Release a mutex lock.
    @param  mutex       Pointer to the mutex.
    @return             Result value */

Bool MutexUnLock(mutex_hdl *lock)
{
  if(NULL == lock)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }

  pthread_mutex_unlock(lock);

  return SUCCESS;
}

/** Destroy a mutex lock.
    @param  mutex       Pointer to the mutex.
    @return             Result value */

Bool MutexDestroy(mutex_hdl *lock)
{
  if(NULL == lock)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }
  pthread_mutex_destroy(lock);
  return SUCCESS;
}


Bool FsOpen(file_hdl *file_hdl_ptr, const char8_t *path, const uint32_t mode)
{
  FILE *fd = NULL;
  Bool ret = FAILURE;
  char8_t *open_mode=NULL;

  if(NULL == file_hdl_ptr || NULL == path)
  {
     CALENDER_DEBUG("Invalid parameters.");
     return FAILURE;
  }

  switch(mode){
    case OPEN_MODE_READ_ONLY:
      open_mode = "r";
      break;

    case OPEN_MODE_READ_WRITE:
      open_mode = "w+";
      break;

    case OPEN_MODE_APPEND:
      open_mode = "a+";
      break;

    default:
      open_mode = "r";
      break;
  }

  fd = fopen(path, open_mode);
  if(NULL == fd)
  {
    CALENDER_DEBUG("Failed to open file, error: %s.", strerror(errno));
    ret = FAILURE;
  }
  else
  {
    CALENDER_DEBUG("Success to open file");
    ret = SUCCESS;
  }

  *file_hdl_ptr = fd;
  return ret;
}

Bool FsRead(file_hdl file_hdl_ptr, char8_t *buffer, uint32_t size)
{
  Bool ret = FAILURE;

  if(NULL == file_hdl_ptr || NULL == buffer)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }

  if(size != fread(buffer,1,size,file_hdl_ptr))
  {
    CALENDER_DEBUG("Filed to read file, error %s", strerror(errno));
    ret = FAILURE;
  }
  else
  {
    CALENDER_DEBUG("Succeed to read file");
    ret = SUCCESS;
  }
  return ret;
}

uint32_t FsGetSize(file_hdl file_hdl_ptr)
{
  if(NULL == file_hdl_ptr)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return FAILURE;
  }

  uint32_t size = 0;
  fseek(file_hdl_ptr, 0L, SEEK_END);
  size = ftell(file_hdl_ptr);
  rewind(file_hdl_ptr);
  return size;
}

void FsClose(file_hdl file_hdl_ptr)
{
  if(NULL == file_hdl_ptr)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return;
  }

  fclose(file_hdl_ptr);
}

/* register signal handler should be linux OS dependent*/
void signal_handler_install(void)
{
  signal(SIGINT,  signal_handler); // Ctrl+C handle
  signal(SIGTERM, signal_handler); // Terminate handle
  signal(SIGUSR1, signal_handler); // Interrupt delay
  signal(SIGSEGV, signal_handler); // Segmentation fault
  signal(SIGKILL, signal_handler); // kill process
}

/* get system time stamp*/
int32_t get_time_stamp(void)
{
  return (int32_t)time(NULL);
}


