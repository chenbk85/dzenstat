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
		/* need this to get all events */
		mpd_send_idle_mask(conn, MPD_IDLE_PLAYER);
		s = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		if (s < 0) {
			fprintf(stderr, "select(): %d\n", s);
			return EXIT_FAILURE;
		}
		printf("event!\n");
		mpd_recv_idle(conn, true);
		if (mpd_connection_get_error(conn)) {
			fprintf(stderr, "mpd_recv_idle(): %s\n",
					mpd_connection_get_error_message(conn));
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
