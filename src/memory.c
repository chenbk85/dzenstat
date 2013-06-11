#include "memory.h"
#include "config/memory.h"
#include "config/global.h"
#include <stdio.h>
#include <time.h>
#include <sys/sysinfo.h> /* for alternative approach, see below */

static int update(void);
static int term(void);
static char *dy;

static int used, total, percentage;

int
memory_init(Module *mod)
{
	mod->update = update;
	mod->term = term;
	dy = mod->display;

	/* initial update */
	return update();
}

static int
update(void)
{
	FILE *f;
	int i;

	/* alternative approach, that does however not respect cache memory:
	sysinfo info;
	sysinfo(&info);
	int stotal = info.totalram;
	int sused = stotal - info.freeram - info.bufferram;
	*/

	/* open file */
	f = fopen(path, "r");
	if (f == NULL) {
		wrlog("Could not open file: %s\n", path);
		return -1;
	}

	/* calculate */
	fscanf(f, "MemTotal: %d kB\n", &total);
	fscanf(f, "MemFree: %d kB\n", &i);
	used = total-i;
	fscanf(f, "Buffers: %d kB\n", &i);
	used -= i;
	fscanf(f, "Cached: %d kB\n", &i);
	used -= i;
	percentage = used*100 / total;
	fclose(f);

	/* update display */
	snprintf(dy, DISPLEN,
			"^fg(#%06X)%d%%^fg() ^i(%s/memory.xbm) ^fg(#%X)%.1fM^fg()",
			colour(100-percentage), percentage, path_icons, colour_hl,
			used/1024.0);

	return 0;
}

static int
term(void)
{
	/* nothing */

	return 0;
}

