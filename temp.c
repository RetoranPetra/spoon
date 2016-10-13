#include <err.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __OpenBSD__
#include <sys/sysctl.h>
#include <sys/sensors.h>

int
tempread(char *buf, size_t len)
{
	int mib[5];
	struct sensor temp;
	size_t sz;

	mib[0] = CTL_HW;
	mib[1] = HW_SENSORS;
	mib[2] = 0; /* cpu0 */
	mib[3] = SENSOR_TEMP;
	mib[4] = 0; /* temp0 */
	sz = sizeof(temp);
	if (sysctl(mib, 5, &temp, &sz, NULL, 0) == -1)
		return -1;
	snprintf(buf, len, "%ddegC", (temp.value - 273150000) / 1000000);
	return 0;
}
#else
int
tempread(char *buf, size_t len)
{
	FILE *fp;
	int temp;

	fp = fopen(PATH_TEMP, "r");
	if (fp == NULL) {
		warn("fopen %s", PATH_TEMP);
		return -1;
	}
	fscanf(fp, "%d", &temp);
	fclose(fp);
	snprintf(buf, len, "%ddegC", temp / 1000);
	return 0;
}
#endif
