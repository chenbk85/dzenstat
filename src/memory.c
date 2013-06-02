#include "memory.h"
#include "config/memory.h"
#include "config/global.h"
#include <stdio.h>
#include <time.h>
#include <sys/sysinfo.h>

static int update(void);
static int term(void);
static char *dy;

static int used, total, percentage;
static struct sysinfo info;

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
			"RAM:  ^fg(#%X)%d%%  ^fg()(^fg(#%X)%.1fM^fg())",
			colour_hl, percentage,
			colour(100-percentage), used/1024.0);

	return 0;
}

static int
term(void)
{
	/* TODO */
	return 0;
}

