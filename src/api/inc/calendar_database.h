#ifndef __CALENDAR_DATABASE_H__
#define __CALENDAR_DATABASE_H__

#include "common_include.h"
#include "types.h"

#define WORD_MATCH_THREADSHOLD 80

void calendar_database_deinit(void);

void calendar_database_init(void);

Bool calendar_data_base_event_add(uint32_t weekday, float32_t start_time, float32_t stop_time, char8_t *event_name);

void event_return_all_by_weekday(IN OUT char8_t *answer, IN int32_t weekday, IN int32_t range);

Bool event_pattern_match_calendar_weekday(char8_t *message, char *answer);

Bool event_pattern_match_test_wrapper(char8_t *message, char8_t * event_name);


#endif

