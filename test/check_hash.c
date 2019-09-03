/**
 * Hash test
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_store.h"
#include "../src/wabi_value.h"
#include "../src/wabi_number.h"
#include "../src/wabi_pair.h"
#include "../src/wabi_binary.h"
#include "../src/wabi_map.h"
#include "../src/wabi_symbol.h"

#include "../src/wabi_hash.h"

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

  ck_assert_int_ne(wabi_hash(vnil), 0);
  ck_assert_int_ne(wabi_hash(vfalse), 0);
  ck_assert_int_ne(wabi_hash(vtrue), 0);
  ck_assert_int_ne(wabi_hash(vignore), 0);
}
END_TEST

START_TEST(numbers)
{
  wabi_val zero, neg, pos;
  zero = (wabi_val) wabi_fixnum_new(&store, 0UL);
  neg = (wabi_val) wabi_fixnum_new(&store, -99UL);
  pos = (wabi_val) wabi_fixnum_new(&store, 19UL);
  ck_assert_int_ne(wabi_hash(neg), 0);
  ck_assert_int_ne(wabi_hash(zero), 0);
  ck_assert_int_ne(wabi_hash(pos), 0);
}
END_TEST

START_TEST(binaries)
{
  wabi_val one;
  one = (wabi_val) wabi_binary_leaf_new_from_cstring(&store, "one");
  ck_assert_int_ne(wabi_hash(one), 0);
}
END_TEST


START_TEST(maps)
{
  wabi_map m;
  m = wabi_map_empty(&store);
  m = wabi_map_assoc(&store, m, (wabi_val) wabi_fixnum_new(&store, 0), (wabi_val) wabi_fixnum_new(&store, 0));;
  ck_assert_int_ne(wabi_hash((wabi_val) m), 0);
}
END_TEST

START_TEST(symbols)
{
  wabi_symbol s;
  wabi_binary b;

  b = (wabi_binary) wabi_binary_leaf_new_from_cstring(&store, "abc123");
  s = wabi_symbol_new(&store, (wabi_val) b);

  ck_assert_int_ne(wabi_hash((wabi_val) s), 0);
  ck_assert_int_ne(wabi_hash((wabi_val) s), wabi_hash((wabi_val) b));
}
END_TEST


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
