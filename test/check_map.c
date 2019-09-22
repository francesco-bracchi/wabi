/**
 * Map test
 */

#include <stdlib.h>
#include <stdio.h>

#include <check.h>
#include "../src/wabi_vm.h"
#include "../src/wabi_value.h"
#include "../src/wabi_map.h"
#include "../src/wabi_binary.h"

wabi_vm_t vm;

void
setup(void)
{
  wabi_vm_init(&vm, 400000);
}

void
teardown(void)
{
  wabi_vm_destroy(&vm);
}

START_TEST(assoc_empty)
{
  wabi_map m = wabi_map_empty(&vm);
  wabi_val k = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "key");
  wabi_val v = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "value");
  m = wabi_map_assoc(&vm, m, k, v);

  ck_assert_int_eq(1, wabi_map_length(m));
}
END_TEST

START_TEST(assoc_many)
{
  char str[100];
  wabi_map m;
  wabi_binary_leaf k, v, v0;
  int j, size;

  size = 2000;

  m = wabi_map_empty(&vm);
  for(j = 0; j < size; j++) {
    sprintf(str, "%iK", j);
    k = wabi_binary_leaf_new_from_cstring(&vm, str);
    sprintf(str, "%iV", j);
    v = wabi_binary_leaf_new_from_cstring(&vm, str);
    m = wabi_map_assoc(&vm, m, (wabi_val) k, (wabi_val) v);
  }
  for(j = 0; j < size; j++) {
    sprintf(str, "%iK", j);
    k = wabi_binary_leaf_new_from_cstring(&vm, str);
    sprintf(str, "%iV", j);
    v0 = wabi_binary_leaf_new_from_cstring(&vm, str);
    v = (wabi_binary_leaf) wabi_map_get(m, (wabi_val) k);

    ck_assert_int_eq(wabi_cmp((wabi_val) v, (wabi_val) v0), 0);
  }
}
END_TEST

START_TEST(assoc_update)
{
  char str[100];
  wabi_map m;
  wabi_binary_leaf k, v, v0;
  int j, size;

  size = 200;
  m = wabi_map_empty(&vm);
  for(j = 0; j < size; j++) {
    sprintf(str, "%iK", j);
    k = wabi_binary_leaf_new_from_cstring(&vm, str);
    sprintf(str, "%iV", j);
    v = wabi_binary_leaf_new_from_cstring(&vm, str);
    m = wabi_map_assoc(&vm, m, (wabi_val) k, (wabi_val) v);
  }

  for(j = 0; j < size; j++) {
    sprintf(str, "%iK", j);
    k = wabi_binary_leaf_new_from_cstring(&vm, str);
    sprintf(str, "%iH", j);
    v = wabi_binary_leaf_new_from_cstring(&vm, str);
    m = wabi_map_assoc(&vm, m, (wabi_val) k, (wabi_val) v);
  }

  for(j = 0; j < size; j++) {
    sprintf(str, "%iK", j);
    k = wabi_binary_leaf_new_from_cstring(&vm, str);
    sprintf(str, "%iH", j);
    v0 = wabi_binary_leaf_new_from_cstring(&vm, str);
    v = (wabi_binary_leaf) wabi_map_get(m, (wabi_val) k);
    ck_assert_int_eq(wabi_cmp((wabi_val) v, (wabi_val) v0), 0);
  }
}
END_TEST


START_TEST(assoc_order)
{
  wabi_map m, m0;
  wabi_binary_leaf k, v;

  m = wabi_map_empty(&vm);
  m0 = m;

  k = wabi_binary_leaf_new_from_cstring(&vm, "key1");
  v = wabi_binary_leaf_new_from_cstring(&vm, "val1");
  m = wabi_map_assoc(&vm, m, (wabi_val) k, (wabi_val) v);
  k = wabi_binary_leaf_new_from_cstring(&vm, "key2");
  v = wabi_binary_leaf_new_from_cstring(&vm, "val2");
  m = wabi_map_assoc(&vm, m, (wabi_val) k, (wabi_val) v);

  k = wabi_binary_leaf_new_from_cstring(&vm, "key2");
  v = wabi_binary_leaf_new_from_cstring(&vm, "val2");
  m0 = wabi_map_assoc(&vm, m0, (wabi_val) k, (wabi_val) v);
  k = wabi_binary_leaf_new_from_cstring(&vm, "key1");
  v = wabi_binary_leaf_new_from_cstring(&vm, "val1");
  m0 = wabi_map_assoc(&vm, m0, (wabi_val) k, (wabi_val) v);

  ck_assert_int_eq(wabi_cmp((wabi_val) m, (wabi_val) m0), 0);
}
END_TEST

START_TEST(iter)
{
  wabi_map m;
  wabi_val k, v;
  wabi_map_iter_t iter;
  wabi_map_entry entry;

  m  = wabi_map_empty(&vm);
  wabi_map_iterator_init(&iter, m);

  ck_assert_ptr_null(wabi_map_iterator_current(&iter));

  k = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "1");
  v = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, "one");
  m = wabi_map_assoc(&vm, m, k, v);
  wabi_map_iterator_init(&iter, m);

  entry = wabi_map_iterator_current(&iter);

  ck_assert_ptr_nonnull(wabi_map_iterator_current(&iter));
  ck_assert_int_eq(wabi_cmp((wabi_val) WABI_MAP_ENTRY_KEY(entry), k), 0);

  wabi_map_iterator_next(&iter);
  entry = wabi_map_iterator_current(&iter);

  ck_assert_ptr_null(wabi_map_iterator_current(&iter));
}
END_TEST

START_TEST(iter_hash)
{
  char str[100];
  wabi_map m;
  wabi_val k, v;
  wabi_map_iter_t iter;
  wabi_map_entry entry;
  int len = 360;

  m  = wabi_map_empty(&vm);
  for(int j = 0; j < len; j++) {
    sprintf(str, "K%i", j);
    k = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, str);
    sprintf(str, "V%i", j);
    v = (wabi_val) wabi_binary_leaf_new_from_cstring(&vm, str);
    m = wabi_map_assoc(&vm, m, k, v);
  }
  wabi_map_iterator_init(&iter, m);
  int cnt = 0;
  while((entry = wabi_map_iterator_current(&iter))) {
    cnt++;
    wabi_map_iterator_next(&iter);
  }
  ck_assert_int_eq(cnt, len);
}
END_TEST

Suite *
map_suite(void)
{
  Suite *s;
  TCase *tc_core;

  s = suite_create("Map");

  /* Core test case */
  tc_core = tcase_create("Core");
  tcase_add_checked_fixture(tc_core, setup, teardown);

  tcase_add_test(tc_core, assoc_empty);
  tcase_add_test(tc_core, assoc_many);
  tcase_add_test(tc_core, assoc_update);
  tcase_add_test(tc_core, assoc_order);
  tcase_add_test(tc_core, iter);
  tcase_add_test(tc_core, iter_hash);
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
