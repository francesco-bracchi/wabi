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
#include "../src/wabi_map.h"
#include "../src/wabi_reader.h"
#include "../src/wabi_pr.h"

wabi_vm_t vm;


void
setup(void)
{
  wabi_vm_init(&vm, 50000);
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
  vm.control = wabi_reader_read(&vm, "(/ (+ 20 30) 2)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);

  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(vm.control, wabi_fixnum_new(&vm, 25)), 0);
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
  ck_assert_int_eq(wabi_cmp(wabi_env_lookup((wabi_env) vm.env, sym0), wabi_fixnum_new(&vm, 10)), 0);
}
END_TEST


/*** builtin test ***/
START_TEST(operative)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "(def q (fx e (a) a))");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);
  wabi_val sym0 = wabi_symbol_new(&vm, (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "q"));
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_ptr_nonnull(wabi_env_lookup((wabi_env) vm.env, sym0));
}
END_TEST


START_TEST(invoke_derived)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "((fx e (a) a) (+ 10 20))");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_reader_read(&vm, "(+ 10 20)"), vm.control), 0);

}
END_TEST


/*** builtin test ***/
START_TEST(wrap_fx)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "((wrap (fx e (a) a)) (+ 10 20))");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_reader_read(&vm, "30"), vm.control), 0);

}
END_TEST


/*** builtin test ***/
START_TEST(unwrap)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "((unwrap (wrap (fx e (a) a))) (+ 10 20))");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_reader_read(&vm, "(+ 10 20)"), vm.control), 0);

}
END_TEST


/*** builtin test ***/
START_TEST(map)
{
  wabi_env e0;
  e0 = wabi_builtin_stdenv(&vm);
  vm.control = wabi_reader_read(&vm, "(hmap \"one\" 1 \"two\" (+ 1 1))");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);

  int res = wabi_vm_run(&vm);

  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_map_length((wabi_map) vm.control), 2);

}
END_TEST


START_TEST(branch)
{
  wabi_env e0;
  int res;

  e0 = wabi_builtin_stdenv(&vm);

  vm.control = wabi_reader_read(&vm, "(if true 0 20)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);
  res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_reader_read(&vm, "0"), vm.control), 0);

  vm.control = wabi_reader_read(&vm, "(if 0 0 20)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);
  res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_reader_read(&vm, "0"), vm.control), 0);

  vm.control = wabi_reader_read(&vm, "(if false 0 20)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);
  res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_reader_read(&vm, "20"), vm.control), 0);

  vm.control = wabi_reader_read(&vm, "(if nil 0 20)");
  vm.continuation = (wabi_val) wabi_cont_eval_new(&vm, e0, NULL);
  res = wabi_vm_run(&vm);
  ck_assert_int_eq(res, wabi_vm_result_done);
  ck_assert_int_eq(wabi_cmp(wabi_reader_read(&vm, "20"), vm.control), 0);
}
END_TEST


START_TEST(load)
{
  wabi_env e0;
  int res;
  char * buffer = 0;

  long length;
  FILE * f = fopen("test/test.wabi", "rb");

  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  buffer = malloc(length + 1);
  fread(buffer, 1, length, f);
  buffer[length] = '\0';
  fclose (f);

  e0 = wabi_builtin_stdenv(&vm);
  wabi_builtin_load(&vm, e0, buffer);
  if(vm.errno != 0) {
    printf("error\n");
  }

  printf("result\n");
  wabi_pr(vm.control);
  printf("\n");
  ck_assert_int_eq(vm.errno, 0);
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

  /* tcase_add_test(tc_core, function_call); */
  /* tcase_add_test(tc_core, composite_call); */
  /* tcase_add_test(tc_core, bind); */
  /* tcase_add_test(tc_core, operative); */
  /* tcase_add_test(tc_core, invoke_derived); */
  /* tcase_add_test(tc_core, wrap_fx); */
  /* tcase_add_test(tc_core, unwrap); */
  /* tcase_add_test(tc_core, branch); */
  /* tcase_add_test(tc_core, map); */
  tcase_add_test(tc_core, load);

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
