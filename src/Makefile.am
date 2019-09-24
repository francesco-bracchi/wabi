noinst_LTLIBRARIES = \
	libwabi.la

libwabi_la_SOURCES = \
	wabi_word.h \
	wabi_value.h \
	wabi_binary.h \
	wabi_binary.c \
	wabi_number.h \
	wabi_number.c \
	wabi_symbol.h \
	wabi_symbol.c \
	wabi_pair.h \
	wabi_pair.c \
	wabi_hash.h \
	wabi_hash.c \
	wabi_cmp.h \
	wabi_cmp.c \
	wabi_map.h \
	wabi_map.c \
	wabi_env.h \
	wabi_env.c \
	wabi_combiner.h \
	wabi_combiner.c \
	wabi_system.h \
	wabi_system.c \
	wabi_store.h \
	wabi_store.c \
	wabi_io.h \
	wabi_io.c \
	wabi_cont.h \
	wabi_cont.c \
	wabi_builtin.c \
	wabi_builtin.h \
	wabi_vm.h \
	wabi_vm.c \
	wabi_pr.h \
	wabi_pr.c \
	wabi_reader.h \
	wabi_reader.c

libwabi_la_LIBADD=$(LIBEV)

bin_PROGRAMS = wabi
wabi_SOURCES = wabi_main.c
wabi_LTLIBADD = libwabi.la
