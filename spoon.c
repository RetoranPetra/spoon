#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include "util.h"

int battread(void *, char *, size_t);
int cpuread(void *, char *, size_t);
int dateread(void *, char *, size_t);
int dummyread(void *, char *, size_t);
int loadread(void *, char *, size_t);
int mixread(void *, char *, size_t);
int mpdread(void *, char *, size_t);
int tempread(void *, char *, size_t);
int wifiread(void *, char *, size_t);
int xkblayoutread(void *, char *, size_t);
int fileread(void *, char *, size_t);

struct ent {
	char *fmt;
	int (*read)(void *, char *, size_t);
	void *arg;
};

#include "types.h"
#include "config.h"

int
dummyread(void *arg, char *buf, size_t len)
{
	buf[0] = '\0';
	return 0;
}

void
entcat(char *line, size_t len)
{
	char buf[LINE_MAX];
	char *s, *e;
	struct ent *ent;
	int ret, i;

	s = line;
	e = line + len;
	for (i = 0; i < LEN(ents); i++) {
		ent = &ents[i];
		ret = ent->read(ent->arg, buf, sizeof(buf));
		if (ret == 0 && s < e)
			s += snprintf(s, e - s, ent->fmt, buf);
	}
}

void
loop(void)
{
	char line[LINE_MAX];
	Display *dpy;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL)
		errx(1, "cannot open display");
	for (;;) {
		entcat(line, sizeof(line));
		XStoreName(dpy, DefaultRootWindow(dpy), line);
		XSync(dpy, False);
		sleep(delay);
	}
}

int
main(void)
{
	loop();
	return 0;
}
