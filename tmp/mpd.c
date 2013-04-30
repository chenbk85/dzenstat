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
	FILE *f;
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

		/* write to file (for conky) */
		f = fopen("mpd_conky", "w+");
		if (f == NULL) {
			fprintf(stderr, "failed to open conky file: %s\n", strerror(errno));
			return EXIT_FAILURE;
		}
		fprintf(f, "${color AAAA55}${font Gill Sans:weight=normal:size=18}"
		           "%s$font$color\n", title);
		fprintf(f, "%s\n", artist);
		fprintf(f, "${font Gill Sans:weight=thin:size=18:slant=italic}"
		           "%s$font\n", album);
		fprintf(f, "${image /home/ayekat/.config/conky/img/%s.jpg -n"
		           "-s 100x100}\n", album);
		fclose(f);
	}

	/* never reached (MUAHAHA!) */
	return EXIT_SUCCESS;
}

