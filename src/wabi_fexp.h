#ifndef wabi_fexp_h

#define wabi_fexp_h


typedef struct wabi_fexp_struct {
  wabi_val static_env;
  wabi_val dynamic_env;
  wabi_val arguments;
  wabi_val body;
} wabi_fexp_t;

typedef wabi_fexp_t* wabi_fexp;

#endif
