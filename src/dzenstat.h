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

#define BUFLEN 128      /* length for buffers */
#define DISPLEN 512     /* length for display buffers (a little longer) */

#define LONGDELAY() \
		static clock_t next_update = 0; \
		if (time(NULL) < next_update) return; \
		next_update = time(NULL) + update_interval

typedef struct Module {
	int (*init)(struct Module *mod);
	int (*update)(void);
	int (*interrupt)(void);
	int (*term)(void);
	bool ignore;
	bool has_fd;
	int fd;
	char display[DISPLEN];
} Module;

/* function declarations (TODO replace some by modules) */
void pollEvents(void);
unsigned int colour(int val);
void die(void);
void display(void);
void init(void);
void initNetwork(void);
void sig_handle(int sig);
void updateDate(void);
void updateNetwork(void);
void updateNetworkDisplay(void);
void wrlog(char const *format, ...);

#endif

