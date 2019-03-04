/**
 * error handling
 */

#define wabi_err_c

int wabi_errno;


char* wabi_errmsg[4] =
  {"",
   "Failed to allocate memory",
   "Type Error",
   "Tag not found"
  };

char* wabi_err_msg(int errno)
{
  return wabi_errmsg[errno];
}
