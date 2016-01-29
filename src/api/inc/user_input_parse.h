#ifndef __USER_INPUT_PARSE_H__
#define __USER_INPUT_PARSE_H__

#define THREAD_USER_INPUT_PROCESS_NAME "thread_user_input_proc"

#define QUEUE_NAME "/calendar_msgq"

#define MAX_BUFFER_SIZE 1024

void user_input_thread_init(void);

#endif

