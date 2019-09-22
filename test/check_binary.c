/**
 * Binary test
 */

#include <stdlib.h>
#include <check.h>
#include "../src/wabi_value.h"
#include "../src/wabi_binary.h"

START_TEST(test_binary_create)
{
  ck_assert_int_eq(0 , 0);
}
END_TEST


Suite *
binary_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Binary");

  /* Core test case */
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_binary_create);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void)
{
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = binary_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

}
