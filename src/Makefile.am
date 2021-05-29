noinst_LTLIBRARIES = \
	libwabi.la

libwabi_la_SOURCES = \
	wabi_word.h \
	wabi_value.h \
	wabi_binary.h \
	wabi_binary.c \
	wabi_number.h \
	wabi_atom.h \
	wabi_symbol.h \
	wabi_list.h \
	wabi_hash.h \
	wabi_hash.c \
	wabi_cmp.h \
	wabi_cmp.c \
	wabi_map.h \
	wabi_map.c \
	wabi_vector.h \
	wabi_vector.c \
	wabi_env.h \
	wabi_env.c \
	wabi_place.h \
	wabi_place.c \
	wabi_delim.h \
	wabi_combiner.h \
	wabi_combiner.c \
	wabi_queue.h \
	wabi_system.h \
	wabi_system.c \
	wabi_collect.h \
	wabi_collect.c \
	wabi_store.h \
	wabi_store.c \
	wabi_io.h \
	wabi_io.c \
	wabi_cont.h \
	wabi_cont.c \
	wabi_meta.h \
	wabi_builtin.c \
	wabi_builtin.h \
	wabi_vm.h \
	wabi_vm.c \
	wabi_pr.h \
	wabi_pr.c \
	wabi_reader.h \
	wabi_reader.c

libwabi_la_LIBADD=$(LIBEV) -lm -lpthread

bin_PROGRAMS = wabivm
wabivm_SOURCES = wabi_main.c
wabivm_LDADD = libwabi.la
wabivm_LTLIBADD = libwabi.la
