/**
 * Env test
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_vm.h"
#include "../src/wabi_value.h"
#include "../src/wabi_symbol.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_number.h"
#include "../src/wabi_cmp.h"

#include "../src/wabi_env.h"

wabi_vm_t vm;

void
setup(void)
{
  wabi_vm_init(&vm, 100000);
}

void
teardown(void)
{
  wabi_vm_destroy(&vm);
}

START_TEST(empty)
{
  wabi_env env;
  env = wabi_env_new(&vm);
  ck_assert_ptr_nonnull(env);
}
END_TEST


START_TEST(insert)
{
  wabi_env env;
  wabi_symbol k;
  wabi_val v, v0;
  env = wabi_env_new(&vm);
  k = wabi_symbol_new(&vm, (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "key"));
  v0 = wabi_fixnum_new(&vm, 12U);
  wabi_env_set(&vm, env, k, v0);
  v = wabi_env_lookup(env, k);
  ck_assert_int_eq(0, wabi_cmp(v, v0));
}
END_TEST

START_TEST(extend)
{
  wabi_env env;
  wabi_symbol k;
  wabi_val v, v0;
  env = wabi_env_new(&vm);
  k = wabi_symbol_new(&vm, (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "key"));
  v0 = wabi_fixnum_new(&vm, 12U);
  wabi_env_set(&vm, env, k, v0);
  env = wabi_env_extend(&vm, env);
  v = wabi_env_lookup(env, k);

  ck_assert_int_eq(0, wabi_cmp(v, v0));
}
END_TEST

START_TEST(shadow)
{
  wabi_env env, env0;
  wabi_symbol k;
  wabi_val v, v0, v1;
  env0 = wabi_env_new(&vm);
  k = wabi_symbol_new(&vm, (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "key"));
  v0 = wabi_fixnum_new(&vm, 12U);
  wabi_env_set(&vm, env0, k, v0);
  env = wabi_env_extend(&vm, env0);
  v = wabi_fixnum_new(&vm, 42U);
  wabi_env_set(&vm, env, k, v);

  ck_assert_int_eq(0, wabi_cmp(wabi_env_lookup(env0, k), v0));
  ck_assert_int_eq(0, wabi_cmp(wabi_env_lookup(env, k), v));
}
END_TEST


Suite *
map_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Env");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_checked_fixture(tc_core, setup, teardown);

  tcase_add_test(tc_core, empty);
  tcase_add_test(tc_core, insert);
  /* tcase_add_test(tc_core, extend); */
  /* tcase_add_test(tc_core, shadow); */

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
