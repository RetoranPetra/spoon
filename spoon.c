#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include <mpd/client.h>

#define LEN(x) (sizeof (x) / sizeof *(x))

int dummyread(char *buf, size_t len);
int mpdread(char *buf, size_t len);
int battread(char *buf, size_t len);
int dateread(char *buf, size_t len);
int xkblayoutread(char *buf, size_t len);

struct ent {
	char *fmt;
	int (*read)(char *, size_t);
} ents[] = {
	{ .fmt = "[%s]", .read = mpdread },
	{ .fmt = " ", .read = dummyread },
	{ .fmt = "%s%%", .read = battread },
	{ .fmt = " ", .read = dummyread },
	{ .fmt = "[%s]", .read = xkblayoutread },
	{ .fmt = " ", .read = dummyread },
	{ .fmt = "%s", .read = dateread },
};

int
dummyread(char *buf, size_t len)
{
	buf[0] = '\0';
	return 0;
}

int
mpdread(char *buf, size_t len)
{
	struct mpd_connection *conn;
	struct mpd_song *song;
	const char *artist, *title;
	int ret = 0;

	conn = mpd_connection_new(NULL, 0, 0);
	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		warnx("cannot connect to mpd");
		return -1;
	}
	mpd_send_current_song(conn);
	song = mpd_recv_song(conn);
	if (song == NULL) {
		warnx("cannot recv mpd song");
		ret = -1;
		goto out;
	}
	artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	if (artist != NULL && title != NULL)
		snprintf(buf, len, "%s - %s", artist, title);
	else if (title != NULL)
		strlcpy(buf, title, len);
	else
		ret = -1;
	mpd_song_free(song);
	mpd_response_finish(conn);
out:
	mpd_connection_free(conn);
	return ret;
}

#ifdef __OpenBSD__
#include <sys/ioctl.h>
#include <fcntl.h>
#include <machine/apmvar.h>

int
battread(char *buf, size_t len)
{
	struct apm_power_info info;
	int ret, fd;

	fd = open("/dev/apm", O_RDONLY);
	if (fd < 0) {
		warn("open %s", "/dev/apm");
		return -1;
	}
	ret = ioctl(fd, APM_IOC_GETPOWER, &info);
	if (ret < 0) {
		warn("APM_IOC_GETPOWER %s", "/dev/apm");
		close(fd);
		return -1;
	}
	close(fd);
	snprintf(buf, len, "%d", info.battery_life);
	return 0;
}
#else
int
battread(char *buf, size_t len)
{
	return dummyread(buf, len);
}
#endif

int
dateread(char *buf, size_t len)
{
	struct tm *now;
	time_t t;

	time(&t);
	now = localtime(&t);
	if (now == NULL)
		return -1;
	strftime(buf, len, "%a %d %b %Y %H:%M %Z", now);
	return 0;
}

int
xkblayoutread(char *buf, size_t len)
{
	Display *dpy;
	XkbStateRec state;
	XkbRF_VarDefsRec vd;
	char *tmp = NULL, *str, *tok;
	int i, ret = 0;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		warnx("cannot open display");
		return -1;
	}
	XkbGetState(dpy, XkbUseCoreKbd, &state);
	if (XkbRF_GetNamesProp(dpy, &tmp, &vd) == 0){
		warnx("cannot extract keyboard properties");
		ret = -1;
		goto out0;
	}
	str = strdup(vd.layout);
	if (str == NULL) {
		ret = -1;
		goto out1;
	}
	tok = strtok(str, ",");
	for (i = 0; i < state.group; i++) {
		tok = strtok(NULL, ",");
		if (tok == NULL) {
			warnx("cannot extract layout");
			ret = -1;
			goto out2;
		}
	}
	strlcpy(buf, tok, len);
out2:
	free(str);
out1:
	free(tmp);
	XFree(vd.options);
out0:
	XCloseDisplay(dpy);
	return ret;
}

void
entcat(char *line, size_t len)
{
	char buf[BUFSIZ];
	char *s, *e;
	struct ent *ent;
	int ret, i;

	s = line;
	e = line + len;
	for (i = 0; i < LEN(ents); i++) {
		ent = &ents[i];
		ret = ent->read(buf, sizeof(buf));
		if (ret == 0 && s < e)
			s += snprintf(s, e - s, ent->fmt, buf);
	}
}

void
xsetroot(void)
{
	char line[BUFSIZ];
	Display *dpy;
	int screen;
	Window root;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		errx(1, "cannot open display");
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	for (;;) {
		entcat(line, sizeof(line));
		XStoreName(dpy, root, line);
		XFlush(dpy);
		sleep(1);
	}
}

int
main(void)
{
	xsetroot();
	return 0;
}
