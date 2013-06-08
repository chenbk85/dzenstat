/* Simple system monitor; meant to be used in combination with dzen2.
 * Written by ayekat on a rainy afternoon in december 2012.
 */

#include "dzenstat.h"
#include "config/global.h"
#include "modules.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#define die() exit(EXIT_FAILURE)

/* function declarations */
static void init(void);
static void run(void);
static void display(void);
static int poll_events(void);
static void sig_handle(int sig);
static void term(void);
static void update_date(void);

/* variables */
struct timeval longdelay;
bool interrupted;
fd_set fds;

/* variables for date (TODO: transform into module) */
time_t rawtime;
struct tm *date;

/* seperator icons */
char lsep[BUFLEN], lfsep[BUFLEN], rsep[BUFLEN], rfsep[BUFLEN];

unsigned int
colour(int val)
{
	return (unsigned int)
	       ((int) ((1.0-val*val*val/1000000.0)*255)<<16)+
	       ((int) ((1.0-(val-100)*(val-100)*(val-100)*(-1)/1000000.0)*255)<<8)+
	       51;
}

static void
display(void)
{
	int i;

	/* modules */
	for (i = 0; i < sizeof(modules)/sizeof(Module); i++) {
		if (modules[i].hide)
			continue;
		if (i > 0)
			printf("  ^fg(#%X)%s^fg()   ", colour_sep, lsep);
		printf("%s", modules[i].stumbled ? "STUMBLED" : modules[i].display);
	}

	/* date (TODO: transform into module) */
	printf("  ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
			colour_medium_bg, lfsep, colour_medium_bg, colour_medium);
	printf("%d^fg()^i(%s/glyph_japanese_1.xbm)^fg(#%X) ",
			date->tm_mon+1, path_icons, colour_medium);
	printf("%d^fg()^i(%s/glyph_japanese_7.xbm)^fg(#%X) ",
			date->tm_mday, path_icons, colour_medium);
	printf("(^i(%s/glyph_japanese_%d.xbm))",
			path_icons, date->tm_wday);

	/* time */
	printf(" ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
			colour_light_bg, lfsep, colour_light_bg, colour_light);
	printf("%02d:%02d",
			date->tm_hour, date->tm_min);
	printf("  ^bg()^fg()");

	/* end */
	printf("\n");
}

static void
init(void)
{
	int i;

	/* needed for logging */
	update_date();

	/* force line buffering */
	setvbuf(stdout, NULL, _IOLBF, 1024);

	/* define separator icons */
	snprintf(rfsep, BUFLEN, "^i(%s/glyph_2B80.xbm)", path_icons);
	snprintf(rsep, BUFLEN, "^i(%s/glyph_2B81.xbm)", path_icons);
	snprintf(lfsep, BUFLEN, "^i(%s/glyph_2B82.xbm)", path_icons);
	snprintf(lsep, BUFLEN, "^i(%s/glyph_2B83.xbm)", path_icons);

	/* register signal handler */
	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);

	wrlog("initialising ...\n");

	/* initialise modules */
	for (i = 0; i < sizeof(modules)/sizeof(Module); i++) {
		modules[i].stumbled = modules[i].init(&modules[i]) < 0;
	}

	/* initial output */
	display();
}

static int
poll_events(void)
{
	int i, s;

	longdelay.tv_sec = update_interval;
	longdelay.tv_usec = 0;

	do {
		/* add file_descriptors */
		FD_ZERO(&fds);
		for (i = 0; i < sizeof(modules)/sizeof(Module); i++) {
			if (!modules[i].stumbled && modules[i].has_fd)
				FD_SET(modules[i].fd, &fds);
		}

		/* wait for activity */
		s = select(FD_SETSIZE, &fds, NULL, NULL, &longdelay);

		if (s < 0)  /* error */
			return -1;
		if (!s)     /* timeout */
			return 0;

		/* handle event and refresh */
		for (i = 0; i < sizeof(modules)/sizeof(Module); i++) {
			if (modules[i].has_fd && FD_ISSET(modules[i].fd, &fds)
					&& modules[i].interrupt() < 0)
				sprintf(modules[i].display, "ERROR");
		}
		display();
	} while (longdelay.tv_sec > 0 && longdelay.tv_usec > 0);
}

static void
run(void)
{
	int i;

	interrupted = false;
	while (!interrupted) {
		if (poll_events() < 0) {
			wrlog("poll_events(): fatal error\n");
			interrupted = true;
			return;
		}
		update_date(); /* TODO: transform into module */
		for (i = 0; i < sizeof(modules)/sizeof(Module); i++) {
			if (!modules[i].ignore && !modules[i].stumbled
					&& modules[i].update() < 0)
				sprintf(modules[i].display, "ERROR");
		}
		display();
	}
}

static void
sig_handle(int sig)
{
	wrlog("received signal, shutting down ...\n");
	interrupted = true;
}

static void
term(void)
{
	int i;

	for (i = 0; i < sizeof(modules)/sizeof(Module); i++) {
		modules[i].term();
	}
}

/* TODO: transform into module */
static void
update_date(void)
{
	time(&rawtime);
	date = localtime(&rawtime);
}

void
wrlog(char const *format, ...)
{
	va_list args;
	fprintf(stderr, "[%02d:%02d:%02d] dzenstat: ",
			date->tm_hour, date->tm_min, date->tm_sec);
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

int
main(int argc, char **argv)
{
	init();
	run();
	term();

	return EXIT_SUCCESS;
}

