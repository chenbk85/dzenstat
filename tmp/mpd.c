/* This is a dummy programme for connecting and retrieving information from MPD.
 */

#include <mpd/client.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFLEN 512

int
main(int argc, char **argv)
{
	struct mpd_connection *con;
	int fd, s;
	fd_set fds;
	struct mpd_song *song;
	struct mpd_status *status;
	char title[BUFLEN], artist[BUFLEN], album[BUFLEN];
	enum mpd_state state;

	/* connect & prepare */
	con = mpd_connection_new(NULL, 0, 0);
	fd = mpd_connection_get_fd(con);

	/* add MPD fd to the list of file descriptors
	 * (since this is the only fd, we can put this outside the loop) */
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	while (1) {
		/* only react on state change events (playing, pause, stop, ...) */
		mpd_send_idle_mask(con, MPD_IDLE_PLAYER);

		/* wait for event */
		s = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		if (s < 0) {
			fprintf(stderr, "I/O error: %d\n", s);
			mpd_connection_free(con);
			return EXIT_FAILURE;
		}

		/* empty event buffer */
		mpd_recv_idle(con, true);
		if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
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
		state = mpd_status_get_state(status);
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
			snprintf(title,BUFLEN,"%s",mpd_song_get_tag(song,MPD_TAG_TITLE,0));
			snprintf(artist,BUFLEN,"%s",mpd_song_get_tag(song,MPD_TAG_ARTIST,0));
			snprintf(album,BUFLEN,"%s",mpd_song_get_tag(song,MPD_TAG_ALBUM,0));
			mpd_song_free(song);
			if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
				fprintf(stderr, "MPD error: %s\n",
						mpd_connection_get_error_message(con));
				mpd_connection_free(con);
				return EXIT_FAILURE;
			}
		}

		/* output */
		printf("Title:  %s\n"
		       "Artist: %s\n"
		       "Album:  %s\n"
		       "State:  %s\n"
		       "---\n",
		       title, artist, album,
		       state == MPD_STATE_PLAY ? "playing" :
		       state == MPD_STATE_PAUSE ? "paused" :
		       state == MPD_STATE_STOP ? "stopped" : "unknown");
	}

	/* never reached (MUAHAHA!) */
	return EXIT_SUCCESS;
}

