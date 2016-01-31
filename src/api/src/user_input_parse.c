#include <stdio.h>
#include <string.h>
#include <rtos.h>
#include <errno.h>

#include "user_input_parse.h"
#include "common_include.h"
#include "calendar_manager.h"
#include "calendar_database.h"
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

  if(recv_buffer == NULL || strcmp(recv_buffer, "") == 0)
    strncpy(recv_buffer, "No answer found.", 17);

  printf("\nResult:  %s\n\n\n", recv_buffer);

 // QueueDelete(&mq, QUEUE_NAME);
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

static Bool is_valid_time(char8_t *word, IN OUT float32_t *start_time, IN OUT float32_t *stop_time)
{
  char8_t *ptr_word = word;
  float32_t float_digit1 = 0, float_digit2 = 0;
  uint32_t i = 0;

  if(NULL == word  || NULL == start_time || NULL == stop_time)
  {
    CALENDER_DEBUG("Invalid parameters format.");
    return FAILURE;
  }

  while(*ptr_word == ' ')
    ptr_word ++;

  /*parsing start time. */
  for(i = 0; i < 2; i++)
  {
    if(*ptr_word != ':')
    {
      if(*ptr_word - '0' < 0 && *ptr_word - '0' > 9)
      {
        CALENDER_DEBUG("Errror entry1: time range should be in 0-9");
        return FAILURE;
      }

      if(i==0)
      {
        *start_time = (float32_t)(*ptr_word - '0');
      }
      else
      {
        *start_time = *start_time*10 + (float32_t)(*ptr_word - '0');
      }
      ptr_word++;
    }
  }

  if(*ptr_word!= ':')
  {
    CALENDER_DEBUG("Error entry 1: no ':' append after time found.");
    return FAILURE;
  }

  float_digit1 = (float)(*(ptr_word+1)-'0')/10;
  float_digit2 = (float)(*(ptr_word+2)-'0')/100;
  *start_time = *start_time + float_digit1  + float_digit2;

  CALENDER_DEBUG("Get start time %.2f.", *start_time);

  if(*(ptr_word + 3) != '-' && *(ptr_word + 4 )!= '-')
  {
    return SUCCESS;
  }

  while(ptr_word != '\0')
  {
    if(*ptr_word == '-')
    {
      ptr_word++;
      for(i =0; i < 2; i++)
      {
        if(*ptr_word != ':')
        {
          if(*ptr_word - '0' < 0 && *ptr_word - '0' > 9)
          {
            CALENDER_DEBUG("Error entry 2: time range should be in 0-9");
            return FAILURE;
          }

          if(i==0)
          {
            *stop_time = (uint32_t)(*ptr_word - '0');
          }
          else
          {
            *stop_time = *stop_time*10 + (uint32_t)(*ptr_word - '0');
          }
          ptr_word++;
        }
      }

      if(*ptr_word != ':')
      {
        CALENDER_DEBUG("Error entry 2: no ':' append after found. %c", *ptr_word);
        return FAILURE;
      }
      else
      {
        float_digit1 = (float)(*(ptr_word+1)-'0')/10;
        float_digit2 = (float)(*(ptr_word+2)-'0')/100;
        *stop_time = *stop_time + float_digit1 + float_digit2;
        CALENDER_DEBUG("Get stop time %.2f.", *stop_time);
        return SUCCESS;
      }
    }
    ptr_word++;
  }
  return SUCCESS;
}

inline Bool is_valid_time_test_wrapper(char8_t *word, float32_t *start_time, float32_t *stop_time)
{
  return is_valid_time(word, start_time, stop_time);
}


static uint32_t is_weekday(char *day)
{
  uint32_t ret = 0;

  if(NULL == day)
  {
    return -1;
  }

  if(strcmp(day, "Monday") == 0 || strcmp(day, "monday") == 0  || strcmp(day, "MONDAY") == 0)
  {
    ret = MONDAY_IDX;
  }
  else if(strcmp(day, "Tuesday") == 0 || strcmp(day, "tuesday") == 0 || strcmp(day, "TUESDAY") == 0)
  {
    ret = TUESDAY_IDX;
  }
  else if(strcmp(day, "Wednesday") == 0 || strcmp(day, "wednesday") == 0 || strcmp(day, "WEDNESDAY") == 0)
  {
    ret = WEDNESDAY_IDX;
  }
  else if(strcmp(day, "Thursday") == 0 || strcmp(day, "thursday") == 0 || strcmp(day, "THURSDAY") == 0)
  {
    ret = THUESDAY_IDX;
  }
  else if(strcmp(day, "Friday") == 0 || strcmp(day, "friday") == 0 || strcmp(day, "FRIDAY") == 0)
  {
    ret = FRIDAY_IDX;
  }
  else if(strcmp(day, "Saturday") == 0 || strcmp(day, "saturday") == 0 || strcmp(day, "SATURDAY") == 0)
  {
    ret = SATURDAY_IDX;
  }
  else if(strcmp(day, "Sunday") == 0 || strcmp(day, "sunday") == 0 || strcmp(day, "SUNDAY") == 0)
  {
    ret = SUNDAY_IDX;
  }
  else
  {
    CALENDER_DEBUG("Invalid weekday time format:%d", ret);
    ret = -1;
  }

  CALENDER_DEBUG("Return weekday :%d", ret);

  return ret;
}

inline uint32_t is_weekday_test_wrapper(char *day)
{
  return is_weekday(day);
}

/* check if each line is in proper format such as DAY HOUR EVENT */
static Bool check_line_format(char8_t *line)
{
  char8_t *ptr = line;
  uint32_t space_num = 0;

  while(*ptr != '\0')
  {
    if(*ptr == ' ')
    {
      space_num ++;
    }
    ptr++;
  }

  if(space_num < 2)
  {
    return FAILURE;
  }

  return SUCCESS;
}

inline Bool check_line_format_test_wrapper(char8_t *line)
{
  return check_line_format(line);
}

static Bool line_parse(IN char8_t *line)
{
   char8_t *ptr_line = NULL;
   char8_t word[MAX_LINE_SIZE]="";
   char8_t *ptr_word = word;
   uint32_t space_num = 0;
   uint32_t weekday;
   float32_t start_time = 0;
   float32_t stop_time = 0;
   
   if(NULL == line)
   {
     CALENDER_DEBUG("Error: String line is NULL.");
     return FAILURE;
   }

   if(strlen(line) > MAX_LINE_SIZE)
   {
     CALENDER_DEBUG("Error: Line size exceed maxmium limit, drop line %s", line);
     return FAILURE;
   }

   if(FAILURE == check_line_format(line))
   {
      CALENDER_DEBUG("Error: Line format error, do not have 2 space in the line and drop line %s", line);
      return FAILURE;
   }

   ptr_line = line;

   while(*ptr_line == ' ' || *ptr_line == '\n')
    ptr_line++;

   while(*ptr_line!='\0')
   {
     if(*ptr_line == ' ' && space_num <= 2)
     {
       *(ptr_word) = '\0';
       space_num++;

       if(space_num == 1)
       {
         weekday = is_weekday(word);
         if(-1 == weekday)
         {
           CALENDER_DEBUG("Error: word format error, first word is not weekday and drop word %s", word);
           return FAILURE;
         }
       }
       else if(space_num == 2)
       {
         if(FAILURE == is_valid_time(word, &start_time, &stop_time))
         {
           CALENDER_DEBUG("Error: word format error, second word is not time format and drop word %s", word);
           return FAILURE;          
         }
         space_num++;
         ptr_line++;
       }
       ptr_word = word;
     }
     *ptr_word++ = *ptr_line++;
   }

   *ptr_word = '\0';

    CALENDER_DEBUG("Add a new event to database: database[%d]: start_time: %.2f  stop_time: %.2f event: %s ", weekday, start_time, stop_time, word);

    if(FAILURE == calendar_data_base_event_add(weekday, start_time, stop_time, word))
    {
      CALENDER_DEBUG("Failed to add a new event to database");
      return FAILURE;       
    }
    
    return SUCCESS;
   
}


inline Bool line_parse_test_wrapper(IN char8_t *line)
{
  return line_parse(line);
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

  CALENDER_DEBUG("file size is %d\n", size);

  char line[256];

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
        CALENDER_DEBUG("%s\n", line);
        if(FAILURE == line_parse(line))
        {
           CALENDER_DEBUG("Failed to parse line [%s].", line);
        }
     }

     *pline++ = *ptr++;
  }

  FsClose(fd);
}









