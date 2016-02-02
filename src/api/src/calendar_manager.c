/* Calendar_manager.c
   Calendar manager is the thread which process all input message, it listens the requests from  user input thread, apply
   the pattern match methods and generate proper answer for the user input process thread.

Architecture:         thread                         thread 
                    /             \              /                \      Create
                   |               |             |                 |     Maintain  __________
          query    |               |     query   |                 |     lookup    |         |
   input  ->       |  input        |     ->      |   calendar      |     ->        |calendar |
                   |  process      |             |   manager       |               |database |
          <-       |  thread       |     <-      |   thread        |               |_________|
            answer |               |     answer  |                 |
                   \               /              \                /

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "common_include.h"
#include "calendar_manager.h"
#include "rtos.h"
#include "types.h"
#include "helper_func.h"

/**************************************** Global variable and data definition ******************************/

static char8_t weekday_query_pattern[7][10] = {"monday",  "tuesday", "wednesday", "thursday", "friday", "saturday","sunday"};

static char8_t motivation_pattern[3][10]  = {"what", "do", "want"};

static char8_t weekday_answer_pattern[2][10] = {"which", "which day"};

static char8_t time_schedule_pattern[2][10] = {"when", "what time"};

static char8_t time_available_pattern[2][10] = {"available", "free"};

static char8_t time_occupy_pattern[2][10] = {"busy", "occupy"};

extern int32_t calendar_exit;

extern mutex_hdl calendar_lock;

static char8_t input_buffer[MAX_MSG_QUEUE_SIZE + 1];
static char8_t answer_buffer[MAX_MSG_QUEUE_SIZE +1];


/****************************************** Local function declearation ***************************************/
static uint32_t check_weekday_pattern(char8_t *message);

static uint32_t check_motivation_pattern(char8_t *message);

static uint32_t check_time_schedule_pattern(char8_t *message);

static uint32_t check_time_available_pattern(char8_t *message);

static uint32_t check_time_occupy_pattern(char8_t *message);

static uint32_t check_daylight_pattern(char8_t *message);

static Bool check_weekday_answer_pattern(char8_t *message);

static void process_input_string(IN char8_t *message, OUT char8_t *answer);

static void *calendar_manager_thread_entry(void *param);

/****************************************** local function definition ***************************************/

static uint32_t check_weekday_pattern(char8_t *message)
{
  uint32_t i = 0;
  uint32_t weekday = -1;

  if(NULL == message)
  {
    return -1;
  }

  for(i = 0; i < 7; i++)
  {
    if(strstr(message, weekday_query_pattern[i]) != NULL)
    {
      weekday = i;
      CALENDER_DEBUG("Success to find key word weekday [%d] from : [ %s ].", weekday, message);
      return weekday;
     }
  }
  return -1;
}

static uint32_t check_motivation_pattern(char8_t *message)
{
  int32_t i = 0;

  if(NULL == message)
  {
    return FAILURE;
  }

  for(i = 0; i < 3; i++)
  {
    if(strstr(message, motivation_pattern[i]) != NULL)
    {
        CALENDER_DEBUG("Success to find key word motivation [ %s ] from : [ %s ].", motivation_pattern[i], message);
        return SUCCESS;
    }
  }

  return FAILURE;
}

static uint32_t check_time_schedule_pattern(char8_t *message)
{
  int32_t i = 0;

  if(NULL == message)
  {
    return FAILURE;
  }

  for(i = 0; i < 2; i++)
  {
    if(strstr(message, time_schedule_pattern[i]) != NULL)
    {
        CALENDER_DEBUG("Success to find key word time_schedule_pattern [ %s ] from : [ %s ].", time_schedule_pattern[i], message);
        return SUCCESS;
    }
  }

  return FAILURE;
}


static uint32_t check_time_available_pattern(char8_t *message)
{
  int32_t i = 0;

  if(NULL == message)
  {
    return FAILURE;
  }

  for(i = 0; i < 2; i++)
  {
    if(strstr(message, time_available_pattern[i]) != NULL)
    {
        CALENDER_DEBUG("Success to find key word time_available_pattern [ %s ] from : [ %s ].", time_available_pattern[i], message);
        return SUCCESS;
    }
  }

  return FAILURE;
}

static uint32_t check_time_occupy_pattern(char8_t *message)
{
  int32_t i = 0;

  if(NULL == message)
  {
    return FAILURE;
  }

  for(i = 0; i < 2; i++)
  {
    if(strstr(message, time_occupy_pattern[i]) != NULL)
    {
        CALENDER_DEBUG("Success to find key word time_occupy_pattern [ %s ] from : [ %s ].", time_occupy_pattern[i], message);
        return SUCCESS;
    }
  }

  return FAILURE;
}

static uint32_t check_daylight_pattern(char8_t *message)
{
  uint32_t daylight_range = WHOLE_DAY;

  if(NULL == message)
  {
    return -1;
  }

  if(strstr(message, "morning") != NULL && strstr(message, "afternoon") == NULL && strstr(message, "night") == NULL)
  {
    daylight_range = MORNING_ONLY;
  }
  else if(strstr(message, "morning") == NULL && strstr(message, "afternoon") != NULL && strstr(message, "night") == NULL)
  {
    daylight_range = AFTERNOON_ONLY;
  }
  else if(strstr(message, "morning") == NULL && strstr(message, "afternoon") == NULL && strstr(message, "night") != NULL)
  {
    daylight_range = NIGHT_ONLY;
  }
  else
  {
    daylight_range = WHOLE_DAY;
  }

  CALENDER_DEBUG("daylight pattern is found as: %d", daylight_range);
  return daylight_range;
}

static Bool check_weekday_answer_pattern(char8_t *message)
{
  int32_t i = 0;

  if(NULL == message)
  {
    return FAILURE;
  }

  for(i = 0; i < 2; i++)
  {
    if(strstr(message, weekday_answer_pattern[i]) != NULL)
    {
      CALENDER_DEBUG("Success to find key word to replay a answer with weekday [ %s ] from : [ %s ].", weekday_answer_pattern[i], message);
      return SUCCESS;
    }
  }

  return FAILURE;
}

static void process_input_string(IN char8_t *message, OUT char8_t *answer)
{
  int32_t daylight_range = WHOLE_DAY;
  int32_t event_match = 0;
  int32_t avail = AVAIL_DEFAULT;
  int32_t weekday_query_pattern_presents = 0;
  int32_t motivation_pattern_presents = 0;
  int32_t weekday_answer_pattern_presents = 0;
  int32_t check_time_schedule_pattern_presents = 0;

  if(NULL == message || NULL == answer)
  {
    CALENDER_DEBUG("Invalid parameters.");
    return;
  }

  convert_message_to_lower_case(message);

  CALENDER_DEBUG("Converted message in lower case: %s.", message);

  weekday_query_pattern_presents = check_weekday_pattern(message);

  motivation_pattern_presents = check_motivation_pattern(message);

  daylight_range = check_daylight_pattern(message);

  weekday_answer_pattern_presents = check_weekday_answer_pattern(message);

  check_time_schedule_pattern_presents = check_time_schedule_pattern(message);

  if(SUCCESS == check_time_available_pattern(message))
  {
    avail = AVAIL_FREE;
  }

  if(SUCCESS == check_time_occupy_pattern(message))
  {
    avail = AVAIL_BUSY;
  }

  if((weekday_query_pattern_presents != -1) && (motivation_pattern_presents == SUCCESS) && (weekday_answer_pattern_presents == FAILURE))
  {
    /* In this case return the all the event queried by weekday */
    CALENDER_DEBUG("-------------Entry answer_with_event_by_weekday-----------");
    event_match = calendar_database_retrun_event_by_weekday(answer, weekday_query_pattern_presents, daylight_range);
  }
  else if((weekday_answer_pattern_presents == SUCCESS) && (motivation_pattern_presents == FAILURE) && (weekday_query_pattern_presents == -1) && (avail == AVAIL_DEFAULT))
  {
    /*In this case return all the time schedule according to the event matched with input string */
    CALENDER_DEBUG("-------------Entry event_pattern_match_calendar_weekday-----------");
    event_match = calendar_database_return_weekday_time_by_event(message, answer, daylight_range);
  }
  else if((weekday_answer_pattern_presents == SUCCESS) && (avail != AVAIL_DEFAULT))
  {
    CALENDER_DEBUG("-------------Entry event_pattern_match_calendar_weekday_avail-----------");
    event_match = calendar_database_return_avail_by_weekday(message, answer, daylight_range, avail);
  }
  else if((check_time_schedule_pattern_presents == SUCCESS) && (weekday_query_pattern_presents == -1))
  {
    CALENDER_DEBUG("-------------Entry event_pattern_match_calendar_time-----------");
    event_match = calendar_database_return_weekday_schedule_by_event(message, answer, daylight_range);
  }
  
  CALENDER_DEBUG("Prepared answer message [%s] back to user.", answer);
}


static void *calendar_manager_thread_entry(void *param)
{
  UNUSED(param);
  mqd_hdl mq;

  memset(input_buffer, '\0', MAX_MSG_QUEUE_SIZE+1);
  memset(answer_buffer, '\0', MAX_MSG_QUEUE_SIZE+1);

  if(FAILURE == QueueCreate(&mq, QUEUE_NAME, MAX_MSG_QUEUE_SIZE, MAX_MSG_QUEUE_NUM))
  {
    calendar_quit();
  }

  while(!calendar_exit)
  {
    if(FAILURE == QueueReceive(&mq, input_buffer, MODE_BLOCK))
    {
      calendar_quit();
    }

    CALENDER_DEBUG("Success to receive message : [ %s ] from user input process thread.", input_buffer);

    process_input_string(input_buffer, answer_buffer);

    if(FAILURE == QueueSend(&mq, answer_buffer, MODE_BLOCK))
    {
      calendar_quit();
    }

    CALENDER_DEBUG("Success to Answer message to user input process thread.");
    memset(answer_buffer, '\0', strlen(answer_buffer));
  }

  QueueDelete(&mq, QUEUE_NAME);
}


/****************************************** export function definition ***************************************/


void calendar_manager_thread_init(void)
{
  thread_hdl thread_id;
  Bool ret = FAILURE;

  ret = ThreadCreate( &thread_id,
                      THREAD_CALENDAR_MANAGER_NAME,
                      THREAD_DEFAULT_PRIORITY,
                      NULL,
                      THREAD_DEFAULT_STACK_SIZE,
                      calendar_manager_thread_entry,
                      (void *)(NULL) );

  if(FAILURE == ret)
  {
    CALENDER_DEBUG("Failed to create thread.");
    calendar_quit();
  }

  if(FAILURE == ThreadDetach(&thread_id))
  {
    CALENDER_DEBUG("Failed to detach thread.");
    calendar_quit();
  }
}

