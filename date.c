#include <sys/types.h>

#include <stdio.h>
#include <time.h>

int
dateread(void *arg, char *buf, size_t len)
{
	struct tm *now;
	time_t t;

	time(&t);
	now = localtime(&t);
	if (now == NULL)
		return -1;
	strftime(buf, len, arg, now);
	return 0;
}
