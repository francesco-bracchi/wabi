CC = gcc
RM = rm
CFLAGS  = -g -Wall -O0
LFLAGS =
LIBS =
SRCDIR = "src"
SOURCES	= $(shell ls ${SRCDIR}/*.c)
OBJECTS	= $(SOURCES:.c=.o)

MAIN = wabi
VERSION = "0.0.1"


all: $(MAIN)
	@echo "$(MAIN) Created."

$(MAIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(MAIN)

.PHONY: all clean
