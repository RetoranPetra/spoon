#include <sys/types.h>

#include <stdio.h>
#include <time.h>

extern char timeformat[];

int
dateread(char *buf, size_t len)
{
	struct tm *now;
	time_t t;

	time(&t);
	now = localtime(&t);
	if (now == NULL)
		return -1;
	strftime(buf, len, timeformat, now);
	return 0;
}
