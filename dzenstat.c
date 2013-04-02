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
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <regex.h>
#include <signal.h>
#include <dirent.h>



#define NUM_MAX_CORES 8 /* maximum number of CPUs */
#define NUMIFS 10       /* maximum number of network interfaces */
#define BUFLEN 128      /* length for buffers */

#define C_RED    "FF3333"
#define C_GREEN  "33FF33"
#define C_YELLOW "EEEE33"

typedef struct {
	char *path_charge_now, *path_charge_full, *path_charge_full_design,
	     *path_current_now, *path_capacity, *path_status;
	int h, m, s;
	int charge_now, charge_full, charge_full_design, current_now, capacity;
	bool discharging;
	char display[BUFLEN];
} Battery;

typedef struct {
	char *path_temperature, *path_usage;
	int temperature;
	int usage[NUM_MAX_CORES], busy_last[NUM_MAX_CORES], idle_last[NUM_MAX_CORES];
	char display[BUFLEN];
} CPU;

typedef struct {
	char name[BUFLEN];
	char ip[BUFLEN];
	bool active;
	int quality;
} NetworkInterface;

typedef struct {
	int max, vol, line;
	bool mute;
	char display[BUFLEN];
	char const* path;
} Sound;

/* function declarations */
static void clean(void);
static void die(char const *format, ...);
static void display(void);
static void init(void);
static void initBattery(void);
static void initCPU(void);
static void initSound(void);
static void sig_handle(int sig);
static void updateBattery(void);
static void updateColour(char *str, double val);
static void updateCPU(void);
static void updateDate(void);
static void updateNetwork(void);
static void updateSound(void);

/* variables */
static Battery bat;
static CPU cpu;
static Sound snd;
static struct tm *date;
static struct timespec delay;
static time_t rawtime;
static char *icons_path;
static NetworkInterface *netifs[NUMIFS];

/* seperator icons */
static char lsep[BUFLEN], lfsep[BUFLEN], rsep[BUFLEN], rfsep[BUFLEN];

/* load user configuration */
#include "config.h"

static void
clean(void)
{
	// TODO
}

static void
die(char const *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(EXIT_FAILURE);
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

		// network:
		// TODO
		//printf("   ^fg(#%s)%s^fg()   ", colour_sep, rsep);

		// MPD:
		// TODO
		//printf("   ^fg(#%s)%s^fg()   ", colour_sep, lsep);

		// battery:
		printf("%s", bat.display);
		printf("   ^fg(#%s)%s^fg()   ", colour_sep, lsep);

		// volume:
		printf("%s", snd.display);
		printf("   ^fg(#%s)%s^bg(#%s)^fg(#%s)  ",
				colour_medium_bg, lfsep, colour_medium_bg, colour_medium);

		// date:
		printf("%d^fg()^i(%s/glyph_japanese_1.xbm)^fg(#%s) ",
				date->tm_mon+1, icons_path, colour_medium);
		printf("%d^fg()^i(%s/glyph_japanese_7.xbm)^fg(#%s) ",
				date->tm_mday, icons_path, colour_medium);
		printf("(^i(%s/glyph_japanese_%d.xbm))",
				icons_path, date->tm_wday);
		printf("  ^fg(#%s)%s^bg(#%s)^fg(#%s)  ",
				colour_light_bg, lfsep, colour_light_bg, colour_light);

		// time:
		printf("%02d:%02d",
				date->tm_hour, date->tm_min);
		printf("  ^bg()^fg()");

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
	delay.tv_nsec = 450000000; // = 450000 µs = 450 ms = 0.1s
	initBattery();
	initCPU();
	initSound();

	// icons:
	icons_path = malloc(strlen(getenv("PWD"))+strlen("/icons")+1);
	sprintf(icons_path, "%s/icons", getenv("PWD"));

	sprintf(rfsep, "^i(%s/glyph_2B80.xbm)", icons_path);
	sprintf(rsep, "^i(%s/glyph_2B81.xbm)", icons_path);
	sprintf(lfsep, "^i(%s/glyph_2B82.xbm)", icons_path);
	sprintf(lsep, "^i(%s/glyph_2B83.xbm)", icons_path);

	// register signal handler:
	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);
}

static void
initBattery(void)
{
	int dirlen = strlen(battery_path)+2; // +2 for terminating null and slash
	bat.path_charge_now = malloc(dirlen+strlen("charge_now"));
	sprintf(bat.path_charge_now, "%s/charge_now", battery_path);
	bat.path_charge_full = malloc(dirlen+strlen("charge_full"));
	sprintf(bat.path_charge_full, "%s/charge_full", battery_path);
	bat.path_charge_full_design = malloc(dirlen+strlen("charge_full_design"));
	sprintf(bat.path_charge_full_design, "%s/charge_full_design", battery_path);
	bat.path_current_now = malloc(dirlen+strlen("current_now"));
	sprintf(bat.path_current_now, "%s/current_now", battery_path);
	bat.path_capacity = malloc(dirlen+strlen("capacity"));
	sprintf(bat.path_capacity, "%s/capacity", battery_path);
	bat.path_status = malloc(dirlen+strlen("status"));
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
	snd.path = "/proc/asound/SB/codec#0";
	regex_t regex[3];
	char const* regex_string[3] = {
		"^Node 0x[[:xdigit:]]+ \\[Audio Output\\]",
		"^Amp-Out caps:",
		"^Amp-Out vals:" };
	char buf[BUFLEN];
	int i;

	// compile first regex for finding right node:
	for (i = 0; i < 3; ++i)
		if (regcomp(&regex[i], regex_string[i], REG_EXTENDED))
			die("Could not compile regex: %s\n", regex_string[i]);

	// open file from where we read lines:
	FILE* f;
	if ((f = fopen(snd.path, "r")) == NULL)
		die("Could not open file: %s\n", snd.path);

	// find line with relevant information:
	int reti;
	for (i = 0; i < 2; ++i) {
		while (true) {
			if (fscanf(f, "%[^\n]\n", buf) < 0) {
				die("Reached end of file: %s\n", snd.path);
				break;
			}
			reti = regexec(&regex[i], buf, 0, NULL, 0);
			if (!reti) break;
			else if (reti == REG_NOMATCH) {
				++snd.line;
				continue;
			} else {
				regerror(reti, &regex[i], buf, sizeof(buf));
				die("Regex match failed: %s\n", buf);
			}
		};
	}
	// we're done with regex:
	regfree(&regex[0]);
	regfree(&regex[1]);
	regfree(&regex[2]);

	// get maximum volume:
	sscanf(buf, "%*s %*s ofs=%x%*[^\n]\n", &snd.max);
	snd.line++; // we assume that current volume is on the next line
	fclose(f);

}

static void
sig_handle(int sig)
{
	clean();
	exit(EXIT_SUCCESS);
}

static void
updateBattery(void)
{
	// prevent from updating too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update) return;
	next_update = time(NULL) + update_interval;

	FILE *f;

	// battery state:
	if ((f = fopen(bat.path_status, "r")) == NULL)
		die("Failed to open file: %s\n", bat.path_status);
	bat.discharging = (fgetc(f) == 'D');
	fclose(f);

	// capacity:
	if (use_acpi_real_capacity) {
		if ((f = fopen(bat.path_charge_now, "r")) == NULL)
			die("Failed to open file: %s\n", bat.path_charge_now);
		fscanf(f, "%d", &bat.charge_now);
		fclose(f);
		if ((f = fopen(bat.path_charge_full_design, "r")) == NULL)
			die("Failed to open file: %s\n", bat.path_charge_full_design);
		fscanf(f, "%d", &bat.charge_full_design);
		fclose(f);
		bat.capacity = 100*(double)bat.charge_now / (double)bat.charge_full_design;
	} else {
		if ((f = fopen(bat.path_capacity, "r")) == NULL)
			die("Failed to open file: %s\n", bat.path_capacity);
		fscanf(f, "%d", &bat.capacity);
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

// TODO return a numeric value instead
static void
updateColour(char *str, double val)
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
	if (time(NULL) < next_update) return;
	next_update = time(NULL) + update_interval;

	int i;
	int busy_tot, idle_tot, busy_diff, idle_diff, usage;
	int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

	// usage (TODO the values are somewhat wrong, figure out why):
	FILE *f;
	if ((f = fopen(cpu.path_usage, "r")) == NULL)
		die("Failed to open file: %s\n", cpu.path_usage);
	fscanf(f, "%*[^\n]\n"); // ignore first line
	for (i = 0; i < num_cpus && i < NUM_MAX_CORES; i++) {
		fscanf(f, "%*[^ ] %d %d %d %d %d %d %d %d %d %d%*[^\n]\n",
				&user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal,
				&guest, &guest_nice);
		busy_tot = user+nice+system+irq+softirq+steal+guest+guest_nice;
		busy_diff = busy_tot - cpu.busy_last[i];
		idle_tot = idle + iowait;
		idle_diff = idle_tot - cpu.idle_last[i];
		usage = idle_diff + busy_diff;
		cpu.usage[i] = (busy_diff*1000+5)/10/usage;
		cpu.busy_last[i] = busy_tot;
		cpu.idle_last[i] = idle_tot;
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
	for (i = 0; i < num_cpus && i < NUM_MAX_CORES; i++)
		sprintf(cpu.display, "%s[%2d%%] ", cpu.display, cpu.usage[i]);
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
	FILE *f;
	struct ifreq ifr[NUMIFS];
	struct ifconf ifc;
	int sd, i, j=0, ifnum, addr;

	// prevent from updating too often:
	static clock_t next_update = 0;
	if (time(NULL) < next_update) return;
	next_update = time(NULL) + update_interval;

	// clean old interfaces:
	for (i = 0; i < NUMIFS; ++i)
		if (netifs[i])
			free(netifs[i]);

	// create a socket where we can use ioctl on it to retrieve interface info:
	if ((sd = socket(PF_INET, SOCK_DGRAM, 0)) <= 0)
		die("Failed: socket(PF_INET, SOCK_DGRAM, 0)\n");

	// set 'storage' pointer and maximum 'storage space':
	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_ifcu.ifcu_buf = (caddr_t) ifr;

	// get interface info from the socket created above:
	if (ioctl(sd, SIOCGIFCONF, &ifc) != 0)
		die("Failed: ioctl(sd, SIOCGIFCONF, &ifc)\n");
	
	// get number of found interfaces:
	ifnum = ifc.ifc_len / sizeof(struct ifreq);

	for (i = 0; i < ifnum; ++i) {
		if (ifr[i].ifr_addr.sa_family != AF_INET)
			continue;

		if (!ioctl(sd, SIOCGIFADDR, &ifr[i])) { // get address
			// create a new NetworkInterface entity to assign information to it:
			netifs[j] = malloc(sizeof(NetworkInterface));

			// name:
			snprintf(netifs[j]->name, BUFLEN-1, "%s", ifr[i].ifr_name);

			// address:
			addr = ((struct sockaddr_in *) (&ifr[i].ifr_addr))->sin_addr.s_addr;
			snprintf(netifs[j]->ip, BUFLEN-1, "%d.%d.%d.%d",
					 addr      & 0xFF, (addr>> 8) & 0xFF,
					(addr>>16) & 0xFF, (addr>>24) & 0xFF);

			// state (UP/DOWN):
			netifs[j]->active = (!ioctl(sd, SIOCGIFFLAGS, &ifr[i]) &&
					ifr[i].ifr_flags & IFF_UP);

			// quality (if wlan):
			if (!strcmp(netifs[j]->name, "wlan0")) {
				if ((f = fopen("/proc/net/wireless", "r")) == NULL)
					die("Failed to open file: /proc/net/wireless\n");
				fscanf(f, "%*[^\n]\n%*[^\n]\n%*s %*d %d.%*s",
						&netifs[j]->quality);
			}

			++j;
		}
	}
	close(sd);
}

static void
updateSound(void)
{
	FILE* f;
	int i, l, r;

	// open file from where we read lines:
	if ((f = fopen(snd.path, "r")) == NULL)
		die("Could not open file: %s\n", snd.path);

	// read relevant line into buffer (ignoring preceding ones):
	for (i = 0; i <= snd.line; ++i)
		fscanf(f, "%*[^\n]\n");
	fscanf(f, "%*s %*s [%x %x", &l, &r);
	fclose(f);

	// get volume:
	snd.mute = l&0x80 && r&&0x80;
	snd.vol = ((snd.mute?l-0x80:l)+(snd.mute?r-0x80:r))*50/snd.max;

	// update output:
	if (snd.mute)
		sprintf(snd.display, "^fg(#%s)^i(%s/volume_m.xbm)^fg() [%d%%]",
				C_RED, icons_path, snd.vol);
	else
		sprintf(snd.display, "^fg(#%s)^i(%s/volume_%d.xbm) ^fg(#%s)[%d%%]^fg()",
				C_GREEN, icons_path, snd.vol/34, colour_hl, snd.vol);
}

int
main(int argc, char **argv)
{
	init();
	display();
	return 0;
}

