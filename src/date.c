/* dzenstat module for time & date.
 */

#include "date.h"
#include "config/global.h"
#include <time.h>
#include <stdio.h>
#include <string.h>

/* function declaration */
static int update(void);
static int term(void);
static char *dy;

int
date_init(Module *mod)
{
	mod->update = update;
	mod->term = term;
	mod->noborder = true;
	dy = mod->display;
	return update();
}

static int
update(void)
{
	time_t rawtime;
	struct tm *date;

	time(&rawtime);
	date = localtime(&rawtime);

	/* date */
	dy[0] = 0;
	sprintf(dy+strlen(dy), "  ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
			colour_medium_bg, lfsep, colour_medium_bg, colour_medium);
	sprintf(dy+strlen(dy), "%d^fg()^i(%s/glyph_japanese_1.xbm)^fg(#%X) ",
			date->tm_mon+1, path_icons, colour_medium);
	sprintf(dy+strlen(dy), "%d^fg()^i(%s/glyph_japanese_7.xbm)^fg(#%X) ",
			date->tm_mday, path_icons, colour_medium);
	sprintf(dy+strlen(dy), "(^i(%s/glyph_japanese_%d.xbm))",
			path_icons, date->tm_wday);

	/* time */
	sprintf(dy+strlen(dy), " ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
			colour_light_bg, lfsep, colour_light_bg, colour_light);
	sprintf(dy+strlen(dy), "%02d:%02d",
			date->tm_hour, date->tm_min);
	sprintf(dy+strlen(dy), "  ^bg()^fg()");

	return 0;
}

static int
term(void)
{
	/* nothing */

	return 0;
}

