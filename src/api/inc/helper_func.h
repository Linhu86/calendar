#ifndef __HELPER_FUNC_H__
#define __HELPER_FUNC_H__

#include "rtos.h"
#include "types.h"

void convert_message_to_lower_case(IN OUT char8_t *message);

void convert_weekday_to_string(IN uint32_t weekday, IN OUT char8_t *weekday_string);


#endif

