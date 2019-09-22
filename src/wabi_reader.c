#define wabi_reader_c

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_binary.h"
#include "wabi_pair.h"
#include "wabi_atomic.h"
#include "wabi_symbol.h"
#include "wabi_reader.h"

#define WABI_READER_STACK_SIZE 4096

#define IS_WS(c) ((c) == ' ' || (c) == '\n' || (c) == '\t')

typedef struct wabi_reader_struct {
  wabi_vm vm;
  wabi_val stack;
  char* rest;
  char* cur;
} wabi_reader_t;

typedef wabi_reader_t* wabi_reader;


inline static char*
wabi_reader_ws(char* c)
{
  while(IS_WS(*c)) {
    *c++;
  }
  return c;
}

wabi_reader
wabi_reader_init(wabi_reader reader, wabi_vm vm)
{
  reader->vm = vm;
  reader->stack = wabi_cons(vm, wabi_nil(vm), wabi_nil(vm));
  reader->rest = "";

  return reader;
}

wabi_reader
wabi_reader_feed(wabi_reader reader, char* data)
{
  int l0, l1;
  char* new;

  l0 = strlen(reader->rest);
  l1 = strlen(data);

  new = (char*) malloc(sizeof(char) * (l0 + l1 + 1));
  *new = (char) 0;
  strcat(new, reader->rest);
  strcat(new, data);
  free(reader->rest);
  reader->rest = new;
  reader->cur = reader->rest;
  return reader;
}


wabi_reader
wabi_reader_run(wabi_reader reader)
{
  while(1) {
    if(
  }
}
