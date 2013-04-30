NAME = dzenstat
LFLAGS = -lm -lasound
CFLAGS = -Wall -Wpedantic -std=gnu99 -g

dzenstat: src/dzenstat.c
	gcc ${CFLAGS} ${LFLAGS} -o ${NAME} src/dzenstat.c

alsa: tmp/alsa.c
	gcc ${CFLAGS} -lasound tmp/alsa.c

inotify: tmp/inotify.c
	gcc ${CFLAGS} tmp/inotify.c

netmon: tmp/netmon.c
	gcc ${CFLAGS} tmp/netmon.c

mpd: tmp/mpd.c
	gcc ${CFLAGS} -lmpdclient tmp/mpd.c

xorg: tmp/xorg.c
	gcc ${CFLAGS} -lX11 tmp/xorg.c

