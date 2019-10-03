#ifndef wabi_error_h

#define wabi_error_h

typedef enum wabi_error_type_enum {
  wabi_error_none = 0,
  wabi_error_nomem = 1,
  wabi_error_type_mismatch = 2,
  wabi_error_bindings = 3,
  wabi_error_division_by_zero = 4,
  wabi_error_unbound_name = 5,
  wabi_error_other = 255
} wabi_error_type;

#endif
