/* rtos wrapper for linux OS. */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <mqueue.h>

#include "rtos.h"

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

/** Message queue crate API.*/
Bool QueueCreate( mqd_hdl*          queue,
                            const char8_t*   name,
                            uint32_t             msg_size,
                            uint32_t             msg_count )
{
  Bool ret;
  mqd_t mq;
  struct mq_attr attr;

  if(NULL == queue || NULL == name)
  {
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
  Bool ret;
  ssize_t bytes_write = 0;
  UNUSED(suspend);

  if(NULL == queue)
  {
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
  Bool ret;
  ssize_t bytes_read = 0;
  UNUSED(suspend);

  if(NULL == queue)
  {
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
    CALENDER_DEBUG("Success to receive data");
    ret = SUCCESS;
  }

  return ret;
}

Bool FsOpen(file_hdl *file_hdl_ptr, const char8_t *path, const uint32_t mode)
{
  FILE *fd;
  Bool ret;
  char *open_mode=NULL;

  if(NULL == file_hdl_ptr || NULL == path)
  {
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
  Bool ret;

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
  uint32_t size = 0;
  fseek(file_hdl_ptr, 0L, SEEK_END);
  size = ftell(file_hdl_ptr);
  rewind(file_hdl_ptr);
  return size;
}

void FsClose(file_hdl file_hdl_ptr)
{
  fclose(file_hdl_ptr);
}

