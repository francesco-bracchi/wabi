SUBDIRS = src .
ACLOCAL_AMFLAGS = -I m4
# EXTRA_CFLAGS=-fPIC -Wall -Wextra -O3 -fomit-frame-pointer

test: src/wabivm
	src/wabivm test.wabi

bench: src/wabivm
	src/wabivm bench.wabi

.PHONY: test bench
