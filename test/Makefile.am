TESTS = \
	check_store \
	check_binary \
	check_map

check_PROGRAMS = \
	check_store \
	check_binary \
	check_map

check_store_SOURCES = check_store.c
check_store_LDADD = ../src/libwabi.la
check_store_CFLAGS = @CHECK_CFLAGS@ -lcheck

check_binary_SOURCES = check_binary.c
check_binary_LDADD = ../src/libwabi.la
check_binary_CFLAGS = @CHECK_CFLAGS@ -lcheck

check_map_SOURCES = check_map.c
check_map_LDADD = ../src/libwabi.la
check_map_CFLAGS = @CHECK_CFLAGS@ -lcheck
