/* Playground for testing ALSA's mixer interface.
 * Its purpose is to render dzenstat a bit more notification-based (instead of
 * polling - polling is bad).
 *
 * Compile with: gcc -lasound alsa.c
 */

#include <stdio.h>
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <stdlib.h>

#define RETDIE(FNAME) \
        if (ret < 0) { \
        	fprintf(stderr, "%s = %d\n", FNAME, ret); \
        	return EXIT_FAILURE; \
        }

int
main(int argc, char **argv)
{
	int fd, ret;
	long voll, volr, min, max;
	fd_set fds;
	snd_mixer_t *ctl;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *channel = "Master";
	const char *card = "default";
	struct pollfd pfd;

	/* set up */
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, channel);

	ret = snd_mixer_open(&ctl, 0);
	RETDIE("snd_mixer_open()");
	ret = snd_mixer_attach(ctl, card);
	RETDIE("snd_mixer_attach()");
	ret = snd_mixer_selem_register(ctl, NULL, NULL);
	RETDIE("snd_mixer_selem_register()");
	ret = snd_mixer_load(ctl);
	RETDIE("snd_mixer_load()");

	elem = snd_mixer_find_selem(ctl, sid);
	if (!elem) {
		fprintf(stderr, "snd_mixer_find_selem() = NULL\n");
		return EXIT_FAILURE;
	}

	if (snd_mixer_poll_descriptors(ctl, &pfd, 1)) {
		fd = pfd.fd;
	} else {
		return EXIT_FAILURE;
	}
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	while (1) {
		select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		snd_mixer_handle_events(ctl); /* remove event from fd */
		snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT,
				&voll);
		snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT,
				&volr);
		snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
		printf("volume: %ld%%\n", ((voll+volr)*50)/max);
	}

	/* tear down */
	snd_mixer_detach(ctl, card);
	snd_mixer_close(ctl);
}

