# m (math):  for fmod())
# mpdclient: for connecting to mpd
all: dzenstat.c
	gcc -Wall -lm -lmpdclient -o dzenstat dzenstat.c

debug: dzenstat.c
	gcc -Wall -g -lm -lmpdclient -o dzenstat dzenstat.c
