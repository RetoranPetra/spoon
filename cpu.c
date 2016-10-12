#include <stddef.h>
#include <stdio.h>

#ifdef __OpenBSD__
#include <sys/sysctl.h>

int
cpuread(char *buf, size_t len)
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
#else
int
cpuread(char *buf, size_t len)
{
	return -1;
}
#endif
