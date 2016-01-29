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

static void dispatch_msg_to_calendar_mgr(char8_t *msg)
{
  mqd_t mq;
  ssize_t bytes_write;
  char8_t buffer[MAX_MSG_QUEUE_SIZE];

  memset(buffer, '\0', MAX_MSG_QUEUE_SIZE);

  strncpy(buffer, msg, strlen(msg));

  if(FAILURE == QueueCreate(&mq, QUEUE_NAME, MAX_MSG_QUEUE_SIZE, MAX_MSG_QUEUE_NUM))
  {
    calendar_quit();
  }

  if(FAILURE == QueueSend(&mq, buffer, MODE_BLOCK))
  {
    CALENDER_DEBUG("Failed to dispatch message %s to calendar manager., error: %s.", buffer, strerror(errno));
    calendar_quit();
  }
  else
  {
    CALENDER_DEBUG("Succeed to dispatch message %s to calendar manager.", buffer);
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

static void *calendar_manager_thread_entry(void *param)
{
  UNUSED(param);
  char8_t buffer[MAX_MSG_QUEUE_SIZE + 1];
  mqd_hdl mq;

  memset(buffer, '\0', MAX_MSG_QUEUE_SIZE+1);

  if(FAILURE == QueueCreate(&mq, QUEUE_NAME, MAX_MSG_QUEUE_SIZE, MAX_MSG_QUEUE_NUM))
  {
    calendar_quit();
  }

  while(!calendar_exit)
  {
    if(FAILURE == QueueReceive(&mq, buffer, MODE_BLOCK))
    {
      calendar_quit();
    }

    CALENDER_DEBUG("Success to receive message : %s from user input process thread.", buffer);
  }

  QueueDelete(&mq, QUEUE_NAME);
}

static int32_t user_input_thread_init(void)
{
  thread_hdl thread_id;
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
  thread_hdl thread_id;
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

