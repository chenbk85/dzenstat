all: dwmzen.c
	gcc -Wall -lm -o dwmzen dwmzen.c

debug: dwmzen.c
	gcc -Wall -g -lm -o dwmzen dwmzen.c
