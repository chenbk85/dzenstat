/* dzenstat module for displaying MPD song information.
 */

#include "mpd.h"
#include "config/global.h"
#include "config/mpd.h"
#include <mpd/client.h>
#include <stdio.h>

static int interrupt(void);
static int term(void);
static int get_info(void);

static struct mpd_connection *con;
static char *dy;
static Module *m;

int
mpd_init(Module *mod)
{
	m = mod;
	m->interrupt = interrupt;
	m->term = term;
	m->ignore = true;
	m->has_fd = true;
	dy = m->display;

	/* connect & prepare */
	con = mpd_connection_new(NULL, 0, 0);
	if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
		wrlog("MPD: %s\n", mpd_connection_get_error_message(con));
		return -1;
	}
	m->fd = mpd_connection_get_fd(con);

	/* initial update */
	if (get_info() < 0)
		return -1;

	return 0;

}

static int
interrupt(void)
{
	/* empty event buffer */
	mpd_recv_idle(con, true);
	if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
		wrlog("MPD receive: %s\n", mpd_connection_get_error_message(con));
		return -1;
	}

	return get_info();
}

static int
get_info(void)
{
	struct mpd_song *song;
	struct mpd_status *status;
	char title[BUFLEN], artist[BUFLEN], album[BUFLEN];
	enum mpd_state state;

	/* get status */
	mpd_send_status(con);
	status = mpd_recv_status(con);
	if (status == NULL) {
		wrlog("MPD status: %s\n", mpd_connection_get_error_message(con));
		return -1;
	}
	state = mpd_status_get_state(status);
	mpd_status_free(status);
	if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
		wrlog("MPD state: %s\n", mpd_connection_get_error_message(con));
		return -1;
	}

	/* get song */
	mpd_send_current_song(con);
	while ((song = mpd_recv_song(con)) != NULL) {
		snprintf(title,BUFLEN,"%s",mpd_song_get_tag(song,MPD_TAG_TITLE,0));
		snprintf(artist,BUFLEN,"%s",mpd_song_get_tag(song,MPD_TAG_ARTIST,0));
		snprintf(album,BUFLEN,"%s",mpd_song_get_tag(song,MPD_TAG_ALBUM,0));
		mpd_song_free(song);
		if (mpd_connection_get_error(con) != MPD_ERROR_SUCCESS) {
			wrlog("MPD song: %s\n", mpd_connection_get_error_message(con));
			return -1;
		}
	}

	/* update display */
	if (hide_paused && state != MPD_STATE_PLAY)
		m->hide = true;
	else {
		m->hide = false;
		sprintf(dy, "^fg(#%X)^i(%s/mpd.xbm)^fg() %s",
				state == MPD_STATE_PLAY ? colour_ok : colour_medium_bg, path_icons, title);
	}

	/* send command for next event; react on state changes (play, pause, stop)*/
	mpd_send_idle_mask(con, MPD_IDLE_PLAYER);

	return 0;
}

static int
term(void)
{
	mpd_connection_free(con);
	return 0;
}

