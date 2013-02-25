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
	int usage0, usage1;
	char display[128];
} CPU;

/* function declarations */
void die(char const* format, ...);
void display(void);
void init(void);
void initBattery(void);
void initCPU(void);
void updateBattery(void);
void updateColour(int percentage);
void updateCPU(void);
void updateDate(void);

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
	while (true) {
		updateBattery();
		updateCPU();
		updateDate();

		// temperature:
		printf("%d°C ", cpu.temperature);

		// battery:
		printf("%s ", bat.display);
		/*
		if (!bat.discharging)
			printf("^fg(#5577FF)%d%%^fg() ", bat.capacity);
		else
			printf("%d%% (%dh %dm %ds) ",  bat.capacity, bat.h,bat.m,bat.s);
		*/

		// time:
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
	int dirlen = strlen(battery_path)+1; // +1 for terminating null
	bat.path_charge_now = (char*)malloc(dirlen+strlen("charge_now"));
	sprintf(bat.path_charge_now, "%scharge_now", battery_path);
	bat.path_charge_full = (char*)malloc(dirlen+strlen("charge_full"));
	sprintf(bat.path_charge_full, "%scharge_full", battery_path);
	bat.path_charge_full_design = (char*)malloc(dirlen+strlen("charge_full_design"));
	sprintf(bat.path_charge_full_design, "%scharge_full_design", battery_path);
	bat.path_current_now = (char*)malloc(dirlen+strlen("current_now"));
	sprintf(bat.path_current_now, "%scurrent_now", battery_path);
	bat.path_capacity = (char*)malloc(dirlen+strlen("capacity"));
	sprintf(bat.path_capacity, "%scapacity", battery_path);
	bat.path_status = (char*)malloc(dirlen+strlen("status"));
	sprintf(bat.path_status, "%sstatus", battery_path);
}

void
initCPU(void)
{
	// temperature:
	cpu.path_temperature = (char*)malloc(strlen(cpu_temperature_path)+1);
	sprintf(cpu.path_temperature, "%s", cpu_temperature_path);

	// usage:
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
	if (!bat.discharging)
		sprintf(bat.display,
				"^fg(#5577FF)%d%% ^i(%sglyph_battery_%02d.xbm)^fg()",
				bat.capacity, icons_path, bat.capacity/10*10);
	else
		sprintf(bat.display, "%d%% ^i(%sglyph_battery_%02d.xbm)(%dh %dm %ds)",
				bat.capacity, icons_path, bat.capacity/10*10,
				bat.h, bat.m, bat.s);
}

void
updateColour(int percentage)
{
}

void
updateCPU(void)
{
	FILE *f;

	if ((f = fopen(cpu.path_usage, "r")) == NULL)
		die("Failed to open file: %s\n", cpu.path_usage);

	// TODO calculate usage
	
	// prevent updating temperature too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return;
	next_update = time(NULL) + update_interval;

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

