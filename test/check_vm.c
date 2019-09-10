/**
 * VM test
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_store.h"
#include "../src/wabi_env.h"
#include "../src/wabi_cont.h"
#include "../src/wabi_value.h"
#include "../src/wabi_vm.h"


wabi_store_t store;

void
setup(void)
{
  wabi_store_init(&store, 100000);
}

void
teardown(void)
{
  wabi_store_destroy(&store);
}

START_TEST(empty)
{
  ck_assert_int_eq(1, 0);
}
END_TEST

Suite *
map_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Vm");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_checked_fixture(tc_core, setup, teardown);

  tcase_add_test(tc_core, empty);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void)
{
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = map_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
