#ifndef wabi_test_h

#define wabi_test_h

int (*wabi_test_list[100]) (int x, int y);

wabi_test_list = {
                  &test_bin,
}

#endif
