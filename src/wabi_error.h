#ifndef wabi_error_h

#define wabi_error_h

typedef enum wabi_error_type_enum {
  wabi_error_none = 0,
  wabi_error_nomem = 1,
  wabi_error_type_mismatch = 2,
  wabi_error_bindings = 3,
  wabi_error_division_by_zero = 4,
  wabi_error_unbound_name = 5,
  wabi_error_no_prompt = 6,
  wabi_error_already_defined = 7,
  wabi_error_out_of_range = 8,
  wabi_error_read = 9,
  wabi_error_file_not_found = 10,
  wabi_error_other = 11,
  wabi_error_timeout = 12,
  wabi_error_done = 13
} wabi_error_type;


static inline char const*
wabi_error_name(wabi_error_type e) {
  switch(e) {
  case wabi_error_none:
    return "NO ERROR";
  case wabi_error_nomem:
    return "OUT OF MEMORY";
  case wabi_error_type_mismatch:
    return "TYPE MISMATCH";
  case wabi_error_bindings:
    return "BINDING ERROR";
  case wabi_error_division_by_zero:
    return "DIVISION BY ZERO";
  case wabi_error_unbound_name:
    return "UNBOUND NAME";
  case wabi_error_no_prompt:
    return "NO PROMPT";
  case wabi_error_other:
    return "UNKNOWN ERROR";
  case wabi_error_timeout:
    return "TIME OUT";
  case wabi_error_done:
    return "DONE";
  case wabi_error_already_defined:
    return "SYMBOL ALREADY DEFINED";
  default:
    return "OTHER";
  }
}

#endif
