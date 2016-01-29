#ifndef __CALENDAR_MAIN_H__
#define __CALENDAR_MAIN_H__

#define QUEUE_NAME  "/calendar_msgq"
#define MAX_MSG_QUEUE_SIZE    1024
#define MSG_STOP    "exit"

#define MAX_BUFFER_SIZE 1024

#ifdef DEBUG_INFO_ON
#define CALENDER_DEBUG printf
#else
#define CALENDER_DEBUG
#endif

#endif

