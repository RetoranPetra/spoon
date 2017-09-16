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
	snprintf(buf, len, "v%6.1f%s/s ^%6.1f%s/s", rx
	    / (double)(1 << (10 * irx)), humansztbl[irx], tx
	    / (double)(1 << (10 * itx)), humansztbl[itx]);
	oldrxbytes = rxbytes;
	oldtxbytes = txbytes;
}

#ifdef __OpenBSD__
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <sys/sysctl.h>

int
netspeedread(void *arg, char *buf, size_t len)
{
	int mib[6] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
	char *ifname = arg;
	struct rt_msghdr *rtm;
	struct if_msghdr *ifm;
	struct sockaddr_dl *sdl;
	void *lim, *next, *rtmraw;
	uint64_t rxbytes, txbytes;
	size_t sz;

	if (sysctl(mib, 6, NULL, &sz, NULL, 0) < 0)
		return -1;
	if (!(rtmraw = malloc(sz)))
		return -1;
	if (sysctl(mib, 6, rtmraw, &sz, NULL, 0) < 0) {
		free(rtmraw);
		return -1;
	}
	lim = rtmraw + sz;
	rxbytes = txbytes = 0;
	for (next = rtmraw; next < lim; next += rtm->rtm_msglen) {
		rtm = (struct rt_msghdr *)next;
		if (rtm->rtm_version != RTM_VERSION || rtm->rtm_type
		    != RTM_IFINFO)
			continue;
		ifm = (struct if_msghdr *)next;
		sdl = (struct sockaddr_dl *)(next + ifm->ifm_hdrlen);
		if (sdl->sdl_family != AF_LINK
		    || strncmp(ifname, sdl->sdl_data, sdl->sdl_nlen) != 0)
			continue;
		rxbytes = ifm->ifm_data.ifi_ibytes;
		txbytes = ifm->ifm_data.ifi_obytes;
	}
	updatenetspeed(buf, len, rxbytes, txbytes);
	free(rtmraw);

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
		goto err;
	}
	if (fscanf(fp, "%llu", &rxbytes) != 1) {
		warn("fscanf %s", path);
		goto err;
	}
	fclose(fp);
	(void)snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/tx_bytes", ifname);
	if (!(fp = fopen(path, "r"))) {
		warn("fopen %s", path);
		goto err;
	}
	if (fscanf(fp, "%llu", &txbytes) != 1) {
		warn("fscanf %s", path);
		goto err;
	}
	fclose(fp);

	updatenetspeed(buf, len, rxbytes, txbytes);
	return 0;

err:
	fclose(fp);
	return -1;
}
#endif
