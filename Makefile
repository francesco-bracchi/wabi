CC = gcc
RM = rm
# CFLAGS  = -g -Wall
LFLAGS =
LIBS =
SRCDIR 		= "src"
TESTDIR 	= "test"
SOURCES		= $(shell ls ${SRCDIR}/*.c)
OBJECTS		= $(SOURCES:.c=.o)
INCLUDES        = -I $(SRCDIR) -I $(TESTDIR)
MAIN            = wabi

TESTS	        = $(shell ls ${TESTDIR}/*.c)
TEST_OBJECTS	= $(TESTS:.c=.o)
TEST            = test/run

CFLAGS  = -g -Wall -O2 $(INCLUDES)
VERSION = "0.0.1"


all: $(MAIN)
	@echo "$(MAIN) Created."

$(MAIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

%(TEST): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJECTS) $(TEST_OBJECTS) $(LFLAGS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(TEST_OBJECTS) $(MAIN)

run: $(MAIN)
	./$(MAIN)

.PHONY: all clean run
