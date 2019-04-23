#ifndef wabi_cmp_h

#define wabi_cmp_h

int
wabi_cmp_raw(wabi_val a, wabi_val b);

int
wabi_eq_raw(wabi_val left, wabi_val right);

wabi_val
wabi_eq(wabi_vm vm, wabi_val left, wabi_val right);

wabi_val
wabi_cmp(wabi_vm vm, wabi_val a, wabi_val b);

#endif
