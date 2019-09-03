/**
 * Compare test
 * All values are stricly ordered
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_store.h"
#include "../src/wabi_value.h"
#include "../src/wabi_cmp.h"
#include "../src/wabi_number.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_map.h"
#include "../src/wabi_symbol.h"

#include "../src/wabi_pr.h"


wabi_store_t store;

void
setup(void)
{
  wabi_store_init(&store, 100000);
}

void
teardown(void)
{
  wabi_store_free(&store);
}

START_TEST(types)
{
  wabi_val nil = wabi_store_heap_alloc(&store, 1);
  *nil = wabi_val_nil;

  wabi_val num = (wabi_val) wabi_fixnum_new(&store, 0);

  wabi_val pair = (wabi_val) wabi_cons(&store, num, nil);

  wabi_val bin = (wabi_val) wabi_binary_leaf_new_from_cstring(&store, "binary");

  wabi_val sym = (wabi_val) wabi_symbol_new(&store, bin);

  wabi_val map = (wabi_val) wabi_map_empty(&store);

  ck_assert_int_gt(wabi_cmp(num, nil), 0);
  ck_assert_int_gt(wabi_cmp(sym, num), 0);
  ck_assert_int_gt(wabi_cmp(pair, sym), 0);
  ck_assert_int_gt(wabi_cmp(bin, pair), 0);
  ck_assert_int_gt(wabi_cmp(map, bin), 0);
}
END_TEST

START_TEST(constants)
{
  wabi_val vnil = wabi_store_heap_alloc(&store, 1);
  *vnil = wabi_val_nil;

  wabi_val vfalse = wabi_store_heap_alloc(&store, 1);
  *vfalse = wabi_val_false;

  wabi_val vtrue = wabi_store_heap_alloc(&store, 1);
  *vtrue = wabi_val_true;

  wabi_val vignore = wabi_store_heap_alloc(&store, 1);
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
  wabi_val oneb = (wabi_val) wabi_binary_leaf_new_from_cstring(&store, "one");
  wabi_val twob = (wabi_val) wabi_binary_leaf_new_from_cstring(&store, "two");
  wabi_val one = (wabi_val) wabi_symbol_new(&store, oneb);
  wabi_val two = (wabi_val) wabi_symbol_new(&store, twob);
  wabi_val one1 = (wabi_val) wabi_symbol_new(&store, oneb);
  ck_assert_int_lt(wabi_cmp(one, two), 0);
  ck_assert_int_eq(wabi_cmp(one, one1), 0);
}
END_TEST

START_TEST(numbers)
{
  wabi_val zero = (wabi_val) wabi_fixnum_new(&store, 0UL);
  wabi_val neg = (wabi_val) wabi_fixnum_new(&store, -99UL);
  wabi_val pos = (wabi_val) wabi_fixnum_new(&store, 19UL);

  ck_assert_int_gt(wabi_cmp(neg, zero), 0);
  ck_assert_int_gt(wabi_cmp(zero, pos), 0);
  ck_assert_int_gt(wabi_cmp(neg, pos), 0);
  ck_assert_int_eq(wabi_cmp(neg, neg), 0);
}
END_TEST

START_TEST(binaries)
{
  wabi_val one = (wabi_val) wabi_binary_leaf_new_from_cstring(&store, "one");
  wabi_val two = (wabi_val) wabi_binary_leaf_new_from_cstring(&store, "two");

  ck_assert_int_lt(wabi_cmp(one, two), 0);
}
END_TEST


START_TEST(maps)
{
  wabi_map m, m0;
  m = wabi_map_empty(&store);
  m = wabi_map_assoc(&store, m, (wabi_val) wabi_fixnum_new(&store, 0), (wabi_val) wabi_fixnum_new(&store, 0));
  m0 = wabi_map_assoc(&store, m, (wabi_val) wabi_fixnum_new(&store, 0), (wabi_val) wabi_fixnum_new(&store, 1));
  ck_assert_int_gt(wabi_cmp((wabi_val) m, (wabi_val) m0), 0);
}
END_TEST


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
