#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "helper_func.h"

#define STR_MONDAY     "Monday"
#define STR_TUESDAY    "Tuesday"
#define STR_WEDNESDAY  "Wednesday"
#define STR_THURSDAY   "Thursday"
#define STR_FRIDAY     "Friday"
#define STR_SATURDAY   "Saturday"
#define STR_SUNDAY     "Sunday"


void convert_message_to_lower_case(IN OUT char8_t *message)
{
   char8_t *ptr = message;
   uint32_t i = 0;
   for(i = 0; i < strlen(message); i++)
   {
      if((*ptr)>='A' && (*ptr)<='Z')
      {
         *ptr= (*ptr)+32;
      }
      ptr++;
   }
}


void convert_weekday_to_string(IN uint32_t weekday, IN OUT char8_t *weekday_string)
{
  if(NULL == weekday_string || weekday > 7)
    return;

  switch(weekday)
  {
    case 0:
        strncpy(weekday_string, STR_MONDAY, strlen(STR_MONDAY));
        break;
    case 1:
        strncpy(weekday_string, STR_TUESDAY, strlen(STR_TUESDAY));
        break;
    case 2:
        strncpy(weekday_string, STR_WEDNESDAY, strlen(STR_WEDNESDAY));
        break;
    case 3:
        strncpy(weekday_string, STR_THURSDAY, strlen(STR_THURSDAY));
        break;
    case 4:
        strncpy(weekday_string, STR_FRIDAY, strlen(STR_FRIDAY));
        break;
    case 5:
        strncpy(weekday_string, STR_SATURDAY, strlen(STR_SATURDAY));
        break;
    case 6:
        strncpy(weekday_string, STR_SUNDAY, strlen(STR_SUNDAY));
        break;

  }

}

