#ifndef wabi_delim_h

#define wabi_delim_h

#include "wabi_value.h"
#include "wabi_env.h"
#include "wabi_vm.h"


typedef struct wabi_delim_control_struct {
} wabi_delim_control_t;

typedef wabi_delim_control_t* wabi_delim_control;

void
wabi_delim_builtins(const wabi_vm vm, const wabi_env env);


#endif
