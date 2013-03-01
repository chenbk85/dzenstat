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
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>

#define NUM_MAX_CPUS 8
#define NUM_MAX_IFS 8

typedef struct {
	int vol;
	bool mute;
	char display[128];
} Sound;

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
	char* names[NUM_MAX_IFS];
	char* ips[NUM_MAX_IFS];
	char display[128];
} Network;

/* function declarations */
static void die(char const* format, ...);
static void display(void);
static int ifup(char const* iface);
static void init(void);
static void initBattery(void);
static void initCPU(void);
static void initSound(void);
static void updateBattery(void);
static void updateColour(char* str, double val);
static void updateCPU(void);
static void updateDate(void);
static void updateNetwork(void);
static void updateSound(void);

/* variables */
static Battery bat;
static CPU cpu;
static Network net;
static Sound snd;
static struct tm *date;
static struct timespec delay;
static time_t rawtime;
static char* icons_path;

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
		updateNetwork();
		updateSound();

		// CPU:
		printf("%s", cpu.display);
		printf("   ^fg(#%s)%s^fg()   ", colour_sep, rsep);

		// IP:
		printf("%s", net.display);
		printf("   ^fg(#%s)%s^fg()   ", colour_sep, rsep);

		// MPD:
		// TODO
		printf("   ^fg(#%s)%s^fg()   ", colour_sep, lsep);

		// battery:
		printf("%s", bat.display);
		printf("   ^fg(#%s)%s^fg()   ", colour_sep, lsep);

		// volume:
		printf("%s", snd);
		printf("   ^fg(#%s)%s^fg()   ", colour_sep, lsep);

		// date:
		printf("^fg(#FFFFFF)%d^fg()^i(%s/glyph_japanese_1.xbm) ",
				date->tm_mon+1, icons_path);
		printf("^fg(#FFFFFF)%d^fg()^i(%s/glyph_japanese_7.xbm) ",
				date->tm_mday, icons_path);
		printf("(^i(%s/glyph_japanese_%d.xbm))",
				icons_path, date->tm_wday);
		printf("  ^fg(#%s)%s^fg()^bg(#%s)   ",colour_hlbg, lfsep, colour_hlbg);

		// time:
		printf("^fg(#%s)%02d:%02d^fg()",
				colour_hl, date->tm_hour, date->tm_min);
		printf("   ^bg()");

		// end & sleep:
		printf("\n");
		nanosleep(&delay, NULL);
	}
}

static int
ifup(char const* iface)
{
	int i;
	for (i = 0; i < NUM_MAX_IFS && net.names[i] != NULL; ++i)
		if (strcmp(net.names[i], iface) == 0)
			return i;
	return -1;
}

static void
init(void)
{
	setvbuf(stdout, NULL, _IOLBF, 1024); // force line buffering
	delay.tv_sec = 0;
	delay.tv_nsec = 450000000; // = 100000 µs = 100 ms = 0.1s
	initBattery();
	initCPU();
	initSound();

	// icons:
	icons_path = (char*)malloc(strlen(getenv("PWD"))+strlen("/icons")+1);
	sprintf(icons_path, "%s/icons", getenv("PWD"));

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
initSound(void)
{
}

static void
updateBattery(void)
{
	// prevent from updating too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return;
	next_update = time(NULL) + update_interval;

	FILE *f;

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
	if (bat.discharging) {
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
	}

	// assemble output:
	char colour[7];
	updateColour(colour, bat.capacity/100.0);
	if (!bat.discharging)
		sprintf(bat.display,
				"^fg(#4499CC)%d%% ^i(%s/glyph_battery_%02d.xbm)^fg()",
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
	// prevent from updating too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return;
	next_update = time(NULL) + update_interval;

	int i;

	// CPU usage (TODO the values are somewhat wrong, figure out why):
	FILE *f;
	if ((f = fopen(cpu.path_usage, "r")) == NULL)
		die("Failed to open file: %s\n", cpu.path_usage);
	fscanf(f, "%*[^\n]"); // ignore first line

	int busy_now, user, nice, system, idle_now;
	int busy, idle, total;
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

	// assemble output:
	char w[13], e[13];
	sprintf(w, "^fg(#%s)", colour_warn);
	sprintf(e, "^fg(#%s)", colour_err);
	sprintf(cpu.display, "^i(%s/glyph_cpu.xbm)  ^fg(#FFFFFF)", icons_path);
	for (i = 0; i < num_cpus && i < NUM_MAX_CPUS; i++)
		sprintf(cpu.display, "%s[%d%%] ", cpu.display, cpu.usage[i]);
	sprintf(cpu.display, "%s^fg() %s%d%s°C", cpu.display,
			cpu.temperature>=temp_crit ? e : cpu.temperature>=temp_high ? w:"",
			cpu.temperature, cpu.temperature>=temp_high ? "^fg()" : "");
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
	// prevent from updating too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update)
		return;
	next_update = time(NULL) + update_interval;

	FILE* f;
	int i, q;
	char c[7];

	// get list of interfaces:
	struct ifaddrs* ifas;
	struct ifaddrs* ifa;
	getifaddrs(&ifas);
	for (ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {
		// if it's IPv4:
		if (ifa->ifa_addr->sa_family == AF_INET) {
			net.ips[i] = (char*)malloc(INET_ADDRSTRLEN);
			net.names[i] = (char*)malloc(strlen(ifa->ifa_name)+1);
			inet_ntop(AF_INET, &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr,
					net.ips[i], INET_ADDRSTRLEN);
			sprintf(net.names[i], "%s", ifa->ifa_name);
			i++;
		}
	}

	// assemble output (eth):
	if ((i = ifup("eth0")) >= 0) {
		sprintf(net.display, "^i(%s/glyph_eth.xbm)  ^fg(#%s)%s^fg()",
				net.names[i], colour_hl, net.ips[i]);
	}
	// assemble output (wlan):
	else if ((i = ifup("wlan0")) >= 0) {
		if ((f = fopen("/proc/net/wireless", "r")) == NULL)
			die("Failed to open file: /proc/net/wireless\n");
		fscanf(f, "%*[^\n]\n%*[^\n]%*s %*d %d.%*s", &q);
		updateColour(c, q/100.0);
		sprintf(net.display,
				"^fg(#%s)^i(%s/glyph_wifi_%d.xbm)^fg()  ^fg(#%s)%s^fg()",
				c, icons_path, (q-1)/20, colour_hl, net.ips[i]);
	// assemble output (none or other):
	} else
		sprintf(net.display, "no network");
}

static void
updateSound(void)
{
}

int
main(int argc, char **argv)
{
	fprintf(stderr, argv[0]);
	init();
	display();
	return 0;
}

