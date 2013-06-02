/* dzenstat module for battery information
 */

#include "network.h"
#include "config/network.h"
#include "config/global.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <linux/rtnetlink.h>
#include <unistd.h>

#define NUMIFS 10

typedef struct {
	char name[BUFLEN];
	char ip[BUFLEN];
	bool active;
	int quality;
} NetworkInterface;

/* function declaration */
static int update(void);
static int interrupt(void);
static int ifscan(void);
static int term(void);

/* variable declaration */
static char *dy;
static struct sockaddr_nl sa;
static NetworkInterface *netifs[NUMIFS];
static Module *mod;

int
network_init(Module *m)
{
	mod = m;
	mod->update = update;
	mod->interrupt = interrupt;
	mod->term = term;
	dy = mod->display;
	mod->has_fd = true;

	/* prepare fields */
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR; /*TODO up/down events*/

	/* get file descriptor */
	mod->fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	bind(mod->fd, (struct sockaddr *) &sa, sizeof(sa));

	/* initial update */
	return ifscan() || update();
}

static int
update(void)
{
	FILE *f;
	int i;

	dy[0] = 0;
	for (i = 0; i < NUMIFS; ++i) {
		if (!netifs[i] || (!netifs[i]->active && !show_inactive_if))
			continue;
		if (!strcmp(netifs[i]->name, "wlan0")) {
			/* quality (if wlan) */
			if (!strcmp(netifs[i]->name, "wlan0")) {
				f = fopen("/proc/net/wireless", "r");
				if (f == NULL) {
					wrlog("Failed to open file: /proc/net/wireless\n");
					return -1;
				}
				fscanf(f, "%*[^\n]\n%*[^\n]\n%*s %*d %d.%*s",
						&netifs[i]->quality);
				if (ferror(f)) {
					fclose(f);
					wrlog("Failed to open file: /proc/net/wireless\n");
					fclose(f);
					return -1;
				}
				fclose(f);
			}
			snprintf(dy+strlen(dy), DISPLEN-strlen(dy),
					"^fg(#%X)^i(%s/glyph_wifi_%d.xbm)^fg()",
					colour(netifs[i]->quality), path_icons,
					netifs[i]->quality/20);
		}
		if (!strcmp(netifs[i]->name, "eth0"))
			snprintf(dy+strlen(dy), DISPLEN-strlen(dy),
					"^i(%s/glyph_eth.xbm)", path_icons);
		snprintf(dy+strlen(dy), DISPLEN-strlen(dy),
				"  ^fg(#%X)%s^fg()   ^fg(#%X)%s^fg()   ",
				netifs[i]->active ? colour_hl : colour_err,
				netifs[i]->ip, colour_sep, "|"/*lsep*/);
	}

	return 0;
}

static int
interrupt(void)
{
	char buf[4096];
	struct iovec iov = { buf, sizeof(buf) };
	struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };

	/* clear buffer */
	recvmsg(mod->fd, &msg, 0);

	return ifscan();
}

static int
ifscan(void)
{
	struct ifreq ifr[NUMIFS];
	struct ifconf ifc;
	int sd, i, j=0, ifnum, addr;

	/* clear old interfaces */
	for (i = 0; i < NUMIFS; ++i) {
		if (netifs[i]) {
			free(netifs[i]);
			netifs[i] = NULL;
		}
	}

	/* create a socket where we can use ioctl to retrieve interface info */
	sd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sd <= 0) {
		wrlog("Failed: socket(PF_INET, SOCK_DGRAM, 0)\n");
		return -1;
	}

	/* set pointer and maximum space (???) */
	ifc.ifc_len = sizeof(ifr);
	ifc.ifc_ifcu.ifcu_buf = (caddr_t) ifr;

	/* get interface info from the socket created above */
	if (ioctl(sd, SIOCGIFCONF, &ifc) != 0) {
		wrlog("Failed: ioctl(sd, SIOCGIFCONF, &ifc)\n");
		close(sd);
		return -1;
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
			++j;
		}
	}
	close(sd);
	return 0;
}

static int
term(void)
{
	/* TODO */
	return 0;
}

