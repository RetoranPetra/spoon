#include <err.h>
#include <stdio.h>

#include "util.h"

#ifdef __OpenBSD__
#include <errno.h>
#include <poll.h>
#include <sndio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHANNELS 16

static struct sioctl_hdl *hdl = NULL;
static struct pollfd *pfds = NULL;

static struct {
	unsigned int addr;
	int val;
} channel[MAX_CHANNELS];
static size_t nchannels = 0;

static int
get_output(void)
{
	size_t i;
	int val;

	if (nchannels == 0)
		return 0;

	val = 0;
	for (i = 0; i < nchannels; i++)
		val += channel[i].val;
	return 100 * ((val / (double)nchannels) / 255.0);
}

static void
ondesc(void *arg, struct sioctl_desc *desc, int val)
{
	size_t i;

	if (desc == NULL)
		return;

	if (desc->type != SIOCTL_NUM ||
	    strcmp(desc->func, "level") != 0 ||
	    strcmp(desc->node0.name, "output") != 0)
		return;

	for (i = 0; i < nchannels; i++)
		if (channel[i].addr == desc->addr)
			break;

	if (i < nchannels) {
		channel[i].val = val;
		return;
	}

	if (nchannels >= MAX_CHANNELS) {
		warnx("too many channels");
		return;
	}
	channel[i].addr = desc->addr;
	channel[i].val = val;
	nchannels++;
}

static void
onval(void *arg, unsigned int addr, unsigned int val)
{
	size_t i;

	for (i = 0; i < nchannels; i++)
		if (channel[i].addr == addr) {
			channel[i].val = val;
			break;
		}
}

static int
do_init(void)
{
	hdl = sioctl_open(SIO_DEVANY, SIOCTL_READ, 0);
	if (hdl == NULL) {
		warnx("sioctl_open %s", SIO_DEVANY);
		return 0;
	}
	if (!sioctl_ondesc(hdl, ondesc, NULL)) {
		warnx("sioctl_ondesc");
		sioctl_close(hdl);
		return 0;
	}
	sioctl_onval(hdl, onval, NULL);

	return 1;
}

static int
poll_peek(void)
{
	int nfds, revents;

	if (pfds == NULL) {
		pfds = malloc(sizeof(struct pollfd) * sioctl_nfds(hdl));
		if (pfds == NULL) {
			warnx("out of memory");
			goto out;
		}
	}

	nfds = sioctl_pollfd(hdl, pfds, POLLIN);
	if (nfds == 0)
		return 1;
	while (poll(pfds, nfds, 0) == -1)
		if (errno != EINTR) {
			warn("sioctl poll");
			goto out;
		}
	revents = sioctl_revents(hdl, pfds);
	if (revents & POLLHUP) {
		warnx("sioctl disconnected");
		goto out;
	}

	return 1;

out:
	free(pfds);
	pfds = NULL;
	sioctl_close(hdl);

	return 0;
}

int
mixread(void *arg, char *buf, size_t len)
{
	static int init_done = 0;
	struct pollfd *pfds;
	int nfds;

	if (!init_done) {
		if (!do_init())
			return -1;
		init_done = 1;
	}

	init_done = poll_peek();
	snprintf(buf, len, "%d%%", get_output());

	return 0;
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
