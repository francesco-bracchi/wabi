#ifndef wabi_assert_h

#define wabi_assert_h

void
wabi_assert_init();

void
wabi_assert_store(const char* fun, int res);

int
wabi_assert_results();

#define ASSERT(e) wabi_assert_store(__func__, e);

#endif
