/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>

#define LEN(x) (sizeof (x) / sizeof *(x))

int battread(char *buf, size_t len);
int cpuread(char *buf, size_t len);
int dateread(char *buf, size_t len);
int dummyread(char *buf, size_t len);
int loadread(char *buf, size_t len);
int mixread(char *buf, size_t len);
int mpdread(char *buf, size_t len);
int tempread(char *buf, size_t len);
int wifiread(char *buf, size_t len);
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
