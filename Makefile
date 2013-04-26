NAME = dzenstat
LFLAGS = -lm -lasound
CFLAGS = -Wall -Wpedantic -std=gnu99 -g

# m (math):  for fmod())
dzenstat: dzenstat.c
	gcc ${CFLAGS} ${LFLAGS} -o ${NAME} dzenstat.c

alsa: alsa.c
	gcc ${CFLAGS} -lasound alsa.c

inotify: inotify.c
	gcc ${CFLAGS} inotify.c

netmon: netmon.c
	gcc ${CFLAGS} netmon.c

mpd: mpd.c
	gcc ${CFLAGS} -lmpdclient mpd.c

