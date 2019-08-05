/* o/\** */
/*  * Here are the various types of supported continuations */
/*  *\/ */

/* #ifndef wabi_cont_h */

/* #define wabi_cont_h */

/* #include "wabi_value.h" */

/* typedef struct wabi_cont_def_struct */
/* { */
/*   wabi_word_t prev; */
/*   wabi_word_t env; */
/*   wabi_word_t pattern; */
/* } wabi_cont_def_t; */

/* typedef wabi_cont_def_t* wabi_cont_def; */


/* typedef struct wabi_cont_if_struct */
/* { */
/*   wabi_word_t prev; */
/*   wabi_word_t env; */
/*   wabi_word_t left; */
/*   wabi_word_t right; */
/* } wabi_cont_def_t; */

/* typedef wabi_cont_if_t* wabi_cont_if; */


/* typedef struct wabi_cont_seq_struct */
/* { */
/*   wabi_word_t prev; */
/*   wabi_word_t env; */
/*   wabi_word_t controls; */
/* } wabi_cont_seq_t; */

/* typedef wabi_cont_seq_t* wabi_cont_seq; */


/* typedef struct wabi_cont_call_struct */
/* { */
/*   wabi_word_t prev; */
/*   wabi_word_t env; */
/*   wabi_word_t arguments; */
/* } wabi_cont_apply_t; */

/* typedef wabi_cont_apply_t* wabi_cont_apply; */


/* typedef struct wabi_cont_call_struct */
/* { */
/*   wabi_word_t prev; */
/*   wabi_word_t env; */
/*   wabi_word_t combiner; */
/* } wabi_cont_apply_to_t; */

/* typedef wabi_cont_call_t* wabi_cont_call; */


/* typedef struct wabi_cont_cons_struct */
/* { */
/*   wabi_word_t prev; */
/*   wabi_word_t env; */
/*   wabi_word_t expressions; */
/* } wabi_cont_rator_t; */

/* typedef wabi_cont_cons_t* wabi_cont_cons; */


/* typedef union wabi_cont_union { */
/*   wabi_word_t prev; */
/*   wabi_cont_def_t def; */
/*   wabi_cont_if_t ifc; */
/*   wabi_cont_seq_t seq; */
/*   wabi_cont_apply_t apply; */
/*   wabi_cont_call_t call; */
/*   wabi_cont_cons_t cons; */
/* } wabi_cont_t; */

/* typedef wabi_cont_t* wabi_cont; */

/* #define WABI_CONT_DEF_SIZE 3 */
/* #define WABI_CONT_IF_SIZE 4 */
/* #define WABI_CONT_SEQ_SIZE 3 */
/* #define WABI_CONT_APPLY_SIZE 4 */

/* #define WABI_CONT_PREVIOUS(cont) (*((wabi_val) cont) & WABI_VALUE_MASK) */
/* #define WABI_CONT_ENV(cont) ((wabi_env) *(((wabi_val) cont) + 1)) */
/* #define WABI_CONT_DEF_PATTERN(cont) (((wabi_cont_def) cont)->pattern) */
/* #define WABI_CONT_IF_LEFT(cont) (((wabi_cont_if) cont)->left) */
/* #define WABI_CONT_IF_RIGHT(cont) (((wabi_cont_if) cont)->right) */

/* static inline void */
/* wabi_cont_def_push(wabi_vm vm, wabi_val pattern) */
/* { */
/*   wabi_cont_def res = (wabi_cont_def) wabi_vm_allocate(vm, WABI_CONT_DEF_SIZE); */
/*   if(res) { */
/*     res->prefix.prev = WABI_TAG_CONT_DEF | (wabi_word_t) vm->continuation; */
/*     res->prefix.env = (wabi_word_t) vm->env; */
/*     res->pattern = (wabi_word_t) pattern; */
/*     vm->continuation = res; */
/*     return; */
/*   } */
/*   vm->errno = WABI_ERROR_NOMEM; */
/* } */


/* static inline void */
/* wabi_cont_if_push(wabi_vm vm, wabi_val left, wabi_val right) */
/* { */
/*   wabi_cont_if res = (wabi_cont_if) wabi_vm_allocate(vm, WABI_CONT_IF_SIZE); */
/*   if(res) { */
/*     res->prefix.prev = WABI_TAG_CONT_IF | (wabi_word_t) vm->continuation; */
/*     res->prefix.env = (wabi_word_t) vm->env; */
/*     res->left = (wabi_word_t) left; */
/*     res->right = (wabi_word_t) right; */
/*     vm->continuation = res; */
/*     return; */
/*   } */
/*   vm->errno = WABI_ERROR_NOMEM; */
/* } */


/* static inline void */
/* wabi_cont_seq_push(wabi_vm vm, wabi_val controls) */
/* { */
/*   wabi_cont_seq res = (wabi_cont_seq) wabi_vm_allocate(vm, WABI_CONT_SEQ_SIZE); */
/*   if(res) { */
/*     res->prefix.prev = WABI_TAG_CONT_SEQ | (wabi_word_t) vm->continuation; */
/*     res->prefix.env = (wabi_word_t) vm->env; */
/*     res->controls = (wabi_word_t) controls; */
/*     vm->continuation = res; */
/*     return; */
/*   } */
/*   vm->errno = WABI_ERROR_NOMEM; */
/* } */


/* static inline void */
/* wabi_cont_apply_push(wabi_vm vm, wabi_val arguments) */
/* { */
/*   wabi_cont_apply res = (wabi_cont_apply) wabi_vm_allocate(vm, WABI_CONT_APPLY_SIZE); */
/*   if(res) { */
/*     res->prefix.prev = WABI_TAG_CONT_APPLY | (wabi_word_t) vm->continuation; */
/*     res->prefix.env = (wabi_word_t) vm->env; */
/*     res->arguments = (wabi_word_t) controls; */
/*     vm->continuation = res; */
/*     return; */
/*   } */
/*   vm->errno = WABI_ERROR_NOMEM; */
/* } */

/* static inline void */
/* wabi_cont_eval_push(wabi_vm vm, wabi_val applicative, wabi_val arguments) */
/* { */
/*   wabi_cont_apply res = (wabi_cont_apply) wabi_vm_allocate(vm, WABI_CONT_APPLY_SIZE); */
/*   if(res) { */
/*     wabi_val nil = wabi_nil(vm); */
/*     if(nil) { */
/*       res->prefix.prev = WABI_TAG_CONT_APPLY | (wabi_word_t) vm->continuation; */
/*       res->prefix.env = (wabi_word_t) vm->env; */
/*       res->applicative = (wabi_word_t) applicative, */
/*       res->arguments = (wabi_word_t) controls; */
/*       res->results = nil; */
/*       vm->continuation = res; */
/*       return; */
/*     } */
/*   } */
/*   vm->errno = WABI_ERROR_NOMEM; */
/* } */
/* static inline wabi_cont */
/* wabi_cont_pop(wabi_vm vm) */
/* { */
/*   wabi_cont res = vm->continuation; */
/*   vm->continuation = (wabi_val) vm->continuation.prefix.prev & TAG_VALUE_MASK; */
/*   return res; */
/* } */

/* #endif */
