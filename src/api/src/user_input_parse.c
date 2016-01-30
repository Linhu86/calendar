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
  char8_t send_buffer[MAX_MSG_QUEUE_SIZE];
  char8_t recv_buffer[MAX_MSG_QUEUE_SIZE];

  memset(send_buffer, '\0', MAX_MSG_QUEUE_SIZE);
  memset(recv_buffer, '\0', MAX_MSG_QUEUE_SIZE);

  strncpy(send_buffer, msg, strlen(msg));

  if(FAILURE == QueueCreate(&mq, QUEUE_NAME, MAX_MSG_QUEUE_SIZE, MAX_MSG_QUEUE_NUM))
  {
    calendar_quit();
  }

  if(FAILURE == QueueSend(&mq, send_buffer, MODE_BLOCK))
  {
    CALENDER_DEBUG("Failed to dispatch message %s to calendar manager, error: %s.", send_buffer, strerror(errno));
    calendar_quit();
  }
  else
  {
    CALENDER_DEBUG("Succeed to dispatch message %s to calendar manager.", send_buffer);
  }

  /* Block until receiving message from calendar manager or error occurs. */
  if(FAILURE == QueueReceive(&mq, recv_buffer, MODE_BLOCK))
  {
    CALENDER_DEBUG("Failed to receive answer from calendar manager, error: %s.", strerror(errno));
  }
  CALENDER_DEBUG("Succeed to receive message [ %s ] from calendar manager.", recv_buffer);

  QueueDelete(&mq, QUEUE_NAME);
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
        printf("Error: Not a valid choice.\n");
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

void parse_file(void)
{
  file_hdl fd;
  int32_t size;

  if(FAILURE == FsOpen(&fd, CALENDAR_FILE_NAME, OPEN_MODE_READ_ONLY))
  {
    calendar_quit();
  }

  size =  FsGetSize(fd);

  printf("file size is %d\n", size);

  char line[128];

  char buffer[size];
  memset(buffer, '\0', size);

  if(FAILURE == FsRead(fd, buffer, size))
  {
      calendar_quit();
  }

  char *ptr = buffer;
  char *pline = line;
  int i = 0;

  for(i = 0; i < size; i++)
  {
     if(*ptr == '\n')
     {
        *pline = '\0';
        pline = line;
        printf("%s\n", line);
     }

     *pline++ = *ptr++;
  }

  FsClose(fd);
}









