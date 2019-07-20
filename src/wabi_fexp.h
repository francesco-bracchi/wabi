#ifndef wabi_fexp_h

#define wabi_fexp_h


typedef struct wabi_fexp_struct {
  wabi_env static_env;
  wabi_env dynamic_env;
  wabi_val arguments;
  wabi_val body;
} wabi_fexp_t;


typedef wabi_word_t wabi_fexp_builtin



#endif
