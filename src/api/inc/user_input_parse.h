#ifndef __USER_INPUT_PARSE_H__
#define __USER_INPUT_PARSE_H__

#include "types.h"
#include "common_include.h"

#define THREAD_USER_INPUT_PROCESS_NAME "thread_user_input_proc"

#define CALENDAR_FILE_NAME "calendar.txt"

#define MAX_LINE_SIZE 256

void user_input_thread_init(void);

void parse_file(void);


/* wrapper function exported for unit test. */

inline Bool is_valid_time_test_wrapper(char8_t *word, float32_t *start_time, float32_t *stop_time);

inline uint32_t is_weekday_test_wrapper(char8_t *day);

inline Bool check_line_format_test_wrapper(char8_t *line);

inline Bool line_parse_test_wrapper(char8_t *line);


#endif

