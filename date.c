#include <sys/types.h>

#include <stddef.h>
#include <stdio.h>
#include <time.h>

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
