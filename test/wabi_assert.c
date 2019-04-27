#define wabi_assert_c

#include <stdio.h>
#include "time.h"

long int total, failures;
clock_t start, end;


void
wabi_assert_init()
{
  total = 0;
  failures = 0;
  start = clock();
}


void
wabi_assert_store(const char* str, int line, int res)
{
  total++;
  if(res) {
    printf("\x1b[32mSucceded:\x1b[0m %s:%i\n", str, line);
    return;
  }
  failures++;
  printf("\x1b[31mFailed:  \x1b[0m %s:%i\n", str, line);
}


int
wabi_assert_results() {
  end = clock();
  double cpu_time = ((double) (end - start));
  printf("Total tests: %li\n", total);
  printf("Failed tests: %li\n", failures);
  printf("Success tests: %li\n", total - failures);
  printf("time: %f\n", cpu_time);
  return !failures;
}
