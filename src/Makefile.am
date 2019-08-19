noinst_LTLIBRARIES = \
	libwabi.la

libwabi_la_SOURCES = \
	wabi_word.h \
	wabi_value.h \
	wabi_system.h \
	wabi_system.c \
	wabi_store.h \
	wabi_store.c \
	wabi_io.h \
	wabi_io.c

libwabi_la_LIBADD=$(LIBEV)

bin_PROGRAMS = wabi
wabi_SOURCES = wabi_main.c
wabi_LTLIBADD = libwabi.la
