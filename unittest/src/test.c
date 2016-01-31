#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rtos.h"
#include "common_include.h"
#include "user_input_parse.h"
#include "CUnit/Basic.h"

int init_suite_rtos(void)
{
  system("touch tmp.txt");
  return 0; 
}

int clean_suite_rtos(void)
{
  return 0;
}

static void test_FsOpen(void)
{
  file_hdl fd;
  CU_ASSERT(SUCCESS == FsOpen(&fd, "tmp.txt", OPEN_MODE_READ_ONLY));
  CU_ASSERT(NULL != fd);
}

int init_suite_string_parse(void)
{
  return 0; 
}

int clean_suite_string_parse(void)
{
  return 0;
}

static void test_string_parse(void)
{
  /* is_weekday test. */
  CU_ASSERT(MONDAY_IDX == is_weekday_test_wrapper("Monday"));
  CU_ASSERT(TUESDAY_IDX == is_weekday_test_wrapper("Tuesday"));
  CU_ASSERT(WEDNESDAY_IDX == is_weekday_test_wrapper("Wednesday"));
  CU_ASSERT(THUESDAY_IDX == is_weekday_test_wrapper("Thursday"));
  CU_ASSERT(FRIDAY_IDX == is_weekday_test_wrapper("Friday"));
  CU_ASSERT(SATURDAY_IDX == is_weekday_test_wrapper("Saturday"));
  CU_ASSERT(SUNDAY_IDX == is_weekday_test_wrapper("Sunday"));
  CU_ASSERT(-1 == is_weekday_test_wrapper(""));
  CU_ASSERT(-1 == is_weekday_test_wrapper("SUN"));


  /*is_valid_time test */
  float32_t start_time = 0, stop_time = 0;
  CU_ASSERT(SUCCESS == is_valid_time_test_wrapper("8:30-13:30", &start_time, &stop_time));
  printf("%.2f %.2f\n", start_time, stop_time);
  CU_ASSERT(1 == FLOAT_COMP(8.30, start_time));
  CU_ASSERT(1 == FLOAT_COMP(13.30, stop_time));

  start_time = 0;
  stop_time = 0;
  CU_ASSERT(SUCCESS == is_valid_time_test_wrapper("00:00-9:00", &start_time, &stop_time));
  CU_ASSERT(1 == FLOAT_COMP(0.00, start_time));
  CU_ASSERT(1 == FLOAT_COMP(9.00, stop_time));

  start_time = 0;
  stop_time = 0;
  CU_ASSERT(SUCCESS == is_valid_time_test_wrapper("11:00-23:00", &start_time, &stop_time));
  CU_ASSERT(1==FLOAT_COMP(11.00, start_time));
  CU_ASSERT(1==FLOAT_COMP(23.00, stop_time));

  start_time = 0;
  stop_time = 0;
  CU_ASSERT(FAILURE == is_valid_time_test_wrapper("112:00-23:00", &start_time, &stop_time));
  CU_ASSERT(FAILURE == is_valid_time_test_wrapper("", &start_time, &stop_time));
  CU_ASSERT(FAILURE == is_valid_time_test_wrapper("12:00-238:00", &start_time, &stop_time));

  /*check_line_format_test */
  CU_ASSERT(SUCCESS == check_line_format_test_wrapper("MONDAY 10:00-11:00 Pickup Child."));
  CU_ASSERT(SUCCESS  == check_line_format_test_wrapper("MONDAY  10:00-11:00 Pickup Child."));
  CU_ASSERT(SUCCESS  == check_line_format_test_wrapper("Monday 16:00 Pick kids up"));
  CU_ASSERT(SUCCESS  == check_line_format_test_wrapper("Monday 10:00-11:00 Dentist."));
  CU_ASSERT(FAILURE  == check_line_format_test_wrapper(""));

  CU_ASSERT(SUCCESS == line_parse_test_wrapper("MONDAY 10:00-11:00 Pickup Child."));  
}



/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Suite_rtos_test", init_suite_rtos, clean_suite_rtos);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (NULL == CU_add_test(pSuite, "test of FsOpen", test_FsOpen))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (NULL == CU_add_test(pSuite, "test of string parse", test_string_parse))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}

