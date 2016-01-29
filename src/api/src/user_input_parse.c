#include <stdio.h>
#include <string.h>
#include <rtos.h>
#include <errno.h>

#include "user_input_parse.h"
#include "common_include.h"
#include "calendar_manager.h"
#include "rtos.h"


extern int32_t calendar_exit;

static char8_t input_buffer[MAX_BUFFER_SIZE] = {'\0'};

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

void user_input_thread_init(void)
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











