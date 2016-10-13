#include <stdlib.h>
#include <stdio.h>

int
loadread(char *buf, size_t len)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0)
		return -1;
	snprintf(buf, len, "%.2f %.2f %.2f",
	         avgs[0], avgs[1], avgs[2]);
	return 0;
}
