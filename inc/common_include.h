#ifndef __COMMOM_INCLUDE_H__
#define __COMMOM_INCLUDE_H__

#include "types.h"

#define QUEUE_NAME "/calendar_msgq"

#define MAX_BUFFER_SIZE 1024

#define EVENT_NAME_LEN 256

#define WEEKDAY 7

typedef enum{
    MONDAY_IDX=0,
    TUESDAY_IDX,
    WEDNESDAY_IDX,
    THUESDAY_IDX,
    FRIDAY_IDX,
    SATURDAY_IDX,
    SUNDAY_IDX
} day_idx;

#define EPSINON 0.00001
#define FLOAT_COMP(a, b)  (((a-b >= - EPSINON) && (a-b <= EPSINON)) ? 1 : 0)


typedef enum{
  CALENDAR_RUNNING = 0,
  CALENDAR_EXIT = 1
} CALENDAR_STATUS;

typedef struct event
{
  char8_t event_name[EVENT_NAME_LEN];
  float32_t start_time;
  float32_t stop_time;
  struct event *next;
  struct event *prev;
} event_t;


/* should implement lock to aviod race condition.*/ 
#define calendar_quit(void){ \
  calendar_exit = CALENDAR_EXIT; \
}

#endif

