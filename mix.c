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
	snd_mixer_t *mixerhdl;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	long min, max;
	long vol;

	if (snd_mixer_open(&mixerhdl, 0) < 0) {
		warnx("snd_mixer_open: failed");
		return -1;
	}
	if (snd_mixer_attach(mixerhdl, "default") < 0) {
		warnx("snd_mixer_attach: failed");
		goto err0;
	}
	if (snd_mixer_selem_register(mixerhdl, NULL, NULL) < 0) {
		warnx("snd_mixer_selem_register: failed");
		goto err0;
	}
	if (snd_mixer_load(mixerhdl) < 0) {
		warnx("snd_mixer_load: failed");
		goto err0;
	}
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_name(sid, "Master");
	elem = snd_mixer_find_selem(mixerhdl, sid);
	if (elem == NULL) {
		warnx("snd_mixer_find_selem: failed");
		goto err0;
	}
	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_get_playback_volume(elem, 0, &vol);
	snd_mixer_close(mixerhdl);
	if (max == 0)
		snprintf(buf, len, "0%%");
	else
		snprintf(buf, len, "%ld%%", vol * 100 / max);
	return 0;
err0:
	snd_mixer_close(mixerhdl);
	return -1;
}
#endif
