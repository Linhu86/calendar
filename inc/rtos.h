#ifndef __RTOS_H__
#define __RTOS_H__

#include "types.h"

#define THREAD_DEFAULT_STACK_SIZE 4096

#define THREAD_DEFAULT_PRIORITY   10

#define MAX_MSG_QUEUE_SIZE    1024

#define MAX_MSG_QUEUE_NUM     10

typedef enum{
    MODE_NON_BLOCK = 0,
    MODE_BLOCK = 1
}BLOCK_MODE;

typedef enum{
  OPEN_MODE_READ_ONLY = 0,
  OPEN_MODE_READ_WRITE = 1,
  OPEN_MODE_APPEND = 2
}OPEN_MODE;


typedef void *(*ThreadFn)(void*);

/** Creates a thread
    @param  thread          Pointer to the thread data which must be allocated by caller.
    @param  name            Name of the thread.
                            The name is copied to an internal data structure.
    @param  priority        Thread priority.
    @param  stack           Pointer to the thread callstack. Memory has to be provided by caller.
    @param  stack_size      Size of the thread callstack in bytes.
                            Note that for ARM RVDS compiler the size has to be a multiple of 8.
    @param  thread_fn       Pointer to the function which is called in the thread context. This function has
                            to return to end the thread.
    @param  thread_param    Pointer to user data which is passed to the thread function.
    @return                 Result value.
*/

Bool ThreadCreate(thread_hdl* thread,
                            const char8_t* name,
                            uint32_t priority,
                            void* stack,
                            uint32_t stack_size,
                            ThreadFn thread_fn,
                            void* thread_param);


/* Detach thread from main thread. */
Bool ThreadDetach(thread_hdl* thread);


/** Creates a queue.
    @param  queue       Pointer to the queue data which must be allocated by caller.
    @param  name        Queue unique name.
    @param  msg_size    Message size for each message sent to the queue in bytes.
    @param  msg_count   Maximum number of messages in the queue.
    @return             Result value

}
*/
Bool QueueCreate( mqd_hdl*  queue,
                            const char8_t*  name,
                            uint32_t msg_size,
                            uint32_t msg_count);

/** Deletes a queue.
    @param  queue       Pointer to the queue.
    @return             Result value out of  */

void QueueDelete(mqd_hdl* queue, const char8_t *name);


/** Sends a message to the specified queue.
    @param  queue       Pointer to the queue.
    @param  message     Message to be sent to the queue
    @param  suspend     Suspend mode. Specifies whether or not to suspend the calling task if the queue is full.
    @return             Result value */

Bool QueueSend(mqd_hdl* queue, const void* message, Bool suspend);


/** Receives a message from give queue.
    @param  queue       Pointer to the queue.
    @param  message     Message to be received from the queue. Must be provided by caller.
    @param  suspend     Suspend mode.
    @return             Result value */

Bool QueueReceive(mqd_hdl* queue, void* message, Bool suspend);

/** Block a mutex lock.
    @param  mutex       Pointer to the mutex.
    @return             Result value */

Bool MutexLock(mutex_hdl *lock);

/** Release a mutex lock.
    @param  mutex       Pointer to the mutex.
    @return             Result value */

Bool MutexUnLock(mutex_hdl *lock);


/** Destroy a mutex lock.
    @param  mutex       Pointer to the mutex.
    @return             Result value */

Bool MutexDestroy(mutex_hdl *lock);


/** Open a file .
    @param  file_hdl_ptr  Pointer to the file descriptor.
    @param  path          File path.
    @param  mode          File open mode.
    @return               Result value */
Bool FsOpen(file_hdl *file_hdl_ptr, const char8_t *path, const uint32_t mode);


/** Read bytes from a file.
    @param  file_hdl_ptr  Pointer to the file descriptor.
    @param  buffer        read buffer.
    @param  size          size to be retrieved from file.
    @return               Result value */
Bool FsRead(file_hdl file_hdl_ptr, char8_t *buffer, uint32_t size);


/** Get total file size.
    @param  file_hdl_ptr  Pointer to the file descriptor.
    @return               file size */
uint32_t FsGetSize(file_hdl file_hdl_ptr);

/** Close a file .
    @param  file_hdl_ptr  Pointer to the file descriptor.
    @return               Result value */
void FsClose(file_hdl file_hdl_ptr);

/* get system time stamp */
int32_t get_time_stamp(void);

/* install signal handler */
void signal_handler_install(void);


#endif



