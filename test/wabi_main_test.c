#define wabi_main_test_c

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "wabi_assert.h"
#include "wabi_cmp_test.h"
#include "wabi_map_test.h"
#include "wabi_reader_test.h"


int main(int argc, char** argv)
{
  wabi_assert_init();
  wabi_map_test();
  wabi_cmp_test();
  wabi_reader_test();

  wabi_assert_results();
}
