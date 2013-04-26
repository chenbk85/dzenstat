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
#include <regex.h>
#include <signal.h>
#include <sys/sysinfo.h>
#include <alsa/asoundlib.h>

/* network */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/rtnetlink.h> /* sockaddr_nl */

#define NUMIFS 10       /* maximum number of network interfaces */
#define BUFLEN 128      /* length for buffers */
#define DISPLEN 512     /* length for display buffers (a little longer) */

#define LONGDELAY() \
		static clock_t next_update = 0; \
		if (time(NULL) < next_update) return; \
		next_update = time(NULL) + update_interval;

typedef struct {
	char path_charge_now[BUFLEN], path_charge_full[BUFLEN],
	     path_charge_full_design[BUFLEN], path_current_now[BUFLEN],
	     path_capacity[BUFLEN], path_status[BUFLEN];
	int h, m, s;
	int charge_now, charge_full, charge_full_design, current_now, capacity;
	bool discharging;
} Battery;

typedef struct {
	int busy_last, idle_last;
	int load;
} Core;

typedef struct {
	char const *path_temp, *path_load;
	int temperature;
	Core **cores;
	int num_cores;
} CPU;

typedef struct {
	int used, total, percentage;
	char const *path;
	struct sysinfo info;
} Memory;

typedef struct {
	char name[BUFLEN];
	char ip[BUFLEN];
	bool active;
	int quality;
} NetworkInterface;
int net_fd;
struct sockaddr_nl net_sa;

typedef struct {
	long min, max, vol;
	bool mute;
	char const* path;
	int fd;
	snd_mixer_t *ctl;
	snd_mixer_elem_t *elem;
} Sound;

/* function declarations */
static void pollEvents(void);
static unsigned int colour(int val);
static void die(void);
static void display(void);
static void init(void);
static void initBattery(void);
static void initCPULoad(void);
static void initCPUTemp(void);
static void initMemory(void);
static void initNetwork(void);
static void initSound(void);
static void sig_handle(int sig);
static void updateBattery(void);
static void updateCPU(void);
static void updateCPULoad(void);
static void updateCPUTemp(void);
static void updateDate(void);
static void updateMemory(void);
static void updateNetwork(void);
static void updateSound(void);
static void wrlog(char const *format, ...);

/* variables */
static Battery bat;
static CPU cpu;
static NetworkInterface *netifs[NUMIFS];
static Memory mem;
static Sound snd;
static struct tm *date;
static struct timeval longdelay;
static time_t rawtime;
static bool interrupted;
static fd_set fds;

/* displays & flags */
static char batdisp[DISPLEN]; static bool batflag = false;
static char cpudisp[DISPLEN]; static bool cpuflag = false;
static char memdisp[DISPLEN]; static bool memflag = false;
static char netdisp[DISPLEN]; static bool netflag = false;
static char snddisp[DISPLEN]; static bool sndflag = false;

/* seperator icons */
static char lsep[BUFLEN], lfsep[BUFLEN], rsep[BUFLEN], rfsep[BUFLEN];

/* load user configuration */
#include "config.h"

static void
pollEvents(void)
{
	int s;
	char buf[4096];
	struct iovec iov = { buf, sizeof(buf) };
	struct msghdr msg = { &net_sa, sizeof(net_sa), &iov, 1, NULL, 0, 0 };

	/* wait for activity */
	longdelay.tv_sec = 1;
	longdelay.tv_usec = 0;  /* 450000 µs = 450 ms = 0.45 s */
	s = select(FD_SETSIZE, &fds, NULL, NULL, &longdelay);

	/* handle return value */
	if (s < 0)  /* error */
		return;
	if (!s)     /* timeout */
		return;

	/* network */
	if (FD_ISSET(net_fd, &fds)) {
		/* clear data in buffer */
		recvmsg(net_fd, &msg, 0);
		updateNetwork();
	}

	/* sound */
	if (FD_ISSET(snd.fd, &fds)) {
		/* clear data in buffer */
		snd_mixer_handle_events(snd.ctl);
		updateSound();
	}
}

static unsigned int
colour(int val)
{
	return (unsigned int)
	       ((int) ((1.0-val*val*val/1000000.0)*255)<<16)+
	       ((int) ((1.0-(val-100)*(val-100)*(val-100)*(-1)/1000000.0)*255)<<8)+
	       51;
}

static void
die(void)
{
	exit(EXIT_FAILURE);
}

static void
display(void)
{
	while (!interrupted) {
		updateBattery();
		updateCPU();
		updateDate();
		updateMemory();

		pollEvents();

		/* flags */
		printf("^fg(#%X)", colour_err);
		if (batflag) printf("BAT|");
		if (cpuflag) printf("CPU|");
		if (memflag) printf("MEM|");
		if (netflag) printf("NET|");
		if (sndflag) printf("SND|");
		printf("^fg()   ");

		/* memory */
		printf("%s", memdisp);
		printf("   ^fg(#%X)%s^fg()   ", colour_sep, rsep);

		/* CPU */
		printf("%s", cpudisp);
		printf("   ^fg(#%X)%s^fg()   ", colour_sep, rsep);

		/* network */
		printf("%s", netdisp);

		/* battery */
		printf("%s", batdisp);
		printf("   ^fg(#%X)%s^fg()   ", colour_sep, lsep);

		/* volume */
		printf("%s", snddisp);
		printf("   ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
				colour_medium_bg, lfsep, colour_medium_bg, colour_medium);

		/* date */
		printf("%d^fg()^i(%s/glyph_japanese_1.xbm)^fg(#%X) ",
				date->tm_mon+1, path_icons, colour_medium);
		printf("%d^fg()^i(%s/glyph_japanese_7.xbm)^fg(#%X) ",
				date->tm_mday, path_icons, colour_medium);
		printf("(^i(%s/glyph_japanese_%d.xbm))",
				path_icons, date->tm_wday);
		printf("  ^fg(#%X)%s^bg(#%X)^fg(#%X)  ",
				colour_light_bg, lfsep, colour_light_bg, colour_light);

		/* time */
		printf("%02d:%02d",
				date->tm_hour, date->tm_min);
		printf("  ^bg()^fg()");

		/* end */
		printf("\n");
	}
}

static void
init(void)
{
	/* force line buffering */
	setvbuf(stdout, NULL, _IOLBF, 1024);

	/* initialise modules */
	initBattery();
	initCPULoad();
	initCPUTemp();
	initMemory();
	initNetwork();
	initSound();

	/* clear fd set and add file_descriptors */
	FD_ZERO(&fds);
	FD_SET(snd.fd, &fds);
	FD_SET(net_fd, &fds);

	/* define separator icons */
	snprintf(rfsep, BUFLEN, "^i(%s/glyph_2B80.xbm)", path_icons);
	snprintf(rsep, BUFLEN, "^i(%s/glyph_2B81.xbm)", path_icons);
	snprintf(lfsep, BUFLEN, "^i(%s/glyph_2B82.xbm)", path_icons);
	snprintf(lsep, BUFLEN, "^i(%s/glyph_2B83.xbm)", path_icons);

	/* register signal handler */
	signal(SIGTERM, sig_handle);
	signal(SIGINT, sig_handle);
}

static void
initBattery(void)
{
	snprintf(bat.path_charge_now, BUFLEN, "%s/charge_now", path_bat);
	snprintf(bat.path_charge_full, BUFLEN, "%s/charge_full", path_bat);
	snprintf(bat.path_charge_full_design, BUFLEN, "%s/charge_full_design",
			path_bat);
	snprintf(bat.path_current_now, BUFLEN, "%s/current_now", path_bat);
	snprintf(bat.path_capacity, BUFLEN, "%s/capacity", path_bat);
	snprintf(bat.path_status, BUFLEN, "%s/status", path_bat);
}

static void
initCPULoad(void)
{
	int i;
	FILE *f;
	char buf[BUFLEN];
	cpu.path_load = path_cpu_load;

	/* check if file exists */
	f = fopen(cpu.path_load, "r");
	if (f == NULL) {
		cpu.path_load = NULL;
		return;
	}

	/* determine number of CPU cores */
	for (i = 0;; ++i) {
		fscanf(f, "%*[^\n]\n%s", buf);
		if (strncmp(buf, "cpu", 3) != 0)
			break;
	}
	cpu.num_cores = i;

	/* create Core entities */
	cpu.cores = malloc(cpu.num_cores * sizeof(Core *));
	for (i = 0; i < cpu.num_cores; ++i)
		cpu.cores[i] = malloc(sizeof(Core));
}

static void
initCPUTemp(void)
{
	int i;
	FILE *f;

	/* determine temperature path */
	cpu.path_temp = NULL;
	for (i = 0; i < sizeof(path_cpu_temp); ++i) {
		f = fopen(path_cpu_temp[i], "r");
		if (f != NULL) {
			fclose(f);
			cpu.path_temp = path_cpu_temp[i];
			break;
		}
	}
}

static void
initMemory(void)
{
	mem.path = path_mem;
}

static void
initNetwork(void)
{
	/* prepare fields */
	memset(&net_sa, 0, sizeof(net_sa));
	net_sa.nl_family = AF_NETLINK;
	net_sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

	/* get file descriptor */
	net_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	bind(net_fd, (struct sockaddr *) &net_sa, sizeof(net_sa));

	/* initial update */
	updateNetwork();
}

static void
initSound(void)
{
	/* new */
	int ret = 0;
	snd_mixer_selem_id_t *sid;
	char const *card = "default";
	struct pollfd pfd;

	/* new */

	/* set up control interface */
	ret += snd_mixer_open(&snd.ctl, 0);
	ret += snd_mixer_attach(snd.ctl, card);
	ret += snd_mixer_selem_register(snd.ctl, NULL, NULL);
	ret += snd_mixer_load(snd.ctl);
	if (ret < 0) {
		wrlog("ALSA: Could not open mixer\n");
		die();
	}

	/* get mixer element */
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, "Master");
	snd.elem = snd_mixer_find_selem(snd.ctl, sid);
	if (snd.elem == NULL) {
		wrlog("ALSA: Could not get mixer element\n");
		snd_mixer_detach(snd.ctl, card);
		snd_mixer_close(snd.ctl);
		die();
	}

	/* get poll file descriptor */
	if (snd_mixer_poll_descriptors(snd.ctl, &pfd, 1)) {
		snd.fd = pfd.fd;
	} else {
		wrlog("ALSA: Could not get file descriptor\n");
		die();
	}

	/* get volume range */
	snd_mixer_selem_get_playback_volume_range(snd.elem, &snd.min, &snd.max);

	/* initial update */
	updateSound();
}

static void
sig_handle(int sig)
{
	/* we greatfully complete a loop instead of heartlessly jumping out */
	interrupted = true;
}

static void
updateBattery(void)
{
	FILE *f;
	double hours;

	/* prevent from updating too often */
	LONGDELAY();
	batflag = false;

	/* battery state */
	f = fopen(bat.path_status, "r");
	if (f == NULL) {
		wrlog("Failed to open file: %s\n", bat.path_status);
		batflag = true;
		return;
	}
	bat.discharging = (fgetc(f) == 'D');
	fclose(f);

	/* get current charge if discharging or calculating from design capacity */
	if (use_acpi_real_capacity || bat.discharging) {
		f = fopen(bat.path_charge_now, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", bat.path_charge_now);
			batflag = true;
			return;
		}
		fscanf(f, "%d", &bat.charge_now);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", bat.path_charge_now);
			batflag = true;
			return;
		}
		fclose(f);
	}

	/* calculate from design capacity */
	if (use_acpi_real_capacity) {
		/* full charge */
		f = fopen(bat.path_charge_full_design, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", bat.path_charge_full_design);
			batflag = true;
			return;
		}
		fscanf(f, "%d", &bat.charge_full_design);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", bat.path_charge_now);
			batflag = true;
			return;
		}
		fclose(f);

		/* charge percentage left */
		bat.capacity = 100 * bat.charge_now / bat.charge_full_design;
	}
	
	/* calculate from current capacity */
	else {
		f = fopen(bat.path_capacity, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", bat.path_capacity);
			batflag = true;
			return;
		}
		fscanf(f, "%d", &bat.capacity);
		if (ferror(f)) {
			wrlog("Failed to read file: %s\n", bat.path_capacity);
			batflag = true;
			fclose(f);
			return;
		}
		fclose(f);
	}

	/* calculate time left (if not charging) */
	if (bat.discharging) {
		/* usage */
		f = fopen(bat.path_current_now, "r");
		if (f == NULL) {
			wrlog("Failed to open file: %s\n", bat.path_current_now);
			batflag = true;
			return;
		}
		fscanf(f, "%d", &bat.current_now);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", bat.path_current_now);
			batflag = true;
			return;
		}
		fclose(f);

		/* time left */
		hours = (double) bat.charge_now / (double) bat.current_now;
		bat.h = (int) hours;
		bat.m = (int) (fmod(hours, 1) * 60);
		bat.s = (int) (fmod((fmod(hours, 1) * 60), 1) * 60);
	}

	/* assemble output */
	if (!bat.discharging) {
		snprintf(batdisp, DISPLEN,
				"^fg(#%X)%d%% ^i(%s/glyph_battery_%02d.xbm)^fg()",
				colour_bat, bat.capacity, path_icons, bat.capacity/10*10);
	} else {
		snprintf(batdisp, DISPLEN,
				"^fg(#%X)%d%% ^i(%s/glyph_battery_%02d.xbm)^fg()  ^fg(#%X)%dh %02dm %02ds^fg()",
				colour(bat.capacity), bat.capacity, path_icons,
				bat.capacity/10*10, colour_hl, bat.h, bat.m, bat.s);
	}
}

static void
updateCPU(void)
{
	int i;
	char w[16], e[16];

	/* prevent from updating too often */
	LONGDELAY();
	cpuflag = false;

	updateCPULoad();
	updateCPUTemp();

	/* assemble output */
	snprintf(w, 13, "^fg(#%X)", colour_warn);
	snprintf(e, 13, "^fg(#%X)", colour_err);
	cpudisp[0] = 0;
	snprintf(cpudisp, DISPLEN, "^i(%s/glyph_cpu.xbm)  ^fg(#%X)",
			path_icons, colour_hl);
	for (i = 0; i < cpu.num_cores; i++)
		snprintf(cpudisp+strlen(cpudisp), DISPLEN-strlen(cpudisp), "[%2d%%] ",
				cpu.cores[i]->load);
	snprintf(cpudisp+strlen(cpudisp), DISPLEN-strlen(cpudisp),
			"^fg() %s%d%s°C",
			cpu.temperature>=temp_crit ? e : cpu.temperature>=temp_high ? w:"",
			cpu.temperature, cpu.temperature>=temp_high ? "^fg()" : "");
}

static void
updateCPULoad(void)
{
	FILE *f;
	int i;
	int busy_tot, idle_tot, busy_diff, idle_diff, usage;
	int user, nice, system, idle, iowait, irq, softirq, steal, guest,guest_nice;

	/* usage */
	f = fopen(cpu.path_load, "r");
	if (f == NULL) {
		wrlog("Failed to open file: %s\n", cpu.path_load);
		cpuflag = true;
		return;
	}
	fscanf(f, "%*[^\n]\n"); /* ignore first line */
	for (i = 0; i < cpu.num_cores; i++) {
		fscanf(f, "%*[^ ] %d %d %d %d %d %d %d %d %d %d%*[^\n]\n",
				&user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal,
				&guest, &guest_nice);
		if (ferror(f)) {
			fclose(f);
			wrlog("Failed to read file: %s\n", cpu.path_load);
			cpuflag = true;
			return;
		}

		/* calculate usage */
		busy_tot = user+nice+system+irq+softirq+steal+guest+guest_nice;
		busy_diff = busy_tot - cpu.cores[i]->busy_last;
		idle_tot = idle + iowait;
		idle_diff = idle_tot - cpu.cores[i]->idle_last;
		usage = idle_diff + busy_diff;
		cpu.cores[i]->load = (busy_diff*1000+5)/10/usage;
		cpu.cores[i]->busy_last = busy_tot;
		cpu.cores[i]->idle_last = idle_tot;
	}
	fclose(f);
}

static void
updateCPUTemp(void)
{
	FILE *f;

	f = fopen(cpu.path_temp, "r");
	if (f == NULL) {
		wrlog("Failed to open file: %s\n", cpu.path_temp);
		cpuflag = true;
		return;
	}
	fscanf(f, "%d", &(cpu.temperature));
	if (ferror(f)) {
		fclose(f);
		wrlog("Failed to read file: %s\n", cpu.path_temp);
		cpuflag = true;
		return;
	}
	fclose(f);
	cpu.temperature /= 1000;
}

static void
updateDate(void)
{
	time(&rawtime);
	date = localtime(&rawtime);
}

static void
updateMemory(void)
{
	FILE *f;
	int i;

	/* prevent from updating too often */
	LONGDELAY();
	memflag = false;

	/* open file */
	f = fopen(mem.path, "r");
	if (f == NULL) {
		wrlog("Could not open file: %s\n", mem.path);
		memflag = true;
		return;
	}

	/* calculate */
	fscanf(f, "MemTotal: %d kB\n", &mem.total);
	fscanf(f, "MemFree: %d kB\n", &i);
	mem.used = mem.total-i;
	fscanf(f, "Buffers: %d kB\n", &i);
	mem.used -= i;
	fscanf(f, "Cached: %d kB\n", &i);
	mem.used -= i;
	mem.percentage = mem.used*100 / mem.total;
	fclose(f);

	/* update display */
	snprintf(memdisp, DISPLEN,
			"RAM:  ^fg(#%X)%d%%  ^fg()(^fg(#%X)%.1fM^fg())",
			colour_hl, mem.percentage,
			colour(100-mem.percentage), mem.used/1024.0);
}

static void
updateNetwork(void)
{
	FILE *f;
	struct ifreq ifr[NUMIFS];
	struct ifconf ifc;
	int sd, i, j=0, ifnum, addr;

	/* prevent from updating too often */
	LONGDELAY();
	netflag = false;

	/* clear old interfaces */
	for (i = 0; i < NUMIFS; ++i)
		if (netifs[i]) {
			free(netifs[i]);
			netifs[i] = NULL;
		}

	/* create a socket where we can use ioctl to retrieve interface info */
	sd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sd <= 0) {
		wrlog("Failed: socket(PF_INET, SOCK_DGRAM, 0)\n");
		netflag = true;
		return;
	}

	/* set pointer and maximum space (???) */
	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_ifcu.ifcu_buf = (caddr_t) ifr;

	/* get interface info from the socket created above */
	if (ioctl(sd, SIOCGIFCONF, &ifc) != 0) {
		wrlog("Failed: ioctl(sd, SIOCGIFCONF, &ifc)\n");
		netflag = true;
		close(sd);
		return;
	}
	
	/* get number of found interfaces */
	ifnum = ifc.ifc_len / sizeof(struct ifreq);

	for (i = 0; i < ifnum; ++i) {
		if (ifr[i].ifr_addr.sa_family != AF_INET)
			continue;

		if (!ioctl(sd, SIOCGIFADDR, &ifr[i]) && strcmp(ifr[i].ifr_name, "lo")) {
			/* create a new NetworkInterface to assign information to it */
			netifs[j] = malloc(sizeof(NetworkInterface));

			/* name */
			snprintf(netifs[j]->name, BUFLEN-1, "%s", ifr[i].ifr_name);

			/* address */
			addr = ((struct sockaddr_in *) (&ifr[i].ifr_addr))->sin_addr.s_addr;
			snprintf(netifs[j]->ip, BUFLEN-1, "%d.%d.%d.%d",
					 addr      & 0xFF, (addr>> 8) & 0xFF,
					(addr>>16) & 0xFF, (addr>>24) & 0xFF);

			/* state (UP/DOWN) */
			netifs[j]->active = (!ioctl(sd, SIOCGIFFLAGS, &ifr[i]) &&
					ifr[i].ifr_flags | IFF_UP);

			/* quality (if wlan) */
			if (!strcmp(netifs[j]->name, "wlan0")) {
				f = fopen("/proc/net/wireless", "r");
				if (f == NULL) {
					wrlog("Failed to open file: /proc/net/wireless\n");
					netflag = true;
					close(sd);
					return;
				}
				fscanf(f, "%*[^\n]\n%*[^\n]\n%*s %*d %d.%*s",
						&netifs[j]->quality);
				if (ferror(f)) {
					fclose(f);
					wrlog("Failed to open file: /proc/net/wireless\n");
					netflag = true;
					fclose(f);
					close(sd);
					return;
				}
				fclose(f);
			}

			++j;
		}
	}
	close(sd);

	/* assemble output */
	netdisp[0] = 0;
	for (i = 0; i < NUMIFS; ++i) {
		if (!netifs[i] || (!netifs[i]->active && !show_inactive_if))
			continue;
		if (!strcmp(netifs[i]->name, "wlan0"))
			snprintf(netdisp+strlen(netdisp), DISPLEN-strlen(netdisp),
					"^fg(#%X)^i(%s/glyph_wifi_%d.xbm)^fg()",
					colour(netifs[i]->quality), path_icons,
					netifs[i]->quality/20);
		if (!strcmp(netifs[i]->name, "eth0"))
			snprintf(netdisp+strlen(netdisp), DISPLEN-strlen(netdisp),
					"^i(%s/glyph_eth.xbm)", path_icons);
		snprintf(netdisp+strlen(netdisp), DISPLEN-strlen(netdisp),
				"  ^fg(#%X)%s^fg()   ^fg(#%X)%s^fg()   ",
				netifs[i]->active ? colour_hl : colour_err,
				netifs[i]->ip, colour_sep, lsep);
	}
}

static void
updateSound(void)
{
	int s;
	long l, r;

	snd_mixer_selem_get_playback_volume(snd.elem,SND_MIXER_SCHN_FRONT_LEFT,&l);
	snd_mixer_selem_get_playback_volume(snd.elem,SND_MIXER_SCHN_FRONT_RIGHT,&r);
	snd_mixer_selem_get_playback_switch(snd.elem,SND_MIXER_SCHN_FRONT_LEFT,&s);
	snd.vol = (l+r)*50/snd.max;
	snd.mute = s == 0;

	if (snd.mute)
		snprintf(snddisp, DISPLEN, "^fg(#%X)^i(%s/volume_m.xbm)  ^fg()%2ld%%",
				colour_err, path_icons, snd.vol);
	else
		snprintf(snddisp, DISPLEN,
				"^fg(#%X)^i(%s/volume_%ld.xbm) ^fg(#%X)%2ld%%^fg()",
				colour_ok, path_icons, snd.vol/34, colour_hl, snd.vol);
}

static void
wrlog(char const *format, ...)
{
	va_list args;
	fprintf(stderr, "[%02d:%02d:%02d] dzenstat: ",
			date->tm_hour, date->tm_min, date->tm_sec);
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}

int
main(int argc, char **argv)
{
	updateDate(); /* needed for logging */
	interrupted = false;
	init();
	display();
	wrlog("received shutdown signal ...\n");
	return EXIT_SUCCESS;
}

