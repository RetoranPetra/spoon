#include <err.h>
#include <stdio.h>

#include "util.h"

#ifdef __OpenBSD__
#include <sys/ioctl.h>
#include <sys/audioio.h>

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int
mixread(void *arg, char *buf, size_t len)
{
	mixer_devinfo_t dinfo;
	mixer_ctrl_t mctl;
	int fd, master, ret = 0, i = -1;

	fd = open("/dev/mixer", O_RDONLY);
	if (fd == -1) {
		warn("open %s", "/dev/mixer");
		return -1;
	}
	dinfo.index = 0;
	/* outputs */
	for (; ; dinfo.index++) {
		ret = ioctl(fd, AUDIO_MIXER_DEVINFO, &dinfo);
		if (ret == -1) {
			warn("AUDIO_MIXER_DEVINFO %s", "/dev/mixer");
			goto out;
		}
		if (dinfo.type == AUDIO_MIXER_CLASS &&
		    strcmp(dinfo.label.name, AudioCoutputs) == 0) {
			i = dinfo.index;
			break;
		}
	}
	if (i == -1) {
		warnx("no outputs mixer class: %s", "/dev/mixer");
		goto out;
	}
	/* outputs.master */
	for (; ; dinfo.index++) {
		ret = ioctl(fd, AUDIO_MIXER_DEVINFO, &dinfo);
		if (ret == -1) {
			warn("AUDIO_MIXER_DEVINFO %s", "/dev/mixer");
			goto out;
		}
		if (dinfo.type == AUDIO_MIXER_VALUE &&
		    dinfo.prev == AUDIO_MIXER_LAST &&
		    dinfo.mixer_class == i &&
		    strcmp(dinfo.label.name, AudioNmaster) == 0)
			break;
	}
	mctl.dev = dinfo.index;
	ret = ioctl(fd, AUDIO_MIXER_READ, &mctl);
	if (ret == -1) {
		warn("AUDIO_MIXER_READ %s", "/dev/mixer");
		goto out;
	}
	master = mctl.un.value.level[0] * 100 / 255;
	snprintf(buf, len, "%d%%", master);
out:
	close(fd);
	return ret;
}
#elif __linux__ && !USE_TINYALSA
#include <alsa/asoundlib.h>

static int active;
static int master;

int
mixer_elem_cb(snd_mixer_elem_t *elem, unsigned int mask)
{
	long min, max, vol;
	int r;

	r = snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_UNKNOWN, &active);
	if (r < 0) {
		warnx("snd_mixer_selem_get_playback_switch: %s",
		      snd_strerror(r));
		return -1;
	}
	DPRINTF_D(active);
	r = snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	if (r < 0) {
		warnx("snd_mixer_selem_get_playback_volume_range: %s",
		      snd_strerror(r));
		return -1;
	}
	r = snd_mixer_selem_get_playback_volume(elem,
	    SND_MIXER_SCHN_UNKNOWN, &vol);
	if (r < 0) {
		warnx("snd_mixer_selem_get_playback_volume: %s",
		      snd_strerror(r));
		return -1;
	}
	/* compute percentage */
	vol -= min;
	max -= min;
	if (max == 0)
		master = 0;
	else
		master = 100 * vol / max;
	DPRINTF_D(master);
	return 0;
}

int
mixread(void *arg, char *buf, size_t len)
{
	snd_mixer_selem_id_t *id;
	snd_mixer_elem_t *elem;
	static snd_mixer_t *mixerp;
	struct pollfd pfd[1];
	int r;

	snd_mixer_selem_id_alloca(&id);
	snd_mixer_selem_id_set_name(id, "Master");
	snd_mixer_selem_id_set_index(id, 0);

	if (mixerp != NULL)
		goto readvol;

	r = snd_mixer_open(&mixerp, O_RDONLY);
	if (r < 0) {
		warnx("snd_mixer_open: %s", snd_strerror(r));
		return -1;
	}
	r = snd_mixer_attach(mixerp, "default");
	if (r < 0) {
		warnx("snd_mixer_attach: %s", snd_strerror(r));
		goto out;
	}
	r = snd_mixer_selem_register(mixerp, NULL, NULL);
	if (r < 0) {
		warnx("snd_mixer_selem_register: %s", snd_strerror(r));
		goto out;
	}
	r = snd_mixer_load(mixerp);
	if (r < 0) {
		warnx("snd_mixer_load: %s", snd_strerror(r));
		goto out;
	}
	elem = snd_mixer_find_selem(mixerp, id);
	if (elem == NULL) {
		warnx("could not find mixer element");
		goto out;
	}
	snd_mixer_elem_set_callback(elem, mixer_elem_cb);
	/* force the callback the first time around */
	r = mixer_elem_cb(elem, 0);
	if (r < 0)
		goto out;
readvol:
	r = snd_mixer_poll_descriptors(mixerp, pfd, LEN(pfd));
	if (r < 0) {
		warnx("snd_mixer_poll_descriptors: %s", snd_strerror(r));
		goto out;
	}
	r = snd_mixer_handle_events(mixerp);
	if (r < 0) {
		warnx("snd_mixer_handle_events: %s", snd_strerror(r));
		goto out;
	}
	if (active)
		snprintf(buf, len, "%d%%", master);
	else
		snprintf(buf, len, "!%d%%", master);
	return 0;
out:
	snd_mixer_free(mixerp);
	snd_mixer_close(mixerp);
	mixerp = NULL;
	return -1;
}
#elif __linux__ && USE_TINYALSA
#include <tinyalsa/asoundlib.h>

int
mixread(void *arg, char *buf, size_t len)
{
	static struct mixer	*mixer;
	struct mixer_ctl	*ctl;
	int			 cur, max;

	if (mixer == NULL && (mixer = mixer_open(0)) == NULL) {
		warnx("mixer_open() failed");
		return -1;
	}

	if ((ctl = mixer_get_ctl_by_name(mixer, "Master Playback Switch"))
	    == NULL) {
		warnx("mixer_get_ctl_by_name() failed");
		goto out;
	}
	if (!mixer_ctl_get_value(ctl, 0)) {
		snprintf(buf, len, "mute");
		return 0;
	}

	if ((ctl = mixer_get_ctl_by_name(mixer, "Master Playback Volume"))
	    == NULL) {
		warnx("mixer_get_ctl_by_name() failed");
		goto out;
	}

	cur = mixer_ctl_get_value(ctl, 0);
	max = mixer_ctl_get_range_max(ctl);
	snprintf(buf, len, "%d%%", cur * 100 / max);
	return 0;

out:
	mixer_close(mixer);
	mixer = NULL;
	return -1;
}
#endif
