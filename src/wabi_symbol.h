# ifndef wabi_symbol_h

# define wabi_symbol_h

# include "wabi_object.h"
# include "wabi_vm.h"

wabi_obj
wabi_symbol(wabi_vm vm, wabi_obj str);

wabi_obj
wabi_intern(wabi_vm vm, wabi_obj str);

#endif
