#ifndef __CALENDAR_DATABASE_H__
#define __CALENDAR_DATABASE_H__

#include "common_include.h"
#include "types.h"

void calendar_database_deinit(void);

void calendar_database_init(void);

Bool calendar_data_base_event_add(uint32_t weekday, float32_t start_time, float32_t stop_time, char8_t *event_name);

void event_return_all_by_weekday(IN OUT char8_t *answer, IN int32_t weekday, IN int32_t range);

#endif

