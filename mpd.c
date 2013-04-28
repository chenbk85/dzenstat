#include <mpd/client.h>
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
	struct mpd_connection *con;
	int fd, s;
	fd_set fds;
	struct mpd_song *song;
	struct mpd_status *status;

	/* connect & prepare */
	con = mpd_connection_new(NULL, 0, 0);
	fd = mpd_connection_get_fd(con);

	while (1) {
		mpd_send_idle_mask(con, MPD_IDLE_PLAYER);

		/* add fd to fd list (since this is the only fd, we might also put this
		 * outside the loop, but keep things consistent, won't we? */
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		/* wait for event */
		s = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		if (s < 0) {
			fprintf(stderr, "I/O error: %d\n", s);
			mpd_connection_free(con);
			return EXIT_FAILURE;
		}
		if (s == 0) {
			return EXIT_SUCCESS;
		}

		/* empty buffer */
		mpd_recv_idle(con, true);
		if (mpd_connection_get_error(con)) {
			fprintf(stderr, "MPD error: %s\n",
					mpd_connection_get_error_message(con));
			mpd_connection_free(con);
			return EXIT_FAILURE;
		}

		/* get status */
		mpd_send_status(con);
		status = mpd_recv_status(con);
		if (status == NULL) {
			fprintf(stderr, "MPD error: %s\n",
					mpd_connection_get_error_message(con));
			mpd_connection_free(con);
			return EXIT_FAILURE;
		}
		if (mpd_status_get_state(status) == MPD_STATE_PLAY)
			printf("playing:\n");
		if (mpd_status_get_state(status) == MPD_STATE_PAUSE)
			printf("paused:\n");
		mpd_status_free(status);
		if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
			fprintf(stderr, "MPD error: %s\n",
					mpd_connection_get_error_message(con));
			mpd_connection_free(con);
			return EXIT_FAILURE;
		}

		/* get song */
		mpd_send_current_song(con);
		while ((song = mpd_recv_song(con)) != NULL) {
			printf("title: %s\n", mpd_song_get_tag(song, MPD_TAG_TITLE, 0));
			printf("artist: %s\n", mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
			printf("album: %s\n", mpd_song_get_tag(song, MPD_TAG_ALBUM, 0));
			mpd_song_free(song);
			if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
				fprintf(stderr, "MPD error: %s\n",
						mpd_connection_get_error_message(con));
				mpd_connection_free(con);
				return EXIT_FAILURE;
			}
		}
		printf("\n");
	}
	return EXIT_SUCCESS;
}

