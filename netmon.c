/* Playground for testing netlink.
 * Its purpose is to render dzenstat a bit more notification-based (instead of
 * polling - polling is bad).
 */

#include <string.h>          // memset
#include <stdlib.h>          // EXIT_FAILURE, EXIT_SUCCESS
#include <stdio.h>           // printf
//#include <asm/types.h>       // ??? (was in the manpage, but not needed)
#include <sys/socket.h>      // anything network related
#include <linux/rtnetlink.h> // sockaddr_nl

#define BUFSIZE 4096

int main()
{
	int fd, s, len;
	char buf[BUFSIZE];
	struct iovec iov = { buf, sizeof(buf) };
	struct sockaddr_nl sa;
	struct msghdr msg = { &sa, sizeof(sa), &iov, 1, NULL, 0, 0 };
	struct nlmsghdr *nh;
	fd_set socket_set;

	// defined fields of the socket address netlink:
	memset(&sa, 0, sizeof(sa));
	sa.nl_family = AF_NETLINK;
	sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

	// get file descriptor:
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	bind(fd, (struct sockaddr *) &sa, sizeof(sa));

	while (1) {
		// clear socket set:
		FD_ZERO(&socket_set);

		// add our file descriptor to the list of sockets to be observed:
		FD_SET(fd, &socket_set);

		// wait for activity on socket with select():
		s = select(FD_SETSIZE, &socket_set, NULL, NULL, NULL);

		// notification:
		if (s < 0) {
			fprintf(stderr, "failure: select() = %d\n", s);
			continue;
		}
		printf("success: select() = %d\n", s);

		// read message:
		len = recvmsg(fd, &msg, 0);

		// handle message (TODO 'man 7 netlink' for how to do that)
	}
}

