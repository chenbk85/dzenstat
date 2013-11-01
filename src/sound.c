/* dzenstat module for sound (ALSA)
 */

#include "sound.h"
#include "config/global.h"
#include "config/sound.h"
#include <alsa/asoundlib.h>

static int interrupt(void);
static int term(void);
static char *dy;

static long min, max, vol;
static bool mute;
static snd_mixer_t *ctl;
static snd_mixer_elem_t *elem;

int
sound_init(Module *mod)
{
	int ret = 0;
	snd_mixer_selem_id_t *sid;
	char const *card = "default";
	struct pollfd pfd;

	mod->interrupt = interrupt;
	mod->term = term;
	dy = mod->display;
	mod->ignore = true; /* don't update periodically */

	/* set up control interface */
	ret += snd_mixer_open(&ctl, 0);
	ret += snd_mixer_attach(ctl, card);
	ret += snd_mixer_selem_register(ctl, NULL, NULL);
	ret += snd_mixer_load(ctl);
	if (ret < 0) {
		wrlog("ALSA: Could not open mixer\n");
		return -1;
	}

	/* get mixer element */
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, "Master");
	elem = snd_mixer_find_selem(ctl, sid);
	if (elem == NULL) {
		wrlog("ALSA: Could not get mixer element\n");
		snd_mixer_detach(ctl, card);
		snd_mixer_close(ctl);
		return -1;
	}

	/* get poll file descriptor */
	if (snd_mixer_poll_descriptors(ctl, &pfd, 1)) {
		mod->fd = pfd.fd;
	} else {
		wrlog("ALSA: Could not get file descriptor\n");
		return -1;
	}

	/* get volume range */
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);

	/* confirm file descripter */
	mod->has_fd = true;

	/* initial update */
	return interrupt();
}

static int
interrupt(void)
{
	int s;
	long l, r;

	snd_mixer_handle_events(ctl);
	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &l);
	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_RIGHT, &r);
	snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT, &s);
	vol = (l+r)*50/max;
	mute = s == 0;

	if (mute)
		snprintf(dy, DISPLEN, "^fg(#%X)^i(%s/sound_mute.xbm) ^fg()%2ld%%",
				colour_err, path_icons, vol);
	else
		snprintf(dy, DISPLEN,
				"^fg(#%X)^i(%s/sound_%ld.xbm) ^fg(#%X)%2ld%%^fg()",
				colour_ok, path_icons, vol/34, colour_hl, vol);

	return 0;
}

static int
term(void)
{
	return 0;
}

