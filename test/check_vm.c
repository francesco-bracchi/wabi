/**
 * VM test
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_vm.h"
#include "../src/wabi_env.h"
#include "../src/wabi_cont.h"
#include "../src/wabi_value.h"
#include "../src/wabi_symbol.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_number.h"
#include "../src/wabi_builtin.h"
#include "../src/wabi_cont.h"
#include "../src/wabi_pr.h"
#include "../src/wabi_reader.h"

wabi_vm_t vm;

void
setup(void)
{
  wabi_vm_init(&vm, 10000);
}

void
teardown(void)
{
  wabi_vm_destroy(&vm);
}


START_TEST(function_call)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "(+ 20 30)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(vm.control, wabi_fixnum_new(&vm, 50)), 0);
}
END_TEST


START_TEST(composite_call)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "(+ (+ 20 30) 2)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);

  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(vm.control, wabi_fixnum_new(&vm, 52)), 0);
}
END_TEST

START_TEST(bind)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "(def a 10)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);
  wabi_val sym0 = wabi_symbol_new(&vm, (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "a"));

  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_env_lookup(vm.env, sym0), wabi_fixnum_new(&vm, 10)), 0);
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

  tcase_add_test(tc_core, function_call);
  /* tcase_add_test(tc_core, composite_call); */
  tcase_add_test(tc_core, bind);
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
