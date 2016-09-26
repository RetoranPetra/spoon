#include <stddef.h>
#include <stdio.h>

static void
battprint(char *buf, size_t len, int acon , int life)
{
	char c;

	c = acon ? '>' : '<';
	if (life == 100)
		snprintf(buf, len, "[////]=");
	else if (life >= 75)
		snprintf(buf, len, "[///%c]=", c);
	else if (life >= 50)
		snprintf(buf, len, "[//%c%c]=", c, c);
	else if (life >= 25)
		snprintf(buf, len, "[/%c%c%c]=", c, c, c);
	else
		snprintf(buf, len, "[%c%c%c%c]=", c, c, c, c);
}

#ifdef __OpenBSD__
#include <sys/ioctl.h>

#include <err.h>
#include <fcntl.h>
#include <unistd.h>

#include <machine/apmvar.h>

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
	battprint(buf, len, info.ac_state == APM_AC_ON, info.battery_life);
	return 0;
}
#elif __linux__
int
battread(char *buf, size_t len)
{
	FILE *fp;
	int acon;
	int life;

	fp = fopen(PATH_BAT_CAP, "r");
	if (fp == NULL) {
		warn("fopen %s", PATH_BAT_CAP);
		return -1;
	}
	fscanf(fp, "%d", &life);
	fclose(fp);
	fp = fopen(PATH_AC_ONLINE, "r");
	if (fp == NULL) {
		warn("fopen %s", PATH_AC_ONLINE);
		return -1;
	}
	fscanf(fp, "%d", &acon);
	fclose(fp);
	battprint(buf, len, acon, life);
	return 0;
}
#else
int
battread(char *buf, size_t len)
{
	return -1;
}
#endif