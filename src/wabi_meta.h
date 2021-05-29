#ifndef wabi_meta_h

#define wabi_meta_h

#include "wabi_value.h"
#include "wabi_vm.h"
#include "wabi_cont.h"

typedef struct wabi_meta_struct
{
  wabi_word next;
  wabi_word tag;
  wabi_word cont;
} wabi_meta_t;


typedef wabi_meta_t* wabi_meta;

#define WABI_META_SIZE wabi_sizeof(wabi_meta_t)

// TODO: empty is a specific meta, do not use NULL
static const wabi_meta wabi_meta_empty = NULL;

static inline wabi_meta
wabi_meta_push(const wabi_vm vm,
               const wabi_meta next,
               const wabi_val tag,
               const wabi_cont cont)
{
  wabi_meta meta;

  meta = (wabi_meta) wabi_vm_alloc(vm, WABI_META_SIZE);
  if(vm->ert) return NULL;

  meta->next = (wabi_word) next;
  meta->cont = (wabi_word) cont;
  meta->tag = (wabi_word) tag;
  WABI_SET_TAG(meta, wabi_tag_cont_prompt);
  return meta;

}

static inline wabi_meta
wabi_meta_pop(wabi_meta meta) {
  return (wabi_meta) WABI_WORD_VAL(meta->next);
}

static inline wabi_val
wabi_meta_tag(wabi_meta meta)
{
  return (wabi_val) meta->tag;
}

static inline wabi_cont
wabi_meta_cont(wabi_meta meta)
{
  return (wabi_cont) meta->cont;
}

#endif
