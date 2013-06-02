/* Simple system monitor; meant to be used in combination with dzen2.
 * Written by ayekat on a rainy afternoon in december 2012.
 */

#include "dzenstat.h"
#include "config/global.h"
#include "modules.h"

/* variables (TODO replace some by modules) */
struct tm *date;
struct timeval longdelay;
time_t rawtime;
bool interrupted;
fd_set fds;

/* seperator icons */
char lsep[BUFLEN], lfsep[BUFLEN], rsep[BUFLEN], rfsep[BUFLEN];

void
pollEvents(void)
{
	int s;

	longdelay.tv_sec = 0;
	longdelay.tv_usec = 900000;  /* 900000 µs = 900 ms = 0.9 s */

	do {
		/* add file_descriptors */
		FD_ZERO(&fds);
		FD_SET(modules[4].fd, &fds);
		FD_SET(modules[2].fd, &fds);

		/* wait for activity */
		s = select(FD_SETSIZE, &fds, NULL, NULL, &longdelay);

		/* handle return value */
		if (s < 0)  /* error */
			return;
		if (!s)     /* timeout */
			return;

		/* network */
		if (FD_ISSET(modules[2].fd, &fds)) {
			/* clear data in buffer */
			modules[2].interrupt();
		}

		/* sound */
		if (FD_ISSET(modules[4].fd, &fds)) {
			modules[4].interrupt();
		}

	} while (longdelay.tv_usec > 0);
}

unsigned int
colour(int val)
{
	return (unsigned int)
	       ((int) ((1.0-val*val*val/1000000.0)*255)<<16)+
	       ((int) ((1.0-(val-100)*(val-100)*(val-100)*(-1)/1000000.0)*255)<<8)+
	       51;
}

void
die(void)
{
	exit(EXIT_FAILURE);
}

void
display(void)
{
	while (!interrupted) {
		updateDate();
		pollEvents();

		if (modules[0].update() < 0)
			printf("RAM:ERROR");
		else
			printf("%s", modules[0].display);
		printf("   ^fg(#%X)%s^fg()   ", colour_sep, rsep);

		if (modules[1].update() < 0)
			printf("CPU:ERROR");
		else
			printf("%s", modules[1].display);
		printf("   ^fg(#%X)%s^fg()   ", colour_sep, rsep);

		if (modules[2].update() < 0)
			printf("NET:ERROR");
		else
			printf("%s", modules[2].display);
		printf("   ^fg(#%X)%s^fg()   ", colour_sep, rsep);

		if (modules[3].update() < 0)
			printf("BAT:ERROR");
		else
			printf("%s", modules[3].display);
		printf("   ^fg(#%X)%s^fg()   ", colour_sep, lsep);

		printf("%s", modules[4].display);
		printf("   ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
				colour_medium_bg, lfsep, colour_medium_bg, colour_medium);

		/* date */
		printf("%d^fg()^i(%s/glyph_japanese_1.xbm)^fg(#%X) ",
				date->tm_mon+1, path_icons, colour_medium);
		printf("%d^fg()^i(%s/glyph_japanese_7.xbm)^fg(#%X) ",
				date->tm_mday, path_icons, colour_medium);
		printf("(^i(%s/glyph_japanese_%d.xbm))",
				path_icons, date->tm_wday);
		printf("  ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
				colour_light_bg, lfsep, colour_light_bg, colour_light);

		/* time */
		printf("%02d:%02d",
				date->tm_hour, date->tm_min);
		printf("  ^bg()^fg()");

		/* end */
		printf("\n");
	}
}

void
init(void)
{
	int i;

	/* force line buffering */
	setvbuf(stdout, NULL, _IOLBF, 1024);

	for (i = 0; i < sizeof(modules)/sizeof(Module); i++) {
		modules[i].init(&modules[i]);
	}

	/* define separator icons */
	snprintf(rfsep, BUFLEN, "^i(%s/glyph_2B80.xbm)", path_icons);
	snprintf(rsep, BUFLEN, "^i(%s/glyph_2B81.xbm)", path_icons);
	snprintf(lfsep, BUFLEN, "^i(%s/glyph_2B82.xbm)", path_icons);
	snprintf(lsep, BUFLEN, "^i(%s/glyph_2B83.xbm)", path_icons);

	/* register signal handler */
	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);
}

void
sig_handle(int sig)
{
	/* we greatfully complete a loop instead of heartlessly jumping out */
	interrupted = true;
}

void
updateDate(void)
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
	updateDate(); /* needed for logging */
	interrupted = false;
	init();
	display();
	wrlog("received shutdown signal ...\n");
	return EXIT_SUCCESS;
}

