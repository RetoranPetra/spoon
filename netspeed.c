#include <ctype.h>
#include <err.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

extern int delay;

static const char *humansztbl[] = { " B", "kB", "MB", "GB", "TB", "PB", "EB",
    "ZB", "YB" };

static int
humansz(size_t n)
{
	int i;
	for (i = 0; i < LEN(humansztbl) && n >= 1024; i++)
		n /= 1024;
	return i;
}

static void
updatenetspeed(char *buf, size_t len, uint64_t rxbytes, uint64_t txbytes)
{
	static uint64_t oldrxbytes, oldtxbytes;
	uint64_t rx, tx;
	int irx, itx;
	rx = (rxbytes - oldrxbytes) / delay;
	tx = (txbytes - oldtxbytes) / delay;
	irx = humansz(rx);
	itx = humansz(tx);
	snprintf(buf, len, "D:%6.1f%s/s U:%6.1f%s/s", rx
	    / (double)(1 << (10 * irx)), humansztbl[irx], tx
	    / (double)(1 << (10 * itx)), humansztbl[itx]);
	oldrxbytes = rxbytes;
	oldtxbytes = txbytes;
}

#ifdef __OpenBSD__
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h> /* struct if_data */
#include <ifaddrs.h>

int
netspeedread(void *arg, char *buf, size_t len)
{
	char *ifa_name = arg;
	struct if_data *ifa_data;
	struct ifaddrs *ifa, *ifap;
	uint64_t rxbytes, txbytes;

	if (getifaddrs(&ifap) == -1) {
		warn("getifaddrs");
		return -1;
	}

	rxbytes = txbytes = 0;

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if (strcmp(ifa_name, ifa->ifa_name) != 0)
			continue;
		if (ifa->ifa_data == NULL)
			continue;
		ifa_data = ifa->ifa_data;
		rxbytes += ifa_data->ifi_ibytes;
		txbytes += ifa_data->ifi_obytes;
	}

	updatenetspeed(buf, len, rxbytes, txbytes);
	freeifaddrs(ifap);

	return 0;
}
#elif __linux__
int
netspeedread(void *arg, char *buf, size_t len)
{
	char path[PATH_MAX];
	FILE *fp;
	char *ifname = arg;
	unsigned long long rxbytes, txbytes;

	(void)snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/rx_bytes", ifname);
	if (!(fp = fopen(path, "r"))) {
		warn("fopen %s", path);
		return -1;
	}
	if (fscanf(fp, "%llu", &rxbytes) != 1) {
		warn("fscanf %s", path);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	(void)snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_bytes", ifname);
	if (!(fp = fopen(path, "r"))) {
		warn("fopen %s", path);
		return -1;
	}
	if (fscanf(fp, "%llu", &txbytes) != 1) {
		warn("fscanf %s", path);
		fclose(fp);
		return -1;
	}
	fclose(fp);

	updatenetspeed(buf, len, rxbytes, txbytes);
	return 0;
}
#endif
