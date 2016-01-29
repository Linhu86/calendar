#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <errno.h>
#include <time.h>

#include "calendar_main.h"

#define THREAD_USER_INPUT_PROCESS_NAME "thread_user_input_proc"

#define THREAD_CALENDAR_MANAGER_NAME "thread_calendar_manager_proc"

typedef enum{
  CALENDAR_RUNNING = 0,
  CALENDAR_EXIT = 1
} CALENDAR_STATUS;

static int32_t calendar_exit = CALENDAR_RUNNING;

static char8_t input_buffer[MAX_BUFFER_SIZE] = {'\0'};

static void calendar_quit(void)
{
  /* should implement lock to aviod race condition.*/
  calendar_exit = CALENDAR_EXIT;
}

static void clear_buffer(void)
{
  char8_t ch;
  ch = getchar();
  while(ch != '\n')
  {
    ch = getchar();
  }
}

static int32_t dispatch_msg_to_calendar_mgr(char8_t *msg)
{
  mqd_t mq;
  ssize_t bytes_write;
  char8_t buffer[MAX_MSG_QUEUE_SIZE];

  memset(buffer, '\0', MAX_MSG_QUEUE_SIZE);

  strncpy(buffer, msg, strlen(msg));

  /* open the mail queue */
  mq = mq_open(QUEUE_NAME, O_WRONLY);
  if(-1  == mq)
  {
    CALENDER_DEBUG("Failed to create message queue, error: %s.", strerror(errno));
    return -1;
  }
  else
  {
    CALENDER_DEBUG("Succeed to create message queue.");
  }

  CALENDER_DEBUG("Send to calender manager thread.");

  /* send the message */
  bytes_write = mq_send(mq, buffer, MAX_MSG_QUEUE_SIZE, 0);

  if(-1 == bytes_write)
  {
    CALENDER_DEBUG("Failed to dispatch message %s to calendar manager., error: %s.", buffer, strerror(errno));
    return -1;
  }
  else
  {
    CALENDER_DEBUG("Succeed to dispatch message to calendar manager.");
  }
}


static void *user_input_process_thread_entry(void *param)
{
  char8_t ch = 0;
  int32_t char_count = 0;
  int32_t valid_choice = 0;

  UNUSED(param);
    
  while(!calendar_exit)
  {
    printf("\nPlease enter a line of text (Max %d characters)\n", MAX_BUFFER_SIZE);
    char_count = 0;
    
    while(ch != '\n' && char_count <= MAX_BUFFER_SIZE)
    {
      ch = getchar();
      input_buffer[char_count ++] = ch;
    }
    
    input_buffer[--char_count] = '\0';
    printf("\nThe text you entered was: \n");
    printf("%s\n", input_buffer);

    dispatch_msg_to_calendar_mgr(input_buffer);
    
    valid_choice = 0;
    while(valid_choice == 0)
    {
      printf("Continue (Y/N)?\n");
      scanf("%c", &ch);
      ch = toupper(ch);
      if(ch == 'Y' || ch == 'N')
      {
        valid_choice = 1;
        if(ch == 'N') calendar_quit();
      }
      else
      {
        printf("Error: Invalide choice.\n");
      }
      clear_buffer();
    }
  }
}

static mqd_t msgqueue_init(void)
{
  mqd_t mq;
  struct mq_attr attr;
    
  /* initialize the queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_MSG_QUEUE_SIZE;
  attr.mq_curmsgs = 0;

  /* create the message queue */
  mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);

  if(-1  == mq)
  {
    CALENDER_DEBUG("Failed to create message queue, error: %s.", strerror(errno));
  }
  else
  {
    CALENDER_DEBUG("Succeed to create message queue.");
  }
   
  return 0;
}



static void *calendar_manager_thread_entry(void *param)
{
  UNUSED(param);
  char8_t buffer[MAX_MSG_QUEUE_SIZE + 1];
  mqd_t mq;
  struct mq_attr attr;
   
  /* initialize the queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_MSG_QUEUE_SIZE;
  attr.mq_curmsgs = 0;
   
  /* create the message queue */
  mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
   
  if(-1  == mq)
  {
    CALENDER_DEBUG("Failed to create message queue, error: %s.", strerror(errno));
  }
  else
  {
    CALENDER_DEBUG("Succeed to create message queue.");
  }
  
  while(!calendar_exit)
  {
    ssize_t bytes_read;

    /* receive the message and blocking until user input process thread send some event. */
    bytes_read = mq_receive(mq, buffer, MAX_MSG_QUEUE_SIZE, NULL);

    if(-1 == bytes_read)
    {
      CALENDER_DEBUG("Failed to receive data from user input process, error: %s.", strerror(errno));
    }
    else
    {
      CALENDER_DEBUG("Received: %s", buffer);
      buffer[bytes_read] = '\0';
    }

  }

  mq_close(mq);
  mq_unlink(QUEUE_NAME);
}

static int32_t user_input_thread_init(void)
{
  pthread_t thread_id;
  Bool ret;

  ret = ThreadCreate( &thread_id,
                      THREAD_USER_INPUT_PROCESS_NAME,
                      THREAD_DEFAULT_PRIORITY,
                      NULL,
                      THREAD_DEFAULT_STACK_SIZE,
                      user_input_process_thread_entry,
                      (void *)(NULL) );

  if(FAILURE == ret)
  {
    calendar_quit();
  }

  if(FAILURE == ThreadDetach(&thread_id))
  {
    calendar_quit();
  }
}

static int32_t calendar_manager_thread_init(void)
{
  pthread_t thread_id;
  Bool ret;

  ret = ThreadCreate( &thread_id,
                      THREAD_CALENDAR_MANAGER_NAME,
                      THREAD_DEFAULT_PRIORITY,
                      NULL,
                      THREAD_DEFAULT_STACK_SIZE,
                      calendar_manager_thread_entry,
                      (void *)(NULL) );

  if(FAILURE == ret)
  {
    calendar_quit();
  }

  if(FAILURE == ThreadDetach(&thread_id))
  {
    calendar_quit();
  }

}

static void calendar_app_deinit(void)
{

}

static int32_t calendar_app_init(void)
{
  calendar_manager_thread_init();
  user_input_thread_init();
}

int32_t main()
{
  calendar_app_init();

  while(!calendar_exit)
  {
  
  }

  calendar_app_deinit();
}

