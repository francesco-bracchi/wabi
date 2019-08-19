/**
 * Store test
 */

#include <stdlib.h>
#include <check.h>
#include "../src/wabi_store.h"

START_TEST(test_store_create)
{
  wabi_store_t s;
  int res;

  s.space = NULL;
  res = wabi_store_init(&s, 1000);
  ck_assert_int_eq(res, 1);
  ck_assert_ptr_nonnull(s.space);
  wabi_store_free(&s);
}
END_TEST

Suite *
store_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Store");

  /* Core test case */
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_store_create);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void)
{
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = store_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

}
