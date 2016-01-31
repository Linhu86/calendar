#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calendar_database.h"
#include "common_include.h"

/* calendar database

calendar_database

Every day owns an event list sorted by start time.

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

void calendar_database_init(void)
{
  int i = 0;
  for(i = 0; i < WEEKDAY; i++)
  {
    calendar_database[i].is_free_morning = 1;
    calendar_database[i].is_free_afternoon= 1;
    calendar_database[i].day_info_event.start_time=0;
    calendar_database[i].day_info_event.stop_time=0;
    calendar_database[i].day_info_event.next = NULL;
  }
}

void calendar_database_deinit(void)
{

}

Bool calendar_data_base_event_add(uint32_t weekday, uint32_t start_time, uint32_t stop_time, char8_t *event_name)
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

  
  return SUCCESS;
}



