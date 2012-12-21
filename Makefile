all: dzenstat.c
	gcc -Wall -lm -o dzenstat dzenstat.c

debug: dzenstat.c
	gcc -Wall -g -lm -o dzenstat dzenstat.c
