#ifndef wabi_collect_h

#define wabi_collect_h

#include "wabi_vm.h"
#include "wabi_value.h"

wabi_word*
wabi_copy_val(const wabi_vm vm, const wabi_val src);


void
wabi_collect(const wabi_vm vm);



static inline void
wabi_copy_val_size(const wabi_vm vm, const wabi_val obj, const wabi_size size)
{
  wordcopy(vm->stor.heap, obj, size);
  vm->stor.heap += size;
}


// reverse order, because the first word contains references to a kind of tail
// like pairs, the tail part is the first, byte, the same for continuations.
static inline void
wabi_collect_val_size(const wabi_vm vm, const wabi_val obj, const wabi_size size)
{
  wabi_size cnt;
  wabi_word* pos;
  wabi_word tag;

  tag = WABI_WORD_TAG(*obj);
  cnt = 0;
  pos = obj + (size - 1);
  while(cnt < size) {
    *pos = (wabi_word) wabi_copy_val(vm, (wabi_word*) WABI_WORD_VAL(*pos));
    cnt++;
    pos--;
  }
  WABI_SET_TAG(obj, tag);
  vm->stor.scan+= size;
}

#endif
