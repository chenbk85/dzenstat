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

/* macros */
#define LOG_BUFFER_SIZE 256

typedef struct {
	char *path_charge_now, *path_charge_full, *path_charge_full_design,
	     *path_current_now, *path_capacity, *path_status;
	int h, m, s;
	int charge_now, charge_full, charge_full_design, current_now, capacity;
	bool discharging;
} Battery;

typedef struct {
	char *path_temperature, *path_usage;
	int temperature;
	int usage0, usage1;
} CPU;

/* function declarations */
static void die(char const* format, ...);
static void display(void);
static void init(void);
static void initBattery(void);
static void initCPU(void);
static void updateBattery(void);
static void updateCPU(void);
static void updateDate(void);

/* variables */
Battery bat;
CPU cpu;
struct tm *date;
struct timespec delay;
time_t rawtime;

/* load user configuration */
#include "config.h"

void
die(char const *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(-1);
}

void
display(void)
{
	int i;
	while (true) {
		updateBattery();
		updateCPU();
		updateDate();
		printf("%d°C ", cpu.temperature);
		if (bat.discharging)
			printf("%d%% (%dh %dm %ds) ", bat.capacity, bat.h, bat.m, bat.s);
		printf("^fg(#FFFFFF)%02d:%02d^fg():%02d\n",
				date->tm_hour, date->tm_min, date->tm_sec);
		nanosleep(&delay, NULL);
	}
}

void
init(void)
{
	setvbuf(stdout, NULL, _IOLBF, 1024); // force line buffering
	delay.tv_sec = 0;
	delay.tv_nsec = 200000000;
	initBattery();
	initCPU();
}

void
initBattery(void)
{
	// TODO ugly, path names need to be built more dynamically
	bat.path_charge_now = "/sys/class/power_supply/BAT1/charge_now";
	bat.path_charge_full = "/sys/class/power_supply/BAT1/charge_full";
	bat.path_charge_full_design = "/sys/class/power_supply/BAT1/charge_full_design";
	bat.path_current_now = "/sys/class/power_supply/BAT1/current_now";
	bat.path_capacity = "/sys/class/power_supply/BAT1/capacity";
	bat.path_status = "/sys/class/power_supply/BAT1/status";
}

void
initCPU(void)
{
	// TODO ugly, path names need to be built more dynamically
	cpu.path_temperature = "/sys/class/hwmon/hwmon0/device/temp1_input";
	cpu.path_usage = "/proc/stat";
}

void
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
	if (acpi_real_capacity) {
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

	// prevent recalculating time information if charging:
	if (!bat.discharging)
		return;

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

	// time left:
	double hours = (double)bat.charge_now / (double)bat.current_now;
	bat.h = (int)hours;
	bat.m = (int)(fmod(hours, 1) * 60);
	bat.s = (int)(fmod((fmod(hours, 1) * 60), 1) * 60);
}

void
updateCPU(void)
{
	FILE *f;

	if ((f = fopen(cpu.path_usage, "r")) == NULL)
		die("Failed to open file: %s\n", cpu.path_usage);
	
	// prevent from updating temperature too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return;
	next_update = time(NULL) + update_interval;

	if ((f = fopen(cpu.path_temperature, "r")) == NULL)
		die("Failed to open file: %s\n", cpu.path_temperature);
	fscanf(f, "%d", &(cpu.temperature));
	if (ferror(f)) {
		fclose(f);
		die("Failed to read file: %s\n", cpu.path_temperature);
	}
	fclose(f);
	cpu.temperature /= 1000;
}

void
updateDate(void)
{
	time(&rawtime);
	date = gmtime(&rawtime);
}

int
main(int argc, char **argv)
{
	init();
	display();
	return 0;
}

