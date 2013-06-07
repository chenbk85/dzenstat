CC=gcc
LDLIBS=-lm -lasound -lmpdclient
LDFLAGS=
CFLAGS=-DHOST_$(shell hostname) -Wall -Wpedantic -std=gnu99 -g
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dzenstat

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDLIBS) $(OBJECTS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $? -o $@

purge: clean
	rm -f $(EXECUTABLE)

clean:
	rm -f $(OBJECTS)

