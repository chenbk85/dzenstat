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
		next_update = time(NULL) + update_interval

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
	int used, total, percentage;
	char const *path;
	struct sysinfo info;
} Memory;

typedef struct Module {
	int (*init)(struct Module *mod);
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
void pollEvents(void);
unsigned int colour(int val);
void die(void);
void display(void);
void init(void);
void initBattery(void);
void initMemory(void);
void initNetwork(void);
void initSound(void);
void sig_handle(int sig);
void updateBattery(void);
void updateDate(void);
void updateMemory(void);
void updateNetwork(void);
void updateNetworkDisplay(void);
void updateSound(void);
void wrlog(char const *format, ...);

#endif

