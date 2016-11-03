#include <err.h>
#include <stdio.h>

#ifdef __OpenBSD__
#include <sys/ioctl.h>
#include <sys/audioio.h>

#include <fcntl.h>
#include <string.h>

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
#elif __linux__
#include <alsa/asoundlib.h>

int
mixread(void *arg, char *buf, size_t len)
{
	snd_mixer_selem_id_t *id;
	snd_mixer_elem_t *elem;
	snd_mixer_t *mixerp;
	long min, max, vol;
	int ret = -1, r;
	int master;

	snd_mixer_selem_id_alloca(&id);
	snd_mixer_selem_id_set_name(id, "Master");
	snd_mixer_selem_id_set_index(id, 0);

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
	r = snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	if (r < 0) {
		warnx("snd_mixer_selem_get_playback_volume_range: %s",
		      snd_strerror(r));
		goto out;
	}
	r = snd_mixer_selem_get_playback_volume(elem,
	    SND_MIXER_SCHN_FRONT_LEFT, &vol);
	if (r < 0) {
		warnx("snd_mixer_selem_get_playback_volume: %s",
		      snd_strerror(r));
		goto out;
	}
	/* compute percentage */
	vol -= min;
	max -= min;
	if (max == 0)
		master = 0;
	else
		master = 100 * vol / max;
	snprintf(buf, len, "%d%%", master);
	ret = 0;
out:
	snd_mixer_close(mixerp);
	return ret;
}
#endif
