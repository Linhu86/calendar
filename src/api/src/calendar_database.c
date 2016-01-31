#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calendar_database.h"
#include "common_include.h"

/* calendar database

calendar_database

Every day owns an event bidirection list sorted by start time.

[ Monday ]      -> [event 1] <-> [event 2] <->...<-> [event n] ->NULL 

[ Tuesday ]     -> [event 1] <-> [event 2] <->...<-> [event n] ->NULL

[ Wednesday ] -> [event 1] <-> [event 2] <->...<-> [event n] ->NULL

      .
      .
      .
[ Sunday ]       -> [event 1] -> [event 2] ->...-> [event n] ->NULL

*/

typedef struct dayinfo
{
  Bool is_free_morning;
  Bool is_free_afternoon;
  struct event day_info_event;
} dayinfo_t;

dayinfo_t calendar_database[WEEKDAY];

static Bool event_search_from_db_by_time(IN uint32_t weekday, IN float32_t start_time, OUT char8_t *event_name)
{
  event_t *ptr = calendar_database[weekday].day_info_event.next;

  while(!ptr)
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
  event_t *ptr = calendar_database[weekday].day_info_event.next;

  while(!ptr)
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
  int pos = 1;

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
    while(!ptr)
    {
      if(ptr->next)
        ptr = ptr->next;

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
    }
  }

  if(!ptr)
  {
     ptr->next = new_event;
     new_event->prev = ptr;
     new_event->next = NULL;
     CALENDER_DEBUG("Insert new event %s into last pos %d weekday %d.", new_event->event_name, pos, weekday);
  }
}


void calendar_database_init(void)
{
  int i = 0;
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
  int i = 0;
  event_t *ptr = NULL;
  event_t *ptr_tmp = NULL;
  for(i = 0; i < WEEKDAY; i++)
  {
    if(NULL == calendar_database[i].day_info_event.next)
        continue;
    ptr = &calendar_database[i].day_info_event;
    ptr_tmp = ptr->prev;
    while(!ptr)
    {
       ptr_tmp->next = ptr->next;
       ptr->next->prev = ptr_tmp;
       CALENDER_DEBUG("Remove event %s into last pos %d weekday %d.", ptr->event_name, i);
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



