CC=gcc
LDFLAGS=-lm -lasound
CFLAGS=-Wall -Wpedantic -std=gnu99 -g
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dzenstat

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $? -o $@

clean:
	rm $(OBJECTS)

mpd: tmp/mpd.c
	gcc ${CFLAGS} -lmpdclient tmp/mpd.c

xorg: tmp/xorg.c
	gcc ${CFLAGS} -lX11 tmp/xorg.c

