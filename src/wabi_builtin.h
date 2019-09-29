#ifndef wabi_builtin_h

#define wabi_builtin_h

wabi_env
wabi_builtin_stdenv(wabi_vm vm);


/*** this function doesn't belong to this header ***/
void
wabi_builtin_load(wabi_vm vm, wabi_env env, char* data);

#endif
