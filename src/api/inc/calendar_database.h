#ifndef __CALENDAR_DATABASE_H__
#define __CALENDAR_DATABASE_H__

#include "common_include.h"
#include "types.h"

#define WORD_MATCH_THREADSHOLD 80

void calendar_database_deinit(void);

void calendar_database_init(void);

Bool calendar_data_base_event_add(uint32_t weekday, float32_t start_time, float32_t stop_time, char8_t *event_name);

Bool calendar_database_retrun_event_by_weekday(IN OUT char8_t *answer, IN int32_t weekday, IN int32_t range);

Bool calendar_database_return_weekday_time_by_event(char8_t *message, char8_t *answer, int32_t daylight_range);

Bool calendar_database_return_weekday_schedule_by_event(char8_t *message, char8_t *answer, int32_t daylight_range);

Bool calendar_database_return_avail_by_weekday(char8_t *message, char8_t *answer, int32_t daylight_range, int32_t avail);

/* wrapper function exported for unit test. */
inline Bool event_pattern_match_test_wrapper(char8_t *message, char8_t * event_name);


#endif

