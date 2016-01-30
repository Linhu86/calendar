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
    @param  name            Name of the thread. Note that in NUCLEUS OS the maximum length is 8 characters.
                            The name is copied to an internal data structure.
    @param  priority        Thread priority. Applications shall use only values defined in the enum #UComOsThreadPriority
    @param  stack           Pointer to the thread callstack. Memory has to be provided by caller.
                            Note that the pointer has to be byte aligned according to #UCOM_OS_THREAD_STACK_ALIGNMENT.
    @param  stack_size      Size of the thread callstack in bytes.
                            Note that for ARM RVDS compiler the size has to be a multiple of 8.
    @param  thread_fn       Pointer to the function which is called in the thread context. This function has
                            to return to end the thread. Thread function declaration shall start with #UCOM_PROCESS
    @param  thread_param    Pointer to user data which is passed to the thread function.
    @return                 Result value out of #UComCommonReturnCodes or #UComOsReturnCodes.
*/

Bool ThreadCreate(thread_hdl* thread,
                  const char8_t* name,
                  int32_t priority,
                  void* stack,
                  int32_t stack_size,
                  ThreadFn thread_fn,
                  void* thread_param);


/* Detach thread from main thread. */
Bool ThreadDetach(thread_hdl* thread);


/** Creates a queue.
    @param  queue       Pointer to the queue data which must be allocated by caller.
    @param  name        Queue unique name. Note that in NUCLEUS OS the maximum length is 8 characters.
    @param  msg_size    Message size for each message sent to the queue in bytes.
    @param  msg_count   Maximum number of messages in the queue.
    @return             Result value

}
*/
Bool QueueCreate( mqd_hdl*         queue,
                  const char8_t*   name,
                  int32_t          msg_size,
                  int32_t          msg_count );

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
    @param  suspend     Suspend mode. #UCOM_TRUE means to suspend the current thread until message is retrieved from the queue.
    @return             Result value */

Bool QueueReceive(mqd_hdl* queue, void* message, Bool suspend);

Bool FsOpen(file_hdl *file_hdl_ptr, const char8_t *path, int32_t mode);

void FsClose(file_hdl file_hdl_ptr);

Bool FsRead(file_hdl file_hdl_ptr, char8_t *buffer, int32_t size);

int32_t FsGetSize(file_hdl file_hdl_ptr);

void FsClose(file_hdl file_hdl_ptr);

#endif



