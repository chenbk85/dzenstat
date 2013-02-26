/* Simple system monitor; meant to be used in combination with dzen2.
 * Written by ayekat on a rainy afternoon in december 2012.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <ifaddrs.h>

#define NUM_MAX_CPUS 8

typedef struct {
	char *path_charge_now, *path_charge_full, *path_charge_full_design,
	     *path_current_now, *path_capacity, *path_status;
	int h, m, s;
	int charge_now, charge_full, charge_full_design, current_now, capacity;
	bool discharging;
	char display[128];
} Battery;

typedef struct {
	char *path_temperature, *path_usage;
	int temperature;
	int usage[NUM_MAX_CPUS], busy_last[NUM_MAX_CPUS], idle_last[NUM_MAX_CPUS];
	char display[128];
} CPU;

typedef struct {
	char display[128];
} Network;

/* function declarations */
static void die(char const* format, ...);
static void display(void);
static void init(void);
static void initBattery(void);
static void initCPU(void);
static void initNetwork(void);
static void updateBattery(void);
static void updateColour(char* str, double val);
static void updateCPU(void);
static void updateDate(void);
static void updateNetwork(void);

/* variables */
static Battery bat;
static CPU cpu;
static Network net;
static struct tm *date;
static struct timespec delay;
static time_t rawtime;

/* seperator icons */
static char lsep[100], lfsep[100], rsep[100], rfsep[100];

/* load user configuration */
#include "config.h"

static void
die(char const *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(-1);
}

static void
display(void)
{
	while (true) {
		updateBattery();
		updateCPU();
		updateDate();

		// CPU:
		printf("%s  ^fg(#444444)%s^fg()  ", cpu.display, rsep);

		// IP:
		printf("%s  ^fg(#444444)%s^fg()  ", net.display, rsep);

		// battery:
		printf("%s  ^fg(#444444)%s^fg()  ", bat.display, lsep);

		// date:
		printf(" ^fg(#FFFFFF)%d^fg()^i(%s/glyph_japanese_1.xbm)",
				date->tm_mon+1, icons_path);
		printf(" ^fg(#FFFFFF)%d^fg()^i(%s/glyph_japanese_7.xbm) ",
				date->tm_mday, icons_path);
		printf("(^i(%s/glyph_japanese_%d.xbm)) ",
				icons_path, date->tm_wday);
		printf(" ^fg(#444444)%s^fg()", lfsep);

		// time:
		printf("^bg(#444444)  ^fg(#FFFFFF)%02d:%02d^fg()  ^bg()",
				date->tm_hour, date->tm_min);

		// end & sleep:
		printf("\n");
		nanosleep(&delay, NULL);
	}
}

static void
init(void)
{
	setvbuf(stdout, NULL, _IOLBF, 1024); // force line buffering
	delay.tv_sec = 0;
	delay.tv_nsec = 450000000; // = 100000 µs = 100 ms = 0.1s
	initBattery();
	initCPU();

	// seperator icons:
	sprintf(rfsep, "^i(%s/glyph_2B80.xbm)", icons_path);
	sprintf(rsep, "^i(%s/glyph_2B81.xbm)", icons_path);
	sprintf(lfsep, "^i(%s/glyph_2B82.xbm)", icons_path);
	sprintf(lsep, "^i(%s/glyph_2B83.xbm)", icons_path);
}

static void
initBattery(void)
{
	int dirlen = strlen(battery_path)+2; // +2 for terminating null and slash
	bat.path_charge_now = (char*)malloc(dirlen+strlen("charge_now"));
	sprintf(bat.path_charge_now, "%s/charge_now", battery_path);
	bat.path_charge_full = (char*)malloc(dirlen+strlen("charge_full"));
	sprintf(bat.path_charge_full, "%s/charge_full", battery_path);
	bat.path_charge_full_design = (char*)malloc(dirlen+strlen("charge_full_design"));
	sprintf(bat.path_charge_full_design, "%s/charge_full_design", battery_path);
	bat.path_current_now = (char*)malloc(dirlen+strlen("current_now"));
	sprintf(bat.path_current_now, "%s/current_now", battery_path);
	bat.path_capacity = (char*)malloc(dirlen+strlen("capacity"));
	sprintf(bat.path_capacity, "%s/capacity", battery_path);
	bat.path_status = (char*)malloc(dirlen+strlen("status"));
	sprintf(bat.path_status, "%s/status", battery_path);
}

static void
initCPU(void)
{
	// temperature:
	cpu.path_temperature = (char*)malloc(strlen(cpu_temperature_path)+1);
	sprintf(cpu.path_temperature, "%s", cpu_temperature_path);

	// usage:
	cpu.path_usage = "/proc/stat";
}

static void
initNetwork(void)
{
	// TODO
}

static void
updateBattery(void)
{
	FILE *f;

	// prevent from updating too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return;
	next_update = time(NULL) + update_interval;

	// battery state:
	if ((f = fopen(bat.path_status, "r")) == NULL)
		die("Failed to open file: %s\n", bat.path_status);
	bat.discharging = (fgetc(f) == 'D');
	fclose(f);

	// capacity:
	if (use_acpi_real_capacity) {
		if ((f = fopen(bat.path_charge_full_design, "r")) == NULL)
			die("Failed to open file: %s\n", bat.path_charge_full_design);
		fscanf(f, "%d", &(bat.charge_full_design));
		if (ferror(f)) {
			fclose(f);
			die("Failed to read file: %s\n", bat.path_charge_full_design);
		}
		fclose(f);
		bat.capacity = (double)bat.charge_now / (double)bat.charge_full_design;
	} else {
		if ((f = fopen(bat.path_capacity, "r")) == NULL)
			die("Failed to open file: %s\n", bat.path_capacity);
		fscanf(f, "%d", &(bat.capacity));
		if (ferror(f)) {
			fclose(f);
			die("Failed to read file: %s\n", bat.path_capacity);
		}
		fclose(f);
	}

	// prevent recalculating time if charging (not displayed):
	if (!bat.discharging)
		return;

	// time left:
	if ((f = fopen(bat.path_charge_now, "r")) == NULL)
		die("Failed to open file: %s\n", bat.path_charge_now);
	fscanf(f, "%d", &(bat.charge_now));
	if (ferror(f)) {
		fclose(f);
		die("Failed to read file: %s\n", bat.path_charge_now);
	}
	fclose(f);
	if ((f = fopen(bat.path_current_now, "r")) == NULL)
		die("Failed to open file: %s\n", bat.path_current_now);
	fscanf(f, "%d", &bat.current_now);
	if (ferror(f)) {
		fclose(f);
		die("Failed to read file: %s\n", bat.path_current_now);
	}
	fclose(f);

	double hours = (double)bat.charge_now / (double)bat.current_now;
	bat.h = (int)hours;
	bat.m = (int)(fmod(hours, 1) * 60);
	bat.s = (int)(fmod((fmod(hours, 1) * 60), 1) * 60);

	// assemble output:
	char colour[7];
	updateColour(colour, bat.capacity/100.0);
	if (!bat.discharging)
		sprintf(bat.display,
				"^fg(#5577FF)%d%% ^i(%s/glyph_battery_%02d.xbm)^fg()",
				bat.capacity, icons_path, bat.capacity/10*10);
	else {
		sprintf(bat.display,
				"^fg(#%s)%d%% ^i(%s/glyph_battery_%02d.xbm)^fg()",
				colour, bat.capacity, icons_path, bat.capacity/10*10);
		sprintf(bat.display, "%s  ^fg(#FFFFFF)%dh %dm %ds^fg()",
				bat.display, bat.h, bat.m, bat.s);
	}
}

static void
updateColour(char* str, double val)
{
	double r = 1-val*val*val;
	double g = 1-(val-1)*(val-1)*(val-1)*(-1);
	double b = 0.2;
	sprintf(str, "%02X%02X%02X", (int)(r*255), (int)(g*255), (int)(b*255));
}

static void
updateCPU(void)
{
	FILE *f;
	int i;
	int busy_now, user, nice, system, idle_now;
	int busy, idle, total;

	// prevent updating temperature too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return;
	next_update = time(NULL) + update_interval;

	// CPU usage:
	if ((f = fopen(cpu.path_usage, "r")) == NULL)
		die("Failed to open file: %s\n", cpu.path_usage);
	fscanf(f, "%*[^\n]"); // ignore first line
	for (i = 0; i < num_cpus && i < NUM_MAX_CPUS; i++) {
		fscanf(f, "%*s %d %d %d %d%*[^\n]", &user, &nice, &system, &idle_now);
		busy_now = user+nice+system;
		busy = busy_now - cpu.busy_last[i];
		idle = idle_now - cpu.idle_last[i];
		total = idle+busy;
		cpu.usage[i] = (busy*1000+5)/10/total;
		cpu.busy_last[i] = busy_now;
		cpu.idle_last[i] = idle_now;
	}

	// temperature:
	if ((f = fopen(cpu.path_temperature, "r")) == NULL)
		die("Failed to open file: %s\n", cpu.path_temperature);
	fscanf(f, "%d", &(cpu.temperature));
	if (ferror(f)) {
		fclose(f);
		die("Failed to read file: %s\n", cpu.path_temperature);
	}
	fclose(f);
	cpu.temperature /= 1000;

	// assemble display:
	sprintf(cpu.display, "^i(%s/glyph_cpu.xbm)  ^fg(#FFFFFF)", icons_path);
	for (i = 0; i < num_cpus && i < NUM_MAX_CPUS; i++)
		sprintf(cpu.display, "%s[%d%%] ", cpu.display, cpu.usage[i]);
	sprintf(cpu.display, "%s^fg() %d°C", cpu.display, cpu.temperature);
}

static void
updateDate(void)
{
	time(&rawtime);
	date = localtime(&rawtime);
}

static void
updateNetwork(void)
{
	// TODO
}

int
main(int argc, char **argv)
{
	init();
	display();
	return 0;
}

