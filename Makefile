CC      = gcc
RM      = rm
MKDIR   = mkdir
HTTPGET = curl
TAR     = tar

VENDOR_DIR = vendor

# CFLAGS  = -g -Wall
LFLAGS =
SRCDIR 		= "src"
TESTDIR 	= "test"
SOURCES		= $(shell ls ${SRCDIR}/*.c)
OBJECTS		= $(SOURCES:.c=.o)
INCLUDES        =
LIBS            =
MAIN_TEST       = wabi_test

TESTS	        = $(shell ls ${TESTDIR}/*.c)
TEST_OBJECTS	= $(TESTS:.c=.o)

CFLAGS  = -g -Wall -foptimize-sibling-calls -O2 $(INCLUDES) $(LIBS)

VERSION = 0.0.1

LIBEV_VERSION = 4.27

LIBEV_URL = "http://dist.schmorp.de/libev/Attic/libev-$(LIBEV_VERSION).tar.gz"

all: $(MAIN_TEST)
	@echo "$(MAIN_TEST) Created."

$(MAIN_TEST): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN_TEST) $(OBJECTS) $(TEST_OBJECTS) $(LFLAGS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(TEST_OBJECTS) $(MAIN) $(MAIN_TEST)
	-$(RM) -r $(VENDOR_DIR)

test: $(MAIN_TEST)
	./$(MAIN_TEST)

$(VENDOR_DIR)/:
	$(MKDIR) $(VENDOR_DIR)

$(VENDOR_DIR)/libev-$(LIBEV_VERSION).tar.gz: $(VENDOR_DIR)
	$(HTTPGET) $(LIBEV_URL) -o $@

$(VENDOR_DIR)/libev-$(LIBEV_VERSION)/: $(VENDOR_DIR)/libev-$(LIBEV_VERSION).tar.gz
	$(TAR) xvf $(VENDOR_DIR)/libev-$(LIBEV_VERSION).tar.gz -C $(VENDOR_DIR)

$(VENDOR_DIR)/libev-$(LIBEV_VERSION)/Makefile: $(VENDOR_DIR)/libev-$(LIBEV_VERSION)
	cd $(VENDOR_DIR)/libev-$(LIBEV_VERSION) && ./configure

$(VENDOR_DIR)/libev-$(LIBEV_VERSION)/libev.la: $(VENDOR_DIR)/libev-$(LIBEV_VERSION)/Makefile
	$(MAKE) -C $(VENDOR_DIR)/libev-$(LIBEV_VERSION) libev.la

.PHONY: all clean run test
