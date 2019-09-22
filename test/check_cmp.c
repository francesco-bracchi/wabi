/**
 * Compare test
 * All values are stricly ordered
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_vm.h"
#include "../src/wabi_value.h"
#include "../src/wabi_cmp.h"
#include "../src/wabi_number.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_map.h"
#include "../src/wabi_symbol.h"
#include "../src/wabi_env.h"
#include "../src/wabi_combiner.h"

#include "../src/wabi_pr.h"


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

START_TEST(types)
{
  wabi_val nil = wabi_vm_alloc(&vm, 1);
  *nil = wabi_val_nil;

  wabi_val num = (wabi_val) wabi_fixnum_new(&vm, 0);
  wabi_val pair = (wabi_val) wabi_cons(&vm, num, nil);
  wabi_val bin = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "binary");
  wabi_val sym = (wabi_val) wabi_symbol_new(&vm, bin);
  wabi_val map = (wabi_val) wabi_map_empty(&vm);

  ck_assert_int_gt(wabi_cmp(num, nil), 0);
  ck_assert_int_gt(wabi_cmp(sym, num), 0);
  ck_assert_int_gt(wabi_cmp(pair, sym), 0);
  ck_assert_int_gt(wabi_cmp(bin, pair), 0);
  ck_assert_int_gt(wabi_cmp(map, bin), 0);
}
END_TEST

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

  ck_assert_int_eq(wabi_cmp(vnil, vnil), 0);
  ck_assert_int_eq(wabi_cmp(vfalse, vfalse), 0);
  ck_assert_int_eq(wabi_cmp(vtrue, vtrue), 0);
  ck_assert_int_eq(wabi_cmp(vignore, vignore), 0);

  ck_assert_int_eq(wabi_cmp(vignore, vnil), 3);

  ck_assert_int_gt(wabi_cmp(vfalse, vnil), 0);
  ck_assert_int_gt(wabi_cmp(vtrue, vfalse), 0);
  ck_assert_int_gt(wabi_cmp(vignore, vtrue), 0);
}
END_TEST

START_TEST(symbols)
{
  wabi_val oneb = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "one");
  wabi_val twob = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "two");
  wabi_val one = (wabi_val) wabi_symbol_new(&vm, oneb);
  wabi_val two = (wabi_val) wabi_symbol_new(&vm, twob);
  wabi_val one1 = (wabi_val) wabi_symbol_new(&vm, oneb);
  ck_assert_int_lt(wabi_cmp(one, two), 0);
  ck_assert_int_eq(wabi_cmp(one, one1), 0);
}
END_TEST

START_TEST(numbers)
{
  wabi_val zero = (wabi_val) wabi_fixnum_new(&vm, 0UL);
  wabi_val neg = (wabi_val) wabi_fixnum_new(&vm, -99UL);
  wabi_val pos = (wabi_val) wabi_fixnum_new(&vm, 19UL);

  ck_assert_int_gt(wabi_cmp(neg, zero), 0);
  ck_assert_int_gt(wabi_cmp(zero, pos), 0);
  ck_assert_int_gt(wabi_cmp(neg, pos), 0);
  ck_assert_int_eq(wabi_cmp(neg, neg), 0);
}
END_TEST

START_TEST(binaries)
{
  wabi_val one = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "one");
  wabi_val two = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "two");

  ck_assert_int_lt(wabi_cmp(one, two), 0);
}
END_TEST


START_TEST(maps)
{
  wabi_map m, m0;
  m = wabi_map_empty(&vm);
  m = wabi_map_assoc(&vm, m, (wabi_val) wabi_fixnum_new(&vm, 0), (wabi_val) wabi_fixnum_new(&vm, 0));
  m0 = wabi_map_assoc(&vm, m, (wabi_val) wabi_fixnum_new(&vm, 0), (wabi_val) wabi_fixnum_new(&vm, 1));
  ck_assert_int_gt(wabi_cmp((wabi_val) m, (wabi_val) m0), 0);
}
END_TEST


START_TEST(envs)
{
  wabi_env e0, e;
  e0 = wabi_env_new(&vm);
  e = wabi_env_extend(&vm, e0);
  ck_assert_int_ne(wabi_cmp((wabi_val) e0, (wabi_val) e), 0);
}
END_TEST


START_TEST(combiners)
{
  wabi_combiner c0, c;
  wabi_binary cname0, cname;
  cname0 = (wabi_binary) wabi_binary_leaf_new_from_cstring(&vm, "cname0");
  cname = (wabi_binary) wabi_binary_leaf_new_from_cstring(&vm, "cname");

  c0 = wabi_combiner_builtin_new(&vm, cname0, NULL);
  c = wabi_combiner_builtin_new(&vm, cname, NULL);
  ck_assert_int_lt(wabi_cmp((wabi_val) c, (wabi_val) c0), 0);
}
END_TEST

// TODO:
// 1. test compare derived
// 2. test compare (wrap op), op
// 3. test compare (wrap bt_op), bt_op

Suite *
map_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Compare");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_checked_fixture(tc_core, setup, teardown);

  tcase_add_test(tc_core, types);
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
