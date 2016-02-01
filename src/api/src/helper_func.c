#include <stdio.h>
#include <string.h>

#include "helper_func.h"


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

