/* dzenstat module for CPU stats.
 */

#include "cpu.h"
#include "config/global.h"
#include "config/cpu.h"
#include <time.h>
#include <stdio.h>

typedef struct {
	int busy_last, idle_last;
	int load;
} Core;

static int num_cores;
static Core **cores;
static char const *path_temp;
static char *dy;

static int temperature;

static int cpu_update(void);
static int cpu_term(void);

int
cpu_init(Module *mod)
{
	int i;
	FILE *f;
	char buf[BUFLEN];
	dy = mod->display;

	/* load */
	mod->update = cpu_update;
	mod->term = cpu_term;

	/* check if file exists */
	f = fopen(path_load, "r");
	if (f == NULL)
		return -1;

	/* determine number of CPU cores */
	for (i = 0;; ++i) {
		fscanf(f, "%*[^\n]\n%s", buf);
		if (strncmp(buf, "cpu", 3) != 0)
			break;
	}
	num_cores = i;

	/* create Core entities */
	cores = malloc(num_cores * sizeof(Core *));
	for (i = 0; i < num_cores; ++i)
		cores[i] = malloc(sizeof(Core));
	
	/* temperature */

	/* determine temperature path */
	path_temp = NULL;
	for (i = 0; i < sizeof(paths_temp); ++i) {
		f = fopen(paths_temp[i], "r");
		if (f != NULL) {
			fclose(f);
			path_temp = paths_temp[i];
			break;
		}
	}

	return 0;
}

static int
cpu_update(void)
{
	FILE *f;
	int i;
	char w[16], e[16];
	int busy_tot, idle_tot, busy_diff, idle_diff, usage;
	int user, nice, system, idle, iowait, irq, softirq, steal, guest,guest_nice;

	/* prevent from updating too often */
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return 0;
	next_update = time(NULL) + update_interval;

	/* usage */
	f = fopen(path_load, "r");
	if (f == NULL) {
		wrlog("Failed to open file: %s\n", path_load);
		return -1;
	}
	fscanf(f, "%*[^\n]\n"); /* ignore first line */
	for (i = 0; i < num_cores; i++) {
		fscanf(f, "%*[^ ] %d %d %d %d %d %d %d %d %d %d%*[^\n]\n",
				&user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal,
				&guest, &guest_nice);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", path_load);
			return -1;
		}

		/* calculate usage */
		busy_tot = user+nice+system+irq+softirq+steal+guest+guest_nice;
		busy_diff = busy_tot - cores[i]->busy_last;
		idle_tot = idle + iowait;
		idle_diff = idle_tot - cores[i]->idle_last;
		usage = idle_diff + busy_diff;
		cores[i]->load = (busy_diff*1000+5)/10/usage;
		cores[i]->busy_last = busy_tot;
		cores[i]->idle_last = idle_tot;
	}
	fclose(f);

	/* temperature */
	f = fopen(path_temp, "r");
	if (f == NULL) {
		wrlog("Failed to open file: %s\n", path_temp);
		return -1;
	}
	fscanf(f, "%d", &(temperature));
	if (ferror(f)) {
		fclose(f);
		wrlog("Failed to read file: %s\n", path_temp);
		return -1;
	}
	fclose(f);
	temperature /= 1000;

	/* assemble output */
	snprintf(w, 13, "^fg(#%X)", colour_warn);
	snprintf(e, 13, "^fg(#%X)", colour_err);
	dy[0] = 0;
	snprintf(dy, DISPLEN, "^i(%s/glyph_cpu.xbm)  ^fg(#%X)",
			path_icons, colour_hl);
	for (i = 0; i < num_cores; i++)
		snprintf(dy+strlen(dy), DISPLEN-strlen(dy), "[%2d%%] ",
				cores[i]->load);
	snprintf(dy+strlen(dy), DISPLEN-strlen(dy),
			"^fg() %s%d%s°C",
			temperature>=temp_crit ? e : temperature>=temp_high ? w:"",
			temperature, temperature>=temp_high ? "^fg()" : "");

	return 0;
}

static int
cpu_term(void)
{
	/* TODO */
	return 0;
}

