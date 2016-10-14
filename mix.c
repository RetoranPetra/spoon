#include <err.h>
#include <stdio.h>

#ifdef __OpenBSD__
#include <sys/ioctl.h>
#include <sys/audioio.h>

#include <fcntl.h>
#include <string.h>

int
mixread(char *buf, size_t len)
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
#endif
