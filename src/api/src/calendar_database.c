/* calendar_database.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calendar_database.h"
#include "common_include.h"
#include "rtos.h"

/* calendar database

calendar_database

Every day owns an event bidirection list sorted by start time.

[ Monday ]      -> [event 1] <-> [event 2] <->...<-> [event n] ->NULL 

[ Tuesday ]     -> [event 1] <-> [event 2] <->...<-> [event n] ->NULL

[ Wednesday ]   -> [event 1] <-> [event 2] <->...<-> [event n] ->NULL

      .
      .
      .
[ Sunday ]       -> [event 1] -> [event 2] ->...-> [event n] ->NULL

*/

/**************************************** Global variable and data definition ******************************/
typedef struct dayinfo
{
  Bool is_free_morning;
  Bool is_free_afternoon;
  struct event day_info_event;
} dayinfo_t;

dayinfo_t calendar_database[WEEKDAY];

/**************************************** Local variable and data definition ******************************/

static char8_t event_tmp[EVENT_NAME_LEN];

static char8_t tmp[MAX_MSG_QUEUE_SIZE+1];


/****************************************** Local function declearation ***************************************/
static Bool event_search_from_db_by_time(IN uint32_t weekday, IN float32_t start_time, OUT char8_t *event_name);

static Bool time_search_from_db_by_event(IN uint32_t weekday,  IN char8_t *event_name, OUT float32_t *start_time);

static void event_insert_into_db(IN uint32_t weekday, IN event_t *new_event);

static Bool retrieve_time_by_dailyrange(IN int32_t range,IN OUT float32_t *start_time, float32_t *stop_time);

static Bool event_pattern_match(char8_t *message, char8_t *event_name);

/**************************************** Global variable and data definition ******************************/


static Bool event_search_from_db_by_time(IN uint32_t weekday, IN float32_t start_time, OUT char8_t *event_name)
{
  if(weekday >WEEKDAY_LAST || NULL == event_name)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  event_t *ptr = calendar_database[weekday].day_info_event.next;

  while(ptr)
  {
    if(FLOAT_COMP(ptr->start_time, start_time))
    {
        strncpy(event_name, ptr->event_name, strlen(ptr->event_name));
        CALENDER_DEBUG("Found event [%s] by start time %.2f into in weekday %d.", ptr->event_name, start_time, weekday);
        return SUCCESS;
    }
    ptr = ptr->next;
  }

  CALENDER_DEBUG("Failed to find event [%s] by start time %.2f in weekday %d.", ptr->event_name, start_time, weekday);
  return FAILURE;
}

static Bool time_search_from_db_by_event(IN uint32_t weekday,  IN char8_t *event_name, OUT float32_t *start_time)
{
  if(weekday >WEEKDAY_LAST || NULL == event_name || NULL == start_time)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  event_t *ptr = calendar_database[weekday].day_info_event.next;

  while(ptr)
  {
    if(0 == strcmp(event_name, ptr->event_name))
    {
        *start_time = ptr->start_time;
        CALENDER_DEBUG("Found start time %.2f by event [%s] in weekday %d.", *start_time, ptr->event_name, weekday);
        return SUCCESS;
    }
    ptr = ptr->next;
  }

  CALENDER_DEBUG("Failed to find start time %.2f by event [%s] in weekday %d.", *start_time, ptr->event_name, weekday);
  return FAILURE;
}

static void event_insert_into_db(IN uint32_t weekday, IN event_t *new_event)
{
  event_t *ptr = &calendar_database[weekday].day_info_event;
  uint32_t pos = 1;

  if(weekday >WEEKDAY_LAST || NULL == new_event)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return;
  }

  if(new_event->start_time < 12 ||(new_event->stop_time != 0 && new_event->stop_time < 12))
  {
    calendar_database[weekday].is_free_morning = 0;
  }

  if(new_event->start_time > 12 || new_event->stop_time > 12)
  {
    calendar_database[weekday].is_free_afternoon = 0;
  }

  if(!ptr->next)
  {
    ptr->next = new_event;
    new_event ->next = NULL;
    new_event->prev = ptr;
    CALENDER_DEBUG("Insert new event %s into first pos %d weekday %d.", new_event->event_name, pos, weekday);
  }
  else
  {
    while(ptr->next)
    {
      if(new_event->start_time < ptr->start_time)
      {
        new_event->next = ptr;
        new_event->prev = ptr->prev;
        ptr->prev->next = new_event;
        ptr->prev = new_event;
        CALENDER_DEBUG("Insert new event %s into pos %d weekday %d.", new_event->event_name, pos, weekday);
        break;
      }
      pos++;
      ptr = ptr->next;
    }
  }

  if(!ptr->next)
  {
     ptr->next = new_event;
     new_event->prev = ptr;
     new_event->next = NULL;
     CALENDER_DEBUG("Insert new event %s into last pos %d weekday %d.", new_event->event_name, pos, weekday);
  }
}

static Bool retrieve_time_by_dailyrange(IN int32_t range,IN OUT float32_t *start_time, float32_t *stop_time)
{
  if(NULL == start_time || NULL == stop_time)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }


  if(range > DAY_RANGE_LAST)
  {
    CALENDER_DEBUG("daily range value error: %d", range);
    return FAILURE;
  }

  if(WHOLE_DAY == range)
  {
    *start_time = 0.00;
    *stop_time = 24.00;
  }
  else if(MORNING_ONLY == range)
  {
    *start_time = 0.00;
    *stop_time = 11.59;
  }
  else if(AFTERNOON_ONLY == range)
  {
    *start_time = 12.00;
    *stop_time = 17.59;
  }
  else if(NIGHT_ONLY == range)
  {
    *start_time = 18.00;
    *stop_time = 23.59; 
  }

  CALENDER_DEBUG("start time %.2f according to the daily range value: %d", *start_time, range);

  return SUCCESS;
}

static Bool event_pattern_match(char8_t *message, char8_t *event_name)
{
  uint32_t matched_byte = 0;
  uint32_t total_byte = 0;
  char8_t *ptr_event = NULL;
  char8_t word[128];
  char8_t *ptr_word = word;
  uint32_t counter = 0;
  uint32_t matched_index = 0;

  if(NULL == message || NULL == event_name)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  memset(event_tmp, '\0', EVENT_NAME_LEN);
  strncpy(event_tmp, event_name, strlen(event_name));

  convert_message_to_lower_case(event_tmp);

  total_byte = strlen(event_tmp);

  ptr_event = event_tmp;

  CALENDER_DEBUG("start to match [ %s ] with [ %s ]", event_tmp, message);

  while(*ptr_event != '\0')
  {
    if(*ptr_event == ' ')
    {
      *ptr_word = '\0';
      CALENDER_DEBUG("Found a new word: %s counter:%d", word, counter);
      if(strstr(message, word) != NULL)
      {
        matched_byte += counter;
      }
      counter = 0;
      ptr_word = word;
    }
    counter ++;
    *ptr_word++ = *ptr_event++;
  }

  *ptr_word = '\0';

  if(strstr(message, word) != NULL)
  {
    matched_byte += counter;
  }

  matched_index = (matched_byte*100)/total_byte;

  CALENDER_DEBUG("Matched bytes: %d, total bytes:%d, matched_index:%d", matched_byte, total_byte, matched_index);

  if(matched_index >= WORD_MATCH_THREADSHOLD)
  {
    return SUCCESS;
  }
  else
  {
    return FAILURE;
  }
}


/****************************************** export function definition ***************************************/

void calendar_database_init(void)
{
  int32_t i = 0;
  for(i = 0; i < WEEKDAY; i++)
  {
    calendar_database[i].is_free_morning = 1;
    calendar_database[i].is_free_afternoon= 1;
    calendar_database[i].day_info_event.start_time=0;
    calendar_database[i].day_info_event.stop_time=24;
    calendar_database[i].day_info_event.next = NULL;
  }
}

void calendar_database_deinit(void)
{
  int32_t i = 0;
  event_t *ptr = NULL;
  event_t *ptr_tmp = NULL;
  for(i = 0; i < WEEKDAY; i++)
  {
    if(NULL == calendar_database[i].day_info_event.next)
    {
      continue;
    }
    ptr = calendar_database[i].day_info_event.next;
    ptr_tmp = ptr->prev;
    while(ptr)
    {
      ptr_tmp->next = ptr->next;
      if(ptr->next)
      {
        ptr->next->prev = ptr_tmp;
      }
      CALENDER_DEBUG("Remove event %s from weekday %d.", ptr->event_name, i);
      free(ptr);
      ptr = ptr_tmp->next;
    }
    calendar_database[i].day_info_event.next = NULL;
  }

}

Bool calendar_data_base_event_add(uint32_t weekday, float32_t start_time, float32_t stop_time, char8_t *event_name)
{
  if(NULL == event_name)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  event_t *new_event = (event_t *)malloc(sizeof(event_t));

  if(NULL==new_event)
  {
    CALENDER_DEBUG("Malloc failure.");
  }

  strncpy(new_event->event_name, event_name, strlen(event_name));
  new_event->start_time = start_time;
  new_event->stop_time = stop_time;

  event_insert_into_db(weekday, new_event);

  return SUCCESS;
}

Bool calendar_database_retrun_event_by_weekday(IN OUT char8_t *answer, IN int32_t weekday, IN int32_t range)
{
  float32_t start_time = 0;
  float32_t stop_time = 0;
  if(answer == NULL || range > DAY_RANGE_LAST || weekday >WEEKDAY_LAST)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  if(FAILURE == retrieve_time_by_dailyrange(range, &start_time, &stop_time))
  {
    CALENDER_DEBUG("Error: retrieve_time_by_dailyrange.");
    return FAILURE;
  }
  
  event_t *ptr = calendar_database[weekday].day_info_event.next;

  while(NULL != ptr)
  {
    if(ptr->start_time >= start_time)
    {
      strncat(answer, ptr->event_name, strlen(ptr->event_name)+1);
      strncat(answer, ",  ", 4);
      CALENDER_DEBUG("weekday %d append event [%s] into answer list %s.", weekday, ptr->event_name, answer);
    }
    ptr = ptr->next;
  }
  return SUCCESS;
}


inline Bool event_pattern_match_test_wrapper(char8_t *message, char8_t * event_name)
{
  return event_pattern_match(message, event_name);
}


Bool calendar_database_return_weekday_time_by_event(char8_t *message, char8_t *answer, int32_t daylight_range)
{
  uint32_t i = 0;
  uint32_t ret = FAILURE;
  event_t *ptr = NULL;
  char8_t weekday_string[10];
  float32_t start_time = 0;
  float32_t stop_time = 0;

  if(NULL == message || NULL == answer || daylight_range > DAY_RANGE_LAST)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  if(FAILURE == retrieve_time_by_dailyrange(daylight_range, &start_time, &stop_time))
  {
    CALENDER_DEBUG("Error: retrieve_time_by_dailyrange.");
    return FAILURE;
  }

  for(i = 0; i < WEEKDAY; i++)
  {
    event_t *ptr = calendar_database[i].day_info_event.next;
    while(ptr)
    {
      if(SUCCESS == event_pattern_match(message, ptr->event_name))
      {
        if(ptr->start_time > start_time && ptr->start_time < stop_time)
        {
          CALENDER_DEBUG("Find a matched event:[%s] start time:%.2f", ptr->event_name, ptr->start_time);
          convert_weekday_to_string(i, weekday_string);
          strncat(answer, weekday_string, strlen(weekday_string) + 1);
          strncat(answer, ",  ", 4);
          ret = SUCCESS;
        }
      }
      ptr=ptr->next;
    }
  }
  return SUCCESS;
}

Bool calendar_database_return_avail_by_weekday(char8_t *message, char8_t *answer, int32_t daylight_range, int32_t avail)
{
  int32_t i = 0;
  int32_t ret = FAILURE;
  char8_t weekday_string[10];

  if(NULL == message || NULL == answer || daylight_range > DAY_RANGE_LAST || avail > AVAIL_LAST)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  if(daylight_range == WHOLE_DAY)
  {
    if(AVAIL_FREE == avail)
    {
      for(i = 0; i < 7; i++)
      {
        if((calendar_database[i].is_free_morning == 1) && (calendar_database[i].is_free_afternoon == 1))
        {
          memset(weekday_string, '\0', 10);
          convert_weekday_to_string(i, weekday_string);
          strncat(answer, weekday_string, strlen(weekday_string) + 1);
          strncat(answer, ",  ", 4);
          CALENDER_DEBUG("Find %s whole day free.", weekday_string);
          ret = SUCCESS;
        }
      }
    }
    else if(AVAIL_BUSY == avail)
    {
      for(i = 0; i < 7; i++)
      {
        memset(weekday_string, '\0', 10);
        if((calendar_database[i].is_free_morning == 0) || (calendar_database[i].is_free_afternoon == 0))
        {
          convert_weekday_to_string(i, weekday_string);
          strncat(answer, weekday_string, strlen(weekday_string) + 1);
          strncat(answer, ",  ", 4);
          CALENDER_DEBUG("Find %s whole day busy.", weekday_string);
          ret = SUCCESS;
        }
      }
    }
  }
  else if(daylight_range == MORNING_ONLY)
  {
    if(AVAIL_FREE == avail)
    {
      for(i = 0; i < 7; i++)
      {
        memset(weekday_string, '\0', 10);
        if(calendar_database[i].is_free_morning == 1)
        {
          convert_weekday_to_string(i, weekday_string);
          strncat(answer, weekday_string, strlen(weekday_string) + 1);
          strncat(answer, ",  ", 4);
          CALENDER_DEBUG("Find %s morning free.", weekday_string);
          ret = SUCCESS;
        }
      }
    }
    else if(AVAIL_BUSY == avail)
    {
      for(i = 0; i < 7; i++)
      {
        memset(weekday_string, '\0', 10);
        if(calendar_database[i].is_free_morning == 0)
        {
          convert_weekday_to_string(i, weekday_string);
          strncat(answer, weekday_string, strlen(weekday_string) + 1);
          strncat(answer, ",  ", 4);
          CALENDER_DEBUG("Find %s morning busy.", weekday_string);
          ret = SUCCESS;
        }
      }
    }
  }
  else if(daylight_range == AFTERNOON_ONLY)
  {
    if(AVAIL_FREE == avail)
    {
      for(i = 0; i < 7; i++)
      {
        memset(weekday_string, '\0', 10);
        if(calendar_database[i].is_free_afternoon == 1)
        {
          convert_weekday_to_string(i, weekday_string);
          strncat(answer, weekday_string, strlen(weekday_string) + 1);
          strncat(answer, ",  ", 4);
          CALENDER_DEBUG("Find %s afternoon free.", weekday_string);
          ret = SUCCESS;
        }
      }
    }
    else if(AVAIL_BUSY == avail)
    {
      for(i = 0; i < 7; i++)
      {
        memset(weekday_string, '\0', 10);
        if(calendar_database[i].is_free_afternoon == 0)
        {
          convert_weekday_to_string(i, weekday_string);
          strncat(answer, weekday_string, strlen(weekday_string) + 1);
          strncat(answer, ",  ", 4);
          CALENDER_DEBUG("Find %s afternoon busy.", weekday_string);
          ret = SUCCESS;
        }
      }
    }
  }

  return ret;
}

Bool calendar_database_return_weekday_schedule_by_event(char8_t *message, char8_t *answer, int32_t daylight_range)
{
  uint32_t i = 0;
  uint32_t ret = FAILURE;
  event_t *ptr = NULL;
  char8_t weekday_string[10];
  float32_t start_time = 0;
  float32_t stop_time = 0;

  if(NULL == message || NULL == answer || daylight_range > DAY_RANGE_LAST)
  {
    CALENDER_DEBUG("Invalide parameters.");
    return FAILURE;
  }

  if(FAILURE == retrieve_time_by_dailyrange(daylight_range, &start_time, &stop_time))
  {
    CALENDER_DEBUG("Error: retrieve_time_by_dailyrange");
    return FAILURE;
  }

  memset(tmp, '\0', strlen(tmp));

  for(i = 0; i < WEEKDAY; i++)
  {
    event_t *ptr = calendar_database[i].day_info_event.next;
    while(ptr)
    {
      if(SUCCESS == event_pattern_match(message, ptr->event_name))
      {
        if(ptr->start_time > start_time && ptr->start_time < stop_time)
        {
          CALENDER_DEBUG("Find a matched event:[%s] start time:%.2f", ptr->event_name, ptr->start_time);
          convert_weekday_to_string(i, weekday_string);
          snprintf(tmp, strlen(weekday_string) + sizeof(ptr->start_time) + 8, "[%s: %.2f],  ", weekday_string, ptr->start_time);
          strncat(answer, tmp, strlen(tmp) + 1);
          ret = SUCCESS;
        }
      }
      ptr=ptr->next;
    }
  }

  return SUCCESS;
}



