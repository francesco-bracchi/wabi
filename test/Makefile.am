TESTS = \
	check_store \
	check_binary \
	check_map \
	check_cmp \
	check_hash \
	check_env

check_PROGRAMS = \
	check_store \
	check_binary \
	check_map \
	check_cmp \
	check_hash \
	check_env

check_store_SOURCES = check_store.c
check_store_LDADD = ../src/libwabi.la
check_store_CFLAGS = @CHECK_CFLAGS@ -lcheck

check_binary_SOURCES = check_binary.c
check_binary_LDADD = ../src/libwabi.la
check_binary_CFLAGS = @CHECK_CFLAGS@ -lcheck

check_map_SOURCES = check_map.c
check_map_LDADD = ../src/libwabi.la
check_map_CFLAGS = @CHECK_CFLAGS@ -lcheck

check_cmp_SOURCES = check_cmp.c
check_cmp_LDADD = ../src/libwabi.la
check_cmp_CFLAGS = @CHECK_CFLAGS@ -lcheck

check_hash_SOURCES = check_hash.c
check_hash_LDADD = ../src/libwabi.la
check_hash_CFLAGS = @CHECK_CFLAGS@ -lcheck

check_env_SOURCES = check_env.c
check_env_LDADD = ../src/libwabi.la
check_env_CFLAGS = @CHECK_CFLAGS@ -lcheck
