#ifndef wabi_vm_h

#define wabi_vm_h


#include "wabi_value.h"
#include "wabi_system.h"
#include "wabi_env.h"
#include "wabi_cont.h"

typedef wabi_val wabi_control;

int
wabi_vm_run(wabi_control control, wabi_env env, wabi_store store, wabi_cont cont);

#endif
