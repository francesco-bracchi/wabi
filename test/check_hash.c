/**
 * Hash test
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_vm.h"
#include "../src/wabi_value.h"
#include "../src/wabi_number.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_map.h"
#include "../src/wabi_symbol.h"
#include "../src/wabi_env.h"
#include "../src/wabi_combiner.h"

#include "../src/wabi_hash.h"

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

START_TEST(constants)
{
  wabi_val vnil = wabi_vm_alloc(&vm, 1);
  *vnil = wabi_val_nil;

  wabi_val vfalse = wabi_vm_alloc(&vm, 1);
  *vfalse = wabi_val_false;

  wabi_val vtrue = wabi_vm_alloc(&vm, 1);
  *vtrue = wabi_val_true;

  wabi_val vignore = wabi_vm_alloc(&vm, 1);
  *vignore = wabi_val_ignore;

  ck_assert_int_ne(wabi_hash(vnil), 0);
  ck_assert_int_ne(wabi_hash(vfalse), 0);
  ck_assert_int_ne(wabi_hash(vtrue), 0);
  ck_assert_int_ne(wabi_hash(vignore), 0);
}
END_TEST

START_TEST(numbers)
{
  wabi_val zero, neg, pos;
  zero = (wabi_val) wabi_fixnum_new(&vm, 0UL);
  neg = (wabi_val) wabi_fixnum_new(&vm, -99UL);
  pos = (wabi_val) wabi_fixnum_new(&vm, 19UL);
  ck_assert_int_ne(wabi_hash(neg), 0);
  ck_assert_int_ne(wabi_hash(zero), 0);
  ck_assert_int_ne(wabi_hash(pos), 0);
}
END_TEST

START_TEST(binaries)
{
  wabi_val one;
  one = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "one");
  ck_assert_int_ne(wabi_hash(one), 0);
}
END_TEST


START_TEST(maps)
{
  wabi_map m;
  m = wabi_map_empty(&vm);
  m = wabi_map_assoc(&vm, m, (wabi_val) wabi_fixnum_new(&vm, 0), (wabi_val) wabi_fixnum_new(&vm, 0));;
  ck_assert_int_ne(wabi_hash((wabi_val) m), 0);
}
END_TEST

START_TEST(symbols)
{
  wabi_symbol s;
  wabi_binary b;

  b = (wabi_binary) wabi_binary_leaf_new_from_cstring(&vm, "abc123");
  s = wabi_symbol_new(&vm, (wabi_val) b);

  ck_assert_int_ne(wabi_hash((wabi_val) s), 0);
  ck_assert_int_ne(wabi_hash((wabi_val) s), wabi_hash((wabi_val) b));
}
END_TEST

START_TEST(envs)
{
  wabi_env e;
  e = wabi_env_new(&vm);
  ck_assert_int_ne(wabi_hash((wabi_val) e), 0);
}
END_TEST


START_TEST(combiners)
{
  wabi_binary cname = (wabi_binary) wabi_binary_leaf_new_from_cstring(&vm, "builtin_name");
  wabi_combiner c = wabi_operator_builtin_new(&vm, cname, NULL);
  ck_assert_int_ne(wabi_hash((wabi_val) c), 0);
  ck_assert_int_ne(wabi_hash((wabi_val) c), wabi_hash((wabi_val) cname));
}
END_TEST

// TODO:
// 1. test derived,
// 2. test (wrap op) != op
// 3. (wrap bt_op) != bt_op

Suite *
map_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Hash");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_checked_fixture(tc_core, setup, teardown);

  tcase_add_test(tc_core, constants);
  tcase_add_test(tc_core, numbers);
  tcase_add_test(tc_core, binaries);
  tcase_add_test(tc_core, maps);
  tcase_add_test(tc_core, symbols);
  tcase_add_test(tc_core, envs);
  tcase_add_test(tc_core, combiners);
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
