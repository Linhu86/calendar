#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "user_input_parse.h"
#include "calendar_manager.h"
#include "common_include.h"
#include "types.h"

/* To implement signal error handler. */

int32_t calendar_exit = CALENDAR_RUNNING;

static void calendar_app_deinit(void)
{
}

static void calendar_app_init(void)
{
  calendar_manager_thread_init();
  user_input_thread_init();
}

int main()
{

  calendar_app_init();

  while(!calendar_exit)
  {
  
  }

  calendar_app_deinit();
  return 0;
}

