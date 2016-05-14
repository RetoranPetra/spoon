#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LEN(x) (sizeof (x) / sizeof *(x))

int mpdread(char *buf, size_t len);
int dateread(char *buf, size_t len);
int dummyread(char *buf, size_t len);

struct ent {
	char *fmt;
	int (*read)(char *, size_t);
} ents[] = {
	{ .fmt = "[%s]", .read = mpdread },
	{ .fmt = " ", .read = dummyread },
	{ .fmt = "%s", .read = dateread },
};

int
mpdread(char *buf, size_t len)
{
	strlcpy(buf, "mpd", len);
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
	int ret;
	int i;

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
