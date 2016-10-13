#include <err.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>

#define LEN(x) (sizeof (x) / sizeof *(x))

int battread(char *, size_t);
int cpuread(char *, size_t);
int dateread(char *, size_t);
int dummyread(char *, size_t);
int loadread(char *, size_t);
int mixread(char *, size_t);
int mpdread(char *, size_t);
int tempread(char *, size_t);
int wifiread(char *, size_t);
int xkblayoutread(char *, size_t);

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
		sleep(delay);
	}
}

int
main(void)
{
	loop();
	return 0;
}
