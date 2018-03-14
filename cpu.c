#include <err.h>
#include <stdio.h>

#include "types.h"

#ifdef __OpenBSD__
#include <sys/sysctl.h>

int
cpuread(void *arg, char *buf, size_t len)
{
	int mib[2], cpuspeed;
	size_t sz;

	mib[0] = CTL_HW;
	mib[1] = HW_CPUSPEED;
	sz = sizeof(cpuspeed);
	if (sysctl(mib, 2, &cpuspeed, &sz, NULL, 0) < 0)
		return -1;
	snprintf(buf, len, "%4dMHz", cpuspeed);
	return 0;
}
#elif __linux__
int
cpuread(void *arg, char *buf, size_t len)
{
	struct cpuarg *cpuarg = arg;
	FILE *fp;
	int freq;

	fp = fopen(cpuarg->freq, "r");
	if (fp == NULL) {
		warn("fopen %s", cpuarg->freq);
		return -1;
	}
	fscanf(fp, "%d", &freq);
	fclose(fp);
	freq /= 1000;
	snprintf(buf, len, "%4dMHz", freq);
	return 0;
}
#endif
