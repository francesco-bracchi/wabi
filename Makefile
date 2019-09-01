CC      	= gcc
RM      	= rm
MKDIR   	= mkdir
HTTPGET 	= curl
TAR     	= tar

VENDOR_DIR 	= vendor

VERSION 	= 0.0.1
LIBEV_VERSION 	= 4.27
LIBEV_URL 	= "http://dist.schmorp.de/libev/Attic/libev-$(LIBEV_VERSION).tar.gz"

LDLAGS  	= -static
SRCDIR 		= "src"
TESTDIR 	= "test"
SOURCES		= $(shell ls ${SRCDIR}/*.c)
OBJECTS		= $(SOURCES:.c=.o)
INCLUDES        =
LIBS            = -L $(VENDOR_DIR)/libev-$(LIBEV_VERSION)/.libs -l ev
MAIN_TEST       = wabi_test

TESTS	        = $(shell ls ${TESTDIR}/*.c)
TEST_OBJECTS	= $(TESTS:.c=.o)

CFLAGS  = -g -Wall -foptimize-sibling-calls -O2 $(INCLUDES) $(LIBS)


all: $(MAIN_TEST)
	@echo "$(MAIN_TEST) Created."

$(MAIN_TEST): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN_TEST) $(OBJECTS) $(TEST_OBJECTS) $(LFLAGS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(TEST_OBJECTS) $(MAIN) $(MAIN_TEST)
	-$(MAKE) -C $(VENDOR_DIR)/libev-$(LIBEV_VERSION) clean

distclean: clean
	-$(RM) $(VENDOR_DIR)

test: $(MAIN_TEST)
	./$(MAIN_TEST)

$(VENDOR_DIR)/libev-$(LIBEV_VERSION).tar.gz:
	@$(MKDIR) -p $(VENDOR_DIR)
	$(HTTPGET) $(LIBEV_URL) -o $@

$(VENDOR_DIR)/libev-$(LIBEV_VERSION)/Makefile: $(VENDOR_DIR)/libev-$(LIBEV_VERSION).tar.gz
	$(TAR) xvf $(VENDOR_DIR)/libev-$(LIBEV_VERSION).tar.gz -C $(VENDOR_DIR)
	cd $(VENDOR_DIR)/libev-$(LIBEV_VERSION) && ./configure

$(VENDOR_DIR)/libev-$(LIBEV_VERSION)/.libs/libev.a: $(VENDOR_DIR)/libev-$(LIBEV_VERSION)/Makefile
	$(MAKE) -C $(VENDOR_DIR)/libev-$(LIBEV_VERSION)

.PHONY: all clean run test
