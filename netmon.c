/* Playground for testing netlink.
 * Its purpose is to render dzenstat a bit more notification-based (instead of
 * polling - polling is bad).
 */

#include <string.h>          // memset()
#include <stdlib.h>          // EXIT_FAILURE, EXIT_SUCCESS
#include <stdio.h>           // printf()
#include <sys/socket.h>      // <socket stuff>
#include <linux/rtnetlink.h> // sockaddr_nl
#include <unistd.h>          // close()

#define BUFSIZE 4096
#define STDIN 0

int main()
{
	int fd, s, len;
	char buf[BUFSIZE];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
	struct nlmsghdr *nh;
	fd_set fds;

	// defined fields of the socket address netlink:
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

	// get file descriptor:
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	bind(fd, (struct sockaddr *) &sa, sizeof(sa));

	while (1) {
		// clear fd set:
		FD_ZERO(&fds);

		// add our fd and STDIN to the list of fds to be observed:
		FD_SET(fd, &fds);
		FD_SET(STDIN, &fds);

		// wait for activity:
		s = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		printf("ACTIVITY!\n", s);
		if (s < 0) {
			fprintf(stderr, "select() < 0 (%d)\n", s);
			break;
		}

		// check user input:
		if (FD_ISSET(STDIN, &fds)) {
			scanf("%s", buf);
			if (!strcmp(buf, "q"))
				break;
			printf("to exit, type 'q'\n");
		}
		
		// process data:
		else {
			// read message:
			len = recvmsg(fd, &msg, 0);

			// handle message (TODO 'man 7 netlink' for how to do that)
		}
	}
	close(fd);
}

