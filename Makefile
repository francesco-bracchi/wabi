CC = gcc
RM = rm
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

VERSION = "0.0.1"

all: $(MAIN_TEST)
	@echo "$(MAIN_TEST) Created."

$(MAIN_TEST): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN_TEST) $(OBJECTS) $(TEST_OBJECTS) $(LFLAGS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(TEST_OBJECTS) $(MAIN) $(MAIN_TEST)

test: $(MAIN_TEST)
	./$(MAIN_TEST)

.PHONY: all clean run test
