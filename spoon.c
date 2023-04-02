#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "arg.h"
#include "util.h"

int battread(void *, char *, size_t);
int cpuread(void *, char *, size_t);
int dateread(void *, char *, size_t);
int dummyread(void *, char *, size_t);
int loadread(void *, char *, size_t);
int mixread(void *, char *, size_t);
int mymixread(void *, char *, size_t);
int mpdread(void *, char *, size_t);
int tempread(void *, char *, size_t);
int wifiread(void *, char *, size_t);
int xkblayoutread(void *, char *, size_t);
int fileread(void *, char *, size_t);
int keyread(void *, char *, size_t);
int netspeedread(void *, char *, size_t);
int brightnessread(void *, char *, size_t);
int countread(void *, char *, size_t);

struct ent {
	char *fmt;
	int (*read)(void *, char *, size_t);
	void *arg;
};

#include "types.h"
#include "config.h"

char *argv0;
int single;
int tty;

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
xloop(void)
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
		if (single)
			break;
		sleep(delay);
	}
}

void
ttyloop(void)
{
	char line[LINE_MAX];

	for (;;) {
		entcat(line, sizeof(line));
		puts(line);
		fflush(stdout);
		if (single)
			break;
		sleep(delay);
	}
}

void
usage(void)
{
	fprintf(stderr, "%s: [-st]\n", argv0);
	fprintf(stderr, "  -s	One shot mode\n");
	fprintf(stderr, "  -t	Use TTY backend\n");
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *end;

	ARGBEGIN {
	case 's':
		single = 1;
		break;
	case 't':
		tty = 1;
		break;
	default:
		usage();
	} ARGEND

	if (tty)
		ttyloop();
	else
		xloop();
	return 0;
}
