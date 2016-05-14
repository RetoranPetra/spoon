#include <sys/types.h>
#include <sys/ioctl.h>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <machine/apmvar.h>

#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

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
	strlcpy(buf, "mpd", len);
	return 0;
}

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

int
dateread(char *buf, size_t len)
{
	struct tm *now;
	time_t t;

	time(&t);
	now = localtime(&t);
	if (now == NULL)
		return -1;
	strftime(buf, len, "%c", now);
	return 0;
}

int
xkblayoutread(char *buf, size_t len)
{
	Display *dpy;
	XkbStateRec state;
	XkbRF_VarDefsRec vd;
	char *tmp, *str, *tok;
	int i, ret = 0;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		warnx("cannot open display");
		return -1;
	}
	XkbGetState(dpy, XkbUseCoreKbd, &state);
	XkbRF_GetNamesProp(dpy, &tmp, &vd);
	str = strdup(vd.layout);
	if (str == NULL) {
		ret = -1;
		goto out0;
	}
	tok = strtok(str, ",");
	for (i = 0; i < state.group; i++) {
		tok = strtok(NULL, ",");
		if (tok == NULL) {
			warnx("cannot extract layout");
			ret = -1;
			goto out1;
		}
	}
	strlcpy(buf, tok, len);
out1:
	free(str);
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

int
main(void)
{
	char line[BUFSIZ];
	entcat(line, sizeof(line));
	puts(line);
	return 0;
}
