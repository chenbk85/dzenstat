#include <mpd/client.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
	struct mpd_connection *conn;
	int fd, s;

	conn = mpd_connection_new(NULL, 0, 0);
	fd = mpd_connection_get_fd(conn);

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	while (1) {
		s = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		if (s < 0) {
			printf("select() = %d\n", s);
			exit(EXIT_FAILURE);
		}
		printf("event!\n");
	}
	return EXIT_SUCCESS;
}
