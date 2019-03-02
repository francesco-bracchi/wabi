/**
 * error handling
 */

#ifndef wabi_err_h

#define wabi_err_h

#define WABI_ERROR_NOMEM 1
#define WABI_ERROR_ALLOC_FAIL 2
#define WABI_ERROR_TYPE_MISMATCH 3
#define WABI_ERROR_TAG_NOT_FOUND 4

int wabi_errno;

#endif
