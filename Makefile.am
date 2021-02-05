SUBDIRS = src . test
ACLOCAL_AMFLAGS = -I m4
# EXTRA_CFLAGS=-fPIC -Wall -Wextra -O3 -fomit-frame-pointer

test: src/wabi
	src/wabivm test/test.wabi

.PHONY: test
