#ifndef __COMMOM_INCLUDE_H__
#define __COMMOM_INCLUDE_H__

#define QUEUE_NAME "/calendar_msgq"

#define MAX_BUFFER_SIZE 1024

typedef enum{
  CALENDAR_RUNNING = 0,
  CALENDAR_EXIT = 1
} CALENDAR_STATUS;

/* should implement lock to aviod race condition.*/ 
#define calendar_quit(void){ \
  calendar_exit = CALENDAR_EXIT; \
}

#endif

