#ifndef wabi_vm_h

#define wabi_vm_h


#include "wabi_value.h"
#include "wabi_system.h"

int
wabi_vm_run(wabi_system sys,
            wabi_val ctrl,
            wabi_val env);

#endif
