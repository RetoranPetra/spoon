#include <err.h>
#include <stdio.h>

#include "types.h"
#include "util.h"

char *crit[] = {
	"=[!!!!]",
	"       ",
};

void
battprint(char *buf, size_t len, int acon, int life)
{
	char c;
	static int frame = 0;

	c = acon ? '>' : '<';
	if (!acon && life <= 5)
		snprintf(buf, len, "%s", crit[frame++ % LEN(crit)]);
	else if (life >= 80)
		snprintf(buf, len, "=[////]");
	else if (life >= 60)
		snprintf(buf, len, "=[%c///]", c);
	else if (life >= 40)
		snprintf(buf, len, "=[%c%c/]", c, c);
	else if (life >= 20)
		snprintf(buf, len, "=[%c%c%c/]", c, c, c);
	else
		snprintf(buf, len, "=[%c%c%c%c]", c, c, c, c);
}

#ifdef __OpenBSD__
#include <sys/ioctl.h>

#include <fcntl.h>
#include <unistd.h>

#include <machine/apmvar.h>

int
battread(void *arg, char *buf, size_t len)
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

	if (info.battery_state == APM_BATTERY_ABSENT)
		snprintf(buf, len, "[no batt]");
	else
		battprint(buf, len, info.ac_state == APM_AC_ON, info.battery_life);
	return 0;
}
#elif __linux__
int
battread(void *arg, char *buf, size_t len)
{
	FILE *fp;
	int acon;
	int life;
	struct battarg *battarg = arg;

	fp = fopen(battarg->cap, "r");
	if (fp == NULL) {
		warn("fopen %s", battarg->cap);
		return -1;
	}
	fscanf(fp, "%d", &life);
	fclose(fp);
	fp = fopen(battarg->ac, "r");
	if (fp == NULL) {
		warn("fopen %s", battarg->ac);
		return -1;
	}
	fscanf(fp, "%d", &acon);
	fclose(fp);
	battprint(buf, len, acon, life);
	return 0;
}
#endif
