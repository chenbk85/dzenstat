CC=gcc
LDLIBS=-lm -lasound
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

clean:
	rm -f $(OBJECTS)

