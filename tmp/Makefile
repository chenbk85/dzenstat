CC=gcc
CFLAGS=-Wall -Wpedantic -std=gnu99 -g

alsa: alsa.c
	$(CC) $(CFLAGS) $? -lasound

inotify: inotify.c
	$(CC) $(CFLAGS) $?

mpd: mpd.c
	$(CC) $(CFLAGS) $? -lmpdclient

netmon: netmon.c
	$(CC) $(CFLAGS) $?

xorg: xorg.c
	$(CC) $(CFLAGS) $? -lX11

pthread: pthread.c
	$(CC) $(CFLAGS) $? -lpthread

idle: idle.c
	$(CC) $(CFLAGS) $? -lX11 -lXss

