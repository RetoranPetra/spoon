#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "types.h"

int
dateread(void *arg, char *buf, size_t len)
{
	struct datearg *datearg = arg;
	struct tm *now;
	char *oldtz;
	time_t t;

	oldtz = getenv("TZ");
	if (datearg->tz != NULL && setenv("TZ", datearg->tz, 1) == 0)
		tzset();
	t = time(NULL);
	now = localtime(&t);
	if (oldtz != NULL)
		setenv("TZ", oldtz, 1);
	else
		unsetenv("TZ");
	if (now == NULL)
		return -1;
	strftime(buf, len, datearg->fmt, now);
	return 0;
}
