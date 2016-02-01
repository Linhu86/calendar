/* calendar_main.c */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "user_input_parse.h"
#include "calendar_manager.h"
#include "common_include.h"
#include "calendar_database.h"
#include "rtos.h"
#include "types.h"

extern int32_t calendar_exit;

static void calendar_app_init(void)
{

#ifdef OS_LINUX
  signal_handle();
#endif

  calendar_database_init();

  parse_file();

  mutex_lock_init();
  
  calendar_manager_thread_init();
  
  user_input_thread_init();
}

static void calendar_app_deinit(void)
{
  calendar_database_deinit();
  mutex_lock_deinit();
}


int32_t main()
{
  calendar_app_init();

  while(!calendar_exit)
  {
  
  }

  calendar_app_deinit();
  return 0;
}

