/* See LICENSE file for copyright and license details. */
#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include <mpd/client.h>

#include "util.h"

#define LEN(x) (sizeof (x) / sizeof *(x))

int dummyread(char *buf, size_t len);
int mpdread(char *buf, size_t len);
int cpuread(char *buf, size_t len);
int tempread(char *buf, size_t len);
int battread(char *buf, size_t len);
int mixread(char *buf, size_t len);
int wifiread(char *buf, size_t len);
int dateread(char *buf, size_t len);
int xkblayoutread(char *buf, size_t len);

struct ent {
	char *fmt;
	int (*read)(char *, size_t);
};

#include "config.h"

int
dummyread(char *buf, size_t len)
{
	buf[0] = '\0';
	return 0;
}

int
mpdread(char *buf, size_t len)
{
	static struct mpd_connection *conn;
	struct mpd_song *song;
	const char *artist, *title, *name;

	if (conn == NULL) {
		conn = mpd_connection_new(NULL, 0, 0);
		if (conn == NULL)
			return -1;
		if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
			goto out;
	}
	mpd_send_current_song(conn);
	song = mpd_recv_song(conn);
	if (song == NULL) {
		if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
			goto out;
		/* if no song is playing, reuse connection next time */
		return -1;
	}
	artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
	title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
	if (artist != NULL && title != NULL) {
		snprintf(buf, len, "%s - %s", artist, title);
	} else if (title != NULL) {
		strlcpy(buf, title, len);
	} else {
		name = mpd_song_get_uri(song);
		if (name == NULL) {
			mpd_song_free(song);
			goto out;
		}
		strlcpy(buf, name, len);
	}
	mpd_song_free(song);
	if (!mpd_response_finish(conn))
		goto out;
	return 0;
out:
	warnx("failed to talk to mpd");
	mpd_connection_free(conn);
	conn = NULL;
	return -1;
}

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
loop(void)
{
	char line[BUFSIZ];
	Display *dpy;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		errx(1, "cannot open display");
	for (;;) {
		entcat(line, sizeof(line));
		XStoreName(dpy, DefaultRootWindow(dpy), line);
		XSync(dpy, False);
		sleep(1);
	}
}

int
main(void)
{
	loop();
	return 0;
}
