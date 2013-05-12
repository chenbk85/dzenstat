#ifndef DZENSTAT_H
#define DZENSTAT_H

/* Header file for dzenstat.
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
#include <sys/select.h>
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

/* TODO replace by module */
typedef struct {
	char path_charge_now[BUFLEN], path_charge_full[BUFLEN],
	     path_charge_full_design[BUFLEN], path_current_now[BUFLEN],
	     path_capacity[BUFLEN], path_status[BUFLEN];
	int h, m, s;
	int charge_now, charge_full, charge_full_design, current_now, capacity;
	bool discharging;
} Battery;

/* TODO replace by module */
typedef struct {
	int busy_last, idle_last;
	int load;
} Core;

/* TODO replace by module */
typedef struct {
	char const *path_temp, *path_load;
	int temperature;
	Core **cores;
	int num_cores;
} CPU;

/* TODO replace by module */
typedef struct {
	int used, total, percentage;
	char const *path;
	struct sysinfo info;
} Memory;

typedef struct {
	int (*init)(Module *mod, char *disp);
	int (*update)(void);
	int (*term)(void);
	int fd;
	bool is_fd;
	char display[DISPLEN];
} Module;

/* TODO replace by module */
typedef struct {
	char name[BUFLEN];
	char ip[BUFLEN];
	bool active;
	int quality;
} NetworkInterface;
int net_fd;
struct sockaddr_nl net_sa;

/* TODO replace by module */
typedef struct {
	long min, max, vol;
	bool mute;
	char const* path;
	int fd;
	snd_mixer_t *ctl;
	snd_mixer_elem_t *elem;
} Sound;

/* function declarations (TODO replace some by modules) */
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
static void updateNetworkDisplay(void);
static void updateSound(void);
static void wrlog(char const *format, ...);

/* variables (TODO replace some by modules) */
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

/* displays & flags (TODO replace by module) */
static char batdisp[DISPLEN]; static bool batflag = false;
static char cpudisp[DISPLEN]; static bool cpuflag = false;
static char memdisp[DISPLEN]; static bool memflag = false;
static char netdisp[DISPLEN]; static bool netflag = false;
static char snddisp[DISPLEN]; static bool sndflag = false;

/* seperator icons */
static char lsep[BUFLEN], lfsep[BUFLEN], rsep[BUFLEN], rfsep[BUFLEN];

/* load user configuration */
#include "config.h"

#endif
