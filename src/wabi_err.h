/**
 * error handling
 */

#ifndef wabi_err_h

#define wabi_err_h

#define WABI_ERROR_NONE 0
#define WABI_ERROR_NOMEM 1
#define WABI_ERROR_ALLOC_FAIL 2
#define WABI_ERROR_TYPE_MISMATCH 3
#define WABI_ERROR_TAG_NOT_FOUND 4
#define WABI_ERROR_INDEX_OUT_OF_RANGE 5
#define WABI_ERROR_HASH_CONFLICT 6
#define WABI_ERROR_EOF_READ 7
#define WABI_ERROR_SYMBOL_TOO_LONG 8
#define WABI_ERROR_UNKNOWN 9
#define WABI_ERROR_REDEFINE_VARIABLE 10
#define WABI_ERROR_BIND 11
#define WABI_ERROR_ENV_LOOKUP 12

char* wabi_err_msg(int errno);

#endif
