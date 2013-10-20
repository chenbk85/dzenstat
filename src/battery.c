/* dzenstat module for battery information
 */

#include "battery.h"
#include "config/battery.h"
#include "config/global.h"
#include <stdio.h>
#include <time.h>
#include <math.h>

/* function declaration */
static int update(void);
static int term(void);

/* variable declaration */
static char path_charge_now[BUFLEN];
static char path_charge_full[BUFLEN];
static char path_charge_full_design[BUFLEN];
static char path_current_now[BUFLEN];
static char path_capacity[BUFLEN];
static char path_status[BUFLEN];
static char *dy;
static int charge_now, charge_full_design, current_now, capacity;
static int h, m, s;
static bool discharging;

int
battery_init(Module *mod)
{
	mod->update = update;
	mod->term = term;
	dy = mod->display;

	snprintf(path_charge_now, BUFLEN, "%s/charge_now", path);
	snprintf(path_charge_full, BUFLEN, "%s/charge_full", path);
	snprintf(path_charge_full_design, BUFLEN, "%s/charge_full_design", path);
	snprintf(path_current_now, BUFLEN, "%s/current_now", path);
	snprintf(path_capacity, BUFLEN, "%s/capacity", path);
	snprintf(path_status, BUFLEN, "%s/status", path);

	/* initial update */
	return update();
}

static int
update(void)
{
	FILE *f;
	double hours;

	/* battery state */
	f = fopen(path_status, "r");
	if (f == NULL) {
		wrlog("Failed to open file: %s\n", path_status);
		return -1;
	}
	discharging = (fgetc(f) == 'D');
	fclose(f);

	/* get current charge if discharging or calculating from design capacity */
	if (use_acpi_real_capacity || discharging) {
		f = fopen(path_charge_now, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", path_charge_now);
			return -1;
		}
		fscanf(f, "%d", &charge_now);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", path_charge_now);
			return -1;
		}
		fclose(f);
	}

	/* calculate from design capacity */
	if (use_acpi_real_capacity) {
		/* full charge */
		f = fopen(path_charge_full_design, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", path_charge_full_design);
			return -1;
		}
		fscanf(f, "%d", &charge_full_design);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", path_charge_now);
			return -1;
		}
		fclose(f);

		/* charge percentage left */
		capacity = (double) charge_now / charge_full_design * 100;
	}
	
	/* calculate from current capacity */
	else {
		f = fopen(path_capacity, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", path_capacity);
			return -1;
		}
		fscanf(f, "%d", &capacity);
		if (ferror(f)) {
			wrlog("Failed to read file: %s\n", path_capacity);
			fclose(f);
			return -1;
		}
		fclose(f);
	}

	/* calculate time left (if not charging) */
	if (discharging) {
		/* usage */
		f = fopen(path_current_now, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", path_current_now);
			return -1;
		}
		fscanf(f, "%d", &current_now);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", path_current_now);
			return -1;
		}
		fclose(f);

		/* time left */
		hours = (double) charge_now / (double) current_now;
		h = (int) hours;
		m = (int) (fmod(hours, 1) * 60);
		s = (int) (fmod((fmod(hours, 1) * 60), 1) * 60);
	}

	/* assemble output */
	if (!discharging) {
		snprintf(dy, DISPLEN,
				"^fg(#%X)%d%% ^i(%s/battery_%03d.xbm)^fg()",
				colour_bat, capacity, path_icons, capacity/10*10);
	} else {
		snprintf(dy, DISPLEN,
				"^fg(#%X)%d%% ^i(%s/battery_%03d.xbm)^fg() "
				"^fg(#%X)%dh %02dm %02ds^fg()",
				colour(capacity), capacity, path_icons,
				capacity/10*10, colour_hl, h, m, s);
	}
	return 0;
}

static int
term(void)
{
	/* TODO */
	return 0;
}

