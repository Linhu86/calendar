/*******************************************************************************
 *
 * Copyright (C) u-blox Italia S.p.A.
 * u-blox Italia S.p.A., Sgonico, Italy
 *
 * All rights reserved.
 *
 * This source file is the sole property of u-blox Italia S.p.A. Reproduction or utilization of this
 * source in whole or part is forbidden without the written consent of u-blox Italia S.p.A.
 *
 *
 ******************************************************************************/

/***********************************************************
 *
 * Revision Information:
 * File Name: ucore/rtos/src/linux/src/ucom_rtos_linux.c
 *
 ***********************************************************
 *
 * History:
 *
 * Date          Author           Comment
 ***********************************************************
 * 10/02/16      Linhu Ying       Created
 *
 ***********************************************************/


#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <linux/rtc.h>
#include "ucom_rtos_linux.h"

#include "ucom_rtos_yocto.h"

/* FUNCTION-DESCRIPTION
**
** FUNCTION-NAME: UComTrapGenerateSwTrap
**
** PROCESSING:
**
**
** RETURN VALUE:
**
**
*/
void UComTrapGenerateSwTrap(
                           UComUInt32 log_data_size,
                           const void *log_data,
                           UComUInt32 line,
                           const UComChar* filename )
{

  /* BASIC IMPLEMENTATION */
  fprintf(stdout, "\r\n\r\n*************************************************************************************************************************");
  fprintf(stdout, "\r\n\r\n"
                  "  UCOM Exception:\r\n"
                  "    log_data_size = %d\r\n"
                  "    log_data      = %s\r\n"
                  "    file          = %s : %d\r\n",
                  (int)log_data_size,
                  (char *)log_data,
                  filename, (int)line);
  fprintf(stdout, "\r\n*************************************************************************************************************************\r\n");

  fflush(stdout);

  exit((int)log_data_size);

}


void UComTrapStoreException (
                           UComUInt32 log_data_size,
                           const void *log_data,
                           UComUInt32 line,
                           const UComChar* filename )

{
  /* BASIC IMPLEMENTATION, just write exception to STDOUT */
  fprintf(stdout, "\r\n\r\n*************************************************************************************************************************");
  fprintf(stdout, "\r\n\r\n"
                  "  UCOM Exception:\r\n"
                  "    log_data_size = %d\r\n"
                  "    log_data      = %s\r\n"
                  "    file          = %s : %d\r\n",
                  (int)log_data_size,
                  (char *)log_data,
                  filename, (int)line);
  fprintf(stdout, "\r\n*************************************************************************************************************************\r\n");

  fflush(stdout);


}

/*  -------------------------------------------------------------------------
    UComOsIrqGlobalDisable
    ------------------------------------------------------------------------- */
UComBool UComOsIrqGlobalDisable(void)
{
//#error
  return UCOM_TRUE;
} // UComOsIrqGlobalDisable

/*  -------------------------------------------------------------------------
    UComOsIrqGlobalEnable
    ------------------------------------------------------------------------- */
void UComOsIrqGlobalEnable(const UComBool enabled)
{
  UNUSED(enabled);
} // UComOsIrqGlobalEnable

/*  -------------------------------------------------------------------------
    UComOsConvertSysStatus
    ------------------------------------------------------------------------- */
UComInt32 UComOsConvertSysStatus(int status)
{
  UComInt32 rc;

  switch (status)
  {
    case _RTOS_ERROR_SUCCESS:
      rc = UCOM_SUCCESS;
      break;

    case ENOMSG:
      rc = UCOM_OS_QUEUE_EMPTY;
      break;

    case EAGAIN:
      rc = UCOM_OS_QUEUE_FULL;
      break;

    case _RTOS_ERROR_FAILURE:
    default:
      rc = UCOM_FAILURE;
      break;
  }

  return rc;
}

/* -----------------------------------------------------------------------------------
---------------------------------------THREADS---------------------------------------
--------------------------------------------------------------------------------------*/

typedef struct RTOS_THREAD
{
  pthread_t            handle;
  unsigned int         stackSize;
  const char*          name;
  UComOsThreadFn       threadRoutine;
  void*                params;
  UComOsThreadPriority priority;
} RTOS_THREAD_T;

/* Thread callback routine */
void * thread_start_routine (void *arg)
{
  if(UCOM_NULL != arg)
  {
    ((RTOS_THREAD_T*)arg)->threadRoutine(((RTOS_THREAD_T*)arg)->params);
  }

  return (void*)NULL;
}


/*  -------------------------------------------------------------------------
    UComOsThreadCreate
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadCreate(UComOsThread* thread,
                             const UComChar* name,
                             UComOsThreadPriority priority,
                             void* stack,
                             UComUInt32 stack_size,
                             UComOsThreadFn thread_fn,
                             void* thread_param)
{
  RTOS_THREAD_T *thread_hdl = (RTOS_THREAD_T*)thread;
  UNUSED(stack);

  if(UCOM_NULL == thread)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }

  if(UCOM_NULL == thread_fn)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  /* VZ: on linux the minimum stack size shall be PTHREAD_STACK_MIN */
  if(stack_size < PTHREAD_STACK_MIN)
  {
    stack_size = PTHREAD_STACK_MIN;
  }

  thread_hdl->handle = 0;
  thread_hdl->name = name;
  thread_hdl->stackSize = stack_size;
  thread_hdl->threadRoutine = thread_fn;
  thread_hdl->params = thread_param;
  thread_hdl->priority = priority;  

  return UCOM_SUCCESS;
}

/*  -------------------------------------------------------------------------
    UComOsThreadStart
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadStart(UComOsThread* thread)
{
  int ret = 0;
  pthread_attr_t attr;
  int policy = 0, prio = 0;

  if(UCOM_NULL == thread)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_THREAD_T *thread_hdl = (RTOS_THREAD_T*)thread;

  if(UCOM_NULL == thread_hdl)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  /* Initialize thread creation attributes */
  ret = pthread_attr_init(&attr);
  if(0 != ret)
  {
    return UComOsConvertSysStatus(ret);
  }

  ret = pthread_attr_setstacksize(&attr, (thread_hdl->stackSize);
  
  /* will return EINVAL if The stack size is less than PTHREAD_STACK_MIN (16384) bytes. */
  if(0 != ret)
  {
    return UComOsConvertSysStatus(ret);
  }

  /* JOINABLE should already be the default detach state mode */
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  ret = pthread_create(&thread_hdl->handle, &attr, thread_start_routine, (RTOS_THREAD_T*)thread);

  /* Destroy the thread attributes object, since it is no longer needed */
  pthread_attr_destroy(&attr);
  if(0 != ret)
  {
    ucom_log("\n UComOsThreadStart failed with error: %d\n", errno);
    return UComOsConvertSysStatus(ret);
  }

  ucom_log("UComOsThreadStart thread handle: %d\n", ((RTOS_THREAD_T*)thread)->handle);
  return UCOM_SUCCESS;
} // UComOsThreadStart

/*  -------------------------------------------------------------------------
    UComOsThreadDelete
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadDelete(UComOsThread* thread)
{ 
  if(UCOM_NULL == thread)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
   
  RTOS_THREAD_T *thread_hdl = (RTOS_THREAD_T*)thread;
   
  if(UCOM_NULL == thread_hdl)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }
 
  ucom_log("deleting thread handle = %d\n", thread_hdl->handle);
  if(0 != thread_hdl->handle)
  {
    pthread_cancel(thread_hdl->handle);
    if (0 != pthread_join(thread_hdl->handle, NULL))
    {
      ucom_log("deleting thread handle = %d failed with error %d\n", thread_hdl->handle, errno);
      return UComOsConvertSysStatus(errno);
    }
    thread_hdl->handle = 0;
  }

  return UCOM_SUCCESS;
}


/*  -------------------------------------------------------------------------
    UComOsThreadTerminate
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadTerminate(UComOsThread* thread)
{
  RTOS_THREAD_T *thread_hdl = (RTOS_THREAD_T*)thread;

  //ucom_log("\n terminating thread handle = %d\n", ((RTOS_THREAD_T*)thread)->handle);
  if (0 != thread_hdl->handle)
  {
    //ucom_log("\n\n\n ------------ Called pthread_cancel -------------- \n\n\n");
    pthread_cancel(thread_hdl->handle);
    if (0 != pthread_join(thread_hdl->handle, NULL) )
    {
      //ucom_log("\n terminating thread handle = %d failed with error %d\n", ((RTOS_THREAD_T*)thread)->handle, errno);
      return UComOsConvertSysStatus(errno);
    }
    thread_hdl->handle = 0;
  }

  return UCOM_SUCCESS;
} // UComOsThreadTerminate

/*  -------------------------------------------------------------------------
    UComOsThreadGetCurrent
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadGetCurrent(UComOsThread** thread)
{
  if(UCOM_NULL == thread)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_THREAD_T *thread_hdl = (RTOS_THREAD_T*)thread;
  
  if(UCOM_NULL == thread_hdl)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  if(*thread)
  {
    thread_hdl->handle = pthread_self();
  }

  return UCOM_SUCCESS;
}

/*  -------------------------------------------------------------------------
    UComOsThreadSetPriority
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadSetPriority(UComOsThread* thread, UComOsThreadPriority priority)
{
  int ret = 0, policy = 0;
  struct sched_param schedParams;

  if(UCOM_NULL == thread)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_THREAD_T *thread_hdl = (RTOS_THREAD_T*)thread;

  if(UCOM_NULL == thread_hdl)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  RTOS_THREAD_T* thread_hdl = (RTOS_THREAD_T*)thread;

  ret = pthread_getschedparam(thread_hdl->handle, &policy, &schedParams);

  schedParams.sched_priority = priority;

  ret = pthread_setschedparam(thread_hdl->handle, policy,&schedParams);

  return UComOsConvertSysStatus(errno);
} 


/*  -------------------------------------------------------------------------
    UComOsThreadGetPriority
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadGetPriority(UComOsThread* thread, UComOsThreadPriority* priority)
{
  int ret = 0, policy = 0;
  struct sched_param schedParams;

  if(UCOM_NULL == thread)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_THREAD_T *thread_hdl = (RTOS_THREAD_T*)thread;

  if(UCOM_NULL == thread_hdl)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  ret = pthread_getschedparam(thread_hdl->handle, &policy, &schedParams);

  if(*priority)
    *priority = schedParams.sched_priority;

  return UComOsConvertSysStatus(ret);
} 

/*  -------------------------------------------------------------------------
    UComOsThreadSuspend
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadSuspend(UComOsThread* thread)
{
  return UCOM_ERROR_NOT_SUPPORTED;
} // UComOsThreadSuspend

/*  -------------------------------------------------------------------------
    UComOsThreadResume
    ------------------------------------------------------------------------- */
UComInt32 UComOsThreadResume(UComOsThread* thread)
{
  return UCOM_ERROR_NOT_SUPPORTED;
} // UComOsThreadResume


/* -----------------------------------------------------------------------------------
---------------------------------------SEMAPHORES---------------------------------------
ucom_rtos_semaphore.c
--------------------------------------------------------------------------------------*/

typedef struct RTOS_SEMAPHORE
{
  sem_t*              handle;
  unsigned short int  count;
  const char*         name;
} RTOS_SEMAPHORE_T;

/* interrupt safe functions */

static inline int sem_wait_nointr(sem_t *sem)
{
  while(sem_wait(sem))
  {
    if(EINTR != errno)
    return -1;
  }
  return 0;
}


/*  -------------------------------------------------------------------------
    UComOsSemCreate
    ------------------------------------------------------------------------- */

UComInt32 UComOsSemCreate(UComOsSem* sem_data, const UComChar* name, UComUInt32 init_value, UComUInt32 flags)
{

  if(UCOM_NULL == sem_data)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }

  if(UCOM_NULL == name)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  RTOS_SEMAPHORE_T sem_hdl = (RTOS_SEMAPHORE_T *)sem_data;

  UNUSED(flags);
  
  /*
   * akot
   * using named semaphores
   * let sem_open to always succeed by unlink a potentially existing semaphore name
   *
   */

  sem_unlink(name);

  sem_hdl->count = init_value;
  sem_hdl->name  = name;
  sem_hdl->handle = sem_open(name, O_CREAT | O_EXCL, O_RDWR, init_value);

  if(SEM_FAILED == sem_hdl->handle)
  {
      return UComOsConvertSysStatus(errno);
  }

  return UCOM_SUCCESS;
} // UComOsSemCreate


/*  -------------------------------------------------------------------------
    UComOsSemDelete
    ------------------------------------------------------------------------- */
UComInt32 UComOsSemDelete(UComOsSem* sem_data)
{
  if(UCOM_NULL == sem_data)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }

  RTOS_SEMAPHORE_T sem_hdl = (RTOS_SEMAPHORE_T *)sem_data;


  if(UCOM_NULL == sem_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }

  sem_close(((_RTOS_SEMAPHORE*)sem_data)->handle);

  if (0 != sem_unlink(sem_hdl->name))
  {
      return UComOsConvertSysStatus(errno);
  }

  sem_hdl->handle = NULL;
  sem_hdl->count = 0;
  sem_hdl->name = 0;

  return UCOM_SUCCESS;
}


/*  -------------------------------------------------------------------------
    UComOsSemObtain
    ------------------------------------------------------------------------- */
UComInt32 UComOsSemObtain(UComOsSem* sem_data, UComUInt32 min_timeout, UComUInt32 max_timeout)
{
  if(UCOM_NULL == sem_data)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_SEMAPHORE_T sem_hdl = (RTOS_SEMAPHORE_T *)sem_data;

  if(UCOM_NULL == sem_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }

  /*
   * akot
   * interrupt safe sem_wait call used instead
   */
  //if ( 0 != sem_wait(((_RTOS_SEMAPHORE*)sem_data)->handle) )
  if(0 != sem_wait_nointr(sem_hdl->handle)
  {
    return UComOsConvertSysStatus(errno);
  }

  if(sem_hdl->count > 0)
  {
   sem_hdl->count--;
  }

  return UCOM_SUCCESS;
}

/*  -------------------------------------------------------------------------
    UComOsSemRelease
    ------------------------------------------------------------------------- */
UComInt32 UComOsSemRelease(UComOsSem* sem_data)
{
  if(UCOM_NULL == sem_data)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
    
  RTOS_SEMAPHORE_T sem_hdl = (RTOS_SEMAPHORE_T *)sem_data;
    
  if(UCOM_NULL == sem_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }

  if(0 != sem_post(sem_hdl->handle))
  {
    return UComOsConvertSysStatus(errno);
  }
  
  sem_hdl->count++;
  return UCOM_SUCCESS;
}


/* -----------------------------------------------------------------------------------
---------------------------------------QUEUES---------------------------------------
ucom_rtos_queue.c
or
ucom_rtos_msg_queue.c
--------------------------------------------------------------------------------------*/
typedef struct RTOS_QUEUE
{
  mqd_t  handle;
  size_t  messageSize;
  const char* name;
} RTOS_QUEUE_T;


/*  -------------------------------------------------------------------------
    UComOsQueueCreate
    ------------------------------------------------------------------------- */
UComInt32 UComOsQueueCreate(UComOsQueue* queue, const UComChar* name, void* start_addr,
                          UComUInt32 msg_size, UComUInt32 msg_count)
{
  int mqOpenFlags;
  mode_t mqOpenMode;
  char mqName[255];
  struct mq_attr mqAttr;

  if(UCOM_NULL == queue)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }

  UComOsQueue *queue_hdl = (RTOS_QUEUE_T*)queue;

  if(UCOM_NULL == queue_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }
  
  queue_hdl->handle = 0;
  queue_hdl->name = name;
  queue_hdl->messageSize = msg_size;

  mqOpenFlags = O_RDWR  | O_CREAT  | O_EXCL;
  mqOpenMode = S_IRWXU;

  mqAttr.mq_msgsize = msg_size;
  mqAttr.mq_maxmsg = msg_count;
  mqAttr.mq_curmsgs = 0;

  sprintf(mqName, "/%s", name);

  /*
   * AKOT If a queue is being reopened it still may have msg in
   * So delete any previous queue created with 'name'
   */

  mq_unlink(mqName);

  queue_hdl->handle = mq_open(mqName, mqOpenFlags, mqOpenMode, &mqAttr);

  if((mqd_t)-1 == queue_hdl->handle)
  {
    ucom_log("UComOsQueueCreate failed with error %d", errno);
    ucom_log("queue data: ");
    return UComOsConvertSysStatus(errno);
  }

  return UCOM_SUCCESS;
}

/*  -------------------------------------------------------------------------
    UComOsQueueDelete
    ------------------------------------------------------------------------- */
UComInt32 UComOsQueueDelete(UComOsQueue* queue)
{
  char mqName[255];

  if(UCOM_NULL == queue)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }

  UComOsQueue *queue_hdl = (RTOS_QUEUE_T*)queue;

  if(UCOM_NULL == queue_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }

  if(0 != mq_close(((_RTOS_QUEUE*)queue)->handle))
  {
    ucom_log("UComOsQueueDelete of queue handle %p failed with error %d", queue_hdl->handle, errno);
    ucom_log("queue data: ");
  }

  sprintf(mqName, "/%s", ((_RTOS_QUEUE*)queue)->name);

  ucom_log("\n UComOsQueueDelete deleting queue name %s\n", mqName);

  if(0 != mq_unlink(mqName))
  {
    ucom_log("\n UComOsQueueDelete of queue handle %p failed with error %d", queue_hdl->handle, errno);
    ucom_log("\n queue data: ");
    return UComOsConvertSysStatus(errno);
  }

  queue_hdl->handle   = 0;

  return UCOM_SUCCESS;
} 

/*  -------------------------------------------------------------------------
    UComOsQueueSend
    ------------------------------------------------------------------------- */
UComInt32 UComOsQueueSend(UComOsQueue* queue, const void* message, UComBool suspend)
{
  if(UCOM_NULL == queue)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
    
  UComOsQueue *queue_hdl = (RTOS_QUEUE_T*)queue;
    
  if(UCOM_NULL == queue_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }
  
  if(UCOM_NULL == message)
  {
  //ucom_log("\n UComOsQueueSend return UCOM_ERROR_INVALID_PARAM\n");
    return UCOM_ERROR_INVALID_PARAM;
  }

  if(0 != mq_send(((_RTOS_QUEUE*)queue)->handle, message, queue_hdl->messageSize, 0)
  {
  //ucom_log("\n UComOsQueueSend of queue handle %p failed with error %d", ((_RTOS_QUEUE*)queue)->handle, errno);
  //ucom_log("\n queue data: ");
    return UComOsConvertSysStatus(errno);
  }

  return UCOM_SUCCESS;
} 

/*  -------------------------------------------------------------------------
    UComOsQueueReceive
    ------------------------------------------------------------------------- */
UComInt32 UComOsQueueReceive(UComOsQueue* queue, void* message, UComBool suspend)
{
  ssize_t  msgSize;
  unsigned int msgPriority;
  struct mq_attr mqAttr;

  if(UCOM_NULL == queue)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
    
  UComOsQueue *queue_hdl = (RTOS_QUEUE_T*)queue;
    
  if(UCOM_NULL == queue_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }

  if(UCOM_NULL == message)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  if(UCOM_FALSE == suspend)
  {
    mq_getattr(((_RTOS_QUEUE*)queue)->handle, &mqAttr);
    if(mqAttr.mq_curmsgs == 0)
    {
      return UCOM_OS_QUEUE_EMPTY; //queue empty, nothing to read
    }
  }

  memset(message, 0, queue_hdl->messageSize);
  msgSize = mq_receive(queue_hdl->handle, message, queue_hdl->messageSize, &msgPriority);

  if((ssize_t)-1 == msgSize)
  {
    ucom_log("UComOsQueueReceive of queue handle %p failed with error %d", queue_hdl->handle, errno);
    ucom_log("queue data: ");
    return UComOsConvertSysStatus(errno);
  }

  return UCOM_SUCCESS;
} 




/* -----------------------------------------------------------------------------------
---------------------------------------TIMERS---------------------------------------
ucom_rtos_timer.c
--------------------------------------------------------------------------------------*/


#include "ucom_rtos_linux.h"

typedef struct RTOS_TIMER
{
  timer_t id;
  sigset_t mask;
  UComOsTimerCb cb;
  int oneShot;
  void* cb_arg;
} RTOS_TIMER_T;

#define _RTOS_TIMEOUT_SIGNAL SIGRTMIN

/**
  Get current RTC time

  @param  date
  @param  time
  @return current RTC value in UCom format

*/
void ucom_dev_get_current_rtc_time(UComDateTime * datetime)
{
  int rtc_fd = 0;
  struct rtc_time rtcTime;

  /* rtc device name should use define */
  rtc_fd = open("/dev/rtc", O_RDONLY, 0);

  ioctl(rtc_fd, RTC_RD_TIME, &rtcTime);

  datetime->day = rtcTime.tm_wday;
  datetime->month = rtcTime.tm_mon;
  datetime->year = rtcTime.tm_year;
  datetime->hour = rtcTime.tm_hour;
  datetime->minute = rtcTime.tm_min;
  datetime->second = rtcTime.tm_sec;

  close(rtc_fd);
}

UComUInt32 ucom_dev_get_current_millis(void)
{
  struct timespec ts;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
     ucom_log("fail to get CLOCK_MONOTONIC\r\n");
     UCOM_exit(errno);
  }

  return (UComUInt32)((ts.tv_sec*1000) + (ts.tv_nsec/1000000));
}


static inline void timeOutSignal_handler(int signum, siginfo_t *info, void *context __attribute__((unused)))
{
  ((_RTOS_TIMER*)info->si_value.sival_ptr)->cb ( ((_RTOS_TIMER*)info->si_value.sival_ptr)->cb_arg );
}

/*  -------------------------------------------------------------------------
    UComOsTimerCreate
    ------------------------------------------------------------------------- */
UComInt32 UComOsTimerCreate(UComOsTimer* timer,const UComChar* name, UComOsTimerCb callback_fn, void* arg)
{
  struct sigevent sev;
  struct sigaction sa;
  sigset_t mask;

  /* Establish handler for timer signal */
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = timeOutSignal_handler;
  sigemptyset(&sa.sa_mask);

  if(UCOM_NULL == timer)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }

  RTOS_TIMER_T *timer_hdl = (RTOS_TIMER_T *)timer;

  if(UCOM_NULL == timer_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }
  
  if(0 != sigaction(_RTOS_TIMEOUT_SIGNAL, &sa, NULL))
  {
    return UComOsConvertSysStatus(errno);
  }
  
  /* Block timer signal temporarily */
  sigemptyset (&((_RTOS_TIMER*)p_timer)->mask);
  sigaddset(&((_RTOS_TIMER*)p_timer)->mask, _RTOS_TIMEOUT_SIGNAL);
  
  if(0 != sigprocmask(SIG_SETMASK, &timer_hdl->mask, NULL))
  {
    ucom_log("\n UComOsTimerCreate::sigprocmask return %d", errno);
    return UComOsConvertSysStatus(errno);
  }
  
  timer_hdl->id = 0;
  timer_hdl->cb = callback_fn;
  timer_hdl->cb_arg = arg;
  timer_hdl->oneShot = 1; /* default */
  
  memset(&sev, 0, sizeof(struct sigevent));
  
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = _RTOS_TIMEOUT_SIGNAL;
  sev.sigev_value.sival_ptr = p_timer;
  
  if ( 0 != timer_create(CLOCK_REALTIME, &sev, &timer_hdl->id) )
  {
    printf("\n UComOsTimerCreate failed with error %d\n", errno);
    return UComOsConvertSysStatus(errno);
  }
 
  return UCOM_SUCCESS;
} 
 
/*  -------------------------------------------------------------------------
    UComOsTimerDelete
    ------------------------------------------------------------------------- */
UComInt32 UComOsTimerDelete(UComOsTimer* timer)
{
  if(UCOM_NULL == timer)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_TIMER_T *timer_hdl = (RTOS_TIMER_T *)timer;
  
  if(UCOM_NULL == timer_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }


  if(0 == timer_hdl->id)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  if(0 != timer_delete(timer_hdl->id))
  {
    return UComOsConvertSysStatus(errno);
  }

  timer_hdl->id = 0;

  return UCOM_SUCCESS;
} 

/*  -------------------------------------------------------------------------
    UComOsTimerStart
    ------------------------------------------------------------------------- */
UComInt32 UComOsTimerStart(UComOsTimer* timer, UComUInt32 min_timeout, UComUInt32 max_timeout)
{
  struct itimerspec its;

  if(UCOM_NULL == timer)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_TIMER_T *timer_hdl = (RTOS_TIMER_T *)timer;
  
  if(UCOM_NULL == timer_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }
  
  if(0 == timer_hdl->id)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }
  
  its.it_value.tv_sec = max_timeout / 1000;
  its.it_value.tv_nsec = (max_timeout % 1000) * 1E6;
  
  /* if ( 1 == ((_RTOS_TIMER*)p_timer)->oneShot ) */
  /* AKOT
   * we have one-shot timers by default
   */
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;
  
  /* Unblock timer signal blocked during timer create*/
  sigprocmask(SIG_UNBLOCK, &timer_hdl->mask, NULL);
  
  if(0 != timer_settime(timer_hdl->id, 0, &its, NULL))
  {
    return UComOsConvertSysStatus(errno);
  }
  
  return UCOM_SUCCESS;
}

/*  -------------------------------------------------------------------------
    UComOsTimerStop
    ------------------------------------------------------------------------- */
UComInt32 UComOsTimerStop(UComOsTimer* timer)
{
  struct itimerspec its;
  
  if(UCOM_NULL == timer)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_TIMER_T *timer_hdl = (RTOS_TIMER_T *)timer;
  
  if(UCOM_NULL == timer_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }
  
  if(0 == timer_hdl->id)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }
  
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;
  
  if(0 != timer_settime(timer_hdl->id, 0, &its, NULL))
  {
    return UComOsConvertSysStatus(errno);
  }
  
  return UCOM_SUCCESS;
}

/*  -------------------------------------------------------------------------
    UComOsTimerIsRunning
    ------------------------------------------------------------------------- */
UComInt32 UComOsTimerIsRunning(UComOsTimer* timer, UComBool* running)
{
  struct itimerspec curr_value;

  if(UCOM_NULL == timer)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  RTOS_TIMER_T *timer_hdl = (RTOS_TIMER_T *)timer;
  
  if(UCOM_NULL == timer_hdl)
  {
    return UCOM_ERROR_INVALID_HANDLE;
  }

  if(0 == timer_hdl->id)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  if(0 != timer_gettime(timer_hdl->id, &curr_value))
  {
    ucom_log("\n UComOsTimerIsRunning of timer id %d failed with error %d", ((_RTOS_TIMER*)p_timer)->id, errno);
    return UComOsConvertSysStatus(errno);
  }

  *running = (curr_value.it_value.tv_sec > 0) || (curr_value.it_value.tv_nsec > 0);

  return UCOM_SUCCESS;
}



/* -----------------------------------------------------------------------------------
---------------------------------------MEMORY---------------------------------------
ucom_rtos_memmgr.c
--------------------------------------------------------------------------------------*/

typedef struct RTOS_MEM_POOL
{
  void* pool_ptr;  //< Pointer to pool block
  unsigned long pool_size;         //< Size of the memory pool
  unsigned long pool_alloc_count;  //< Allocated item count
} RTOS_MEM_POOL_T;

typedef struct RTOS_MEMALLOC_HEADER
{
  RTOS_MEM_POOL_T *pool;
  unsigned short alloc_size;
} RTOS_MEMALLOC_HEADER_T;

/*  -------------------------------------------------------------------------
    UComOsMemCreatePool
    examle:   rc = UComOsMemCreatePool(&ibmm_dds, (void *)&ibmm_dds_pool, IBMM_NU_DATA_POOL_SIZE);

    static UComUInt64 ibmm_dds_pool[IBMM_NU_DATA_POOL_SIZE/sizeof(UComUInt64)]; // use UComUInt64 to ensure correct pool alignment
    static UComOsMem ibmm_dds;
    ------------------------------------------------------------------------- */
UComInt32 UComOsMemCreatePool(UComOsMem* mem, void* mem_area, UComUInt32 size)
{
  int ret;

  if(UCOM_NULL == mem)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  if(UCOM_NULL == mem_area || 0 == size)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }
  
  RTOS_MEM_POOL_T *mem_hdl = (RTOS_MEM_POOL_T *)mem;
  
  if(mem_hdl->pool_ptr)
  {
    return UCOM_FAILURE;
  }

  mem_hdl->pool_ptr = (void*)UCOM_TRUE; //Set pool as allocated
  mem_hdl->pool_size = size;
  mem_hdl->pool_alloc_count = 0;

  return UCOM_SUCCESS; //success  
} 

/*  -------------------------------------------------------------------------
    UComOsMemAlloc
    ------------------------------------------------------------------------- */
UComInt32 UComOsMemAlloc(UComOsMem* mem, UComUInt32 size, void** ptr)
{
  RTOS_MEMALLOC_HEADER_T *headPtr = UCOM_NULL;

  if(UCOM_NULL == mem)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }

  RTOS_MEM_POOL_T *mem_hdl = (RTOS_MEM_POOL_T *)mem;
  
  if(mem_hdl->pool_ptr)
  {
    return UCOM_FAILURE;
  }

  if(UCOM_NULL == ptr)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  *ptr = 0;
  
  if(!((_RTOS_MEM_POOL*)mem)->pool_ptr )
  {
    //ucom_log("UComOsMemAlloc: !!!ERROR!!! Pool not initialized\n" );
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
  
  if((mem_hdl->pool_alloc_count + size) > mem_hdl->pool_size )
  {
    //ucom_log("\t SysMemAlloc(@0x%x): @0x%x, %d bytes :: !!!OUT OF MEMORY!!!\n", mem, *ptr, size);
    return UCOM_ERROR_OUT_OF_MEMORY;
  }
  
  headPtr = (RTOS_MEMALLOC_HEADER_T *) malloc(sizeof(RTOS_MEMALLOC_HEADER_T) + size);
  
  if(UCOM_NULL == headPtr)
  {
    //ucom_log("\t SysMemAlloc(@0x%x): @0x%x, %d bytes :: !!!OUT OF MEMORY (Linux)!!!\n", mem, *ptr, size);
    return UCOM_ERROR_OUT_OF_MEMORY;
  }
  
  //copy pool data in header
  headPtr->pool = mem_hdl;
  
  //copy alloc size in header
  headPtr->alloc_size = size;
  
  //update alloc count
  mem_hdl->pool_alloc_count += size;
  
  //set pointer for caller
  *ptr = (unsigned char*)headPtr + sizeof(RTOS_MEMALLOC_HEADER_T);
  
  return UCOM_SUCCESS; //success  
} 

/*  -------------------------------------------------------------------------
    UComOsMemFree
    ------------------------------------------------------------------------- */
UComInt32 UComOsMemFree(void* ptr)
{
  RTOS_MEMALLOC_HEADER_T *headPtr;

  if(UCOM_NULL == ptr)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  //get real block
  headPtr = (RTOS_MEMALLOC_HEADER_T *)((unsigned char*)ptr - sizeof(RTOS_MEMALLOC_HEADER_T));

  headPtr->pool->pool_alloc_count -= headPtr->alloc_size; //Update allocated size

  free(headPtr);

  return UCOM_SUCCESS;
}

/*  -------------------------------------------------------------------------
    UComOsMemDeletePool
    ------------------------------------------------------------------------- */
UComInt32 UComOsMemDeletePool(UComOsMem* mem)
{
  if(UCOM_NULL == mem)
  {
    return UCOM_OS_INVALID_CONTROL_STRUCTURE;
  }
    
  RTOS_MEM_POOL_T *mem_hdl = (RTOS_MEM_POOL_T *)mem;
    
  if(mem_hdl->pool_ptr)
  {
    return UCOM_FAILURE;
  }

  if(!mem_hdl->pool_ptr)
  {
    return UCOM_ERROR_INVALID_PARAM;
  }

  //Set pool as deleted
  mem_hdl->pool_ptr = (void*)0;
  
  if(mem_hdl->pool_alloc_count )
  {
    //ucom_log("\t SysMemDeletePool(): !!!WARNING!!! still allocated %d bytes\n", ((_RTOS_MEM_POOL*)mem)->pool_alloc_count);
    //return UCOM_ERROR_INVALID_PARAM;
  }

  return UCOM_SUCCESS; //success
} // UComOsMemDeletePool





