#include <err.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "util.h"

void
wifiprint(char *buf, size_t len, int quality)
{
	char *icon;

	if (quality == 100)
		icon = "::";
	else if (quality >= 75)
		icon = ":.";
	else if (quality >= 50)
		icon = "..";
	else if (quality >= 25)
		icon = ". ";
	else
		icon = "  ";
	snprintf(buf, len, "%s", icon);
}

#ifdef __OpenBSD__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <net/if_media.h>
#include <net80211/ieee80211.h>
#include <net80211/ieee80211_ioctl.h>

int
wifiread(void *arg, char *buf, size_t len)
{
	struct ifaddrs *ifa, *ifas;
	struct ifmediareq ifmr;
	struct ieee80211_nodereq nr;
	struct ieee80211_bssid bssid;
	int s, ibssid, quality;
	char *icon;

	if (getifaddrs(&ifas) < 0) {
		warn("getifaddrs");
		return -1;
	}
	for (ifa = ifas; ifa; ifa = ifa->ifa_next) {
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s < 0) {
			warn("socket");
			continue;
		}
		memset(&ifmr, 0, sizeof(ifmr));
		strlcpy(ifmr.ifm_name, ifa->ifa_name, IF_NAMESIZE);
		if (ioctl(s, SIOCGIFMEDIA, (caddr_t)&ifmr) < 0) {
			close(s);
			continue;
		}
		if ((ifmr.ifm_active & IFM_IEEE80211) == 0) {
			close(s);
			continue;
		}
		if ((ifmr.ifm_active & IFM_IEEE80211_HOSTAP) != 0) {
			close(s);
			continue;
		}
		memset(&bssid, 0, sizeof(bssid));
		strlcpy(bssid.i_name, ifa->ifa_name, sizeof(bssid.i_name));
		ibssid = ioctl(s, SIOCG80211BSSID, &bssid);
		if (ibssid < 0) {
			close(s);
			continue;
		}
		memset(&nr, 0, sizeof(nr));
		memcpy(&nr.nr_macaddr, bssid.i_bssid, sizeof(nr.nr_macaddr));
		strlcpy(nr.nr_ifname, ifa->ifa_name, sizeof(nr.nr_ifname));
		if (ioctl(s, SIOCG80211NODE, &nr) < 0) {
			close(s);
			continue;
		}
		close(s);
		if (nr.nr_rssi == 0)
			continue;
		if (nr.nr_max_rssi == 0) {
			if (nr.nr_rssi <= -100)
				quality = 0;
			else if (nr.nr_rssi >= -50)
				quality = 100;
			else
				quality = 2 * (nr.nr_rssi + 100);
		} else {
			quality = IEEE80211_NODEREQ_RSSI(&nr);
		}
		wifiprint(buf, len, quality);
		break;
	}
	freeifaddrs(ifas);
	if (ifa)
		return 0;
	return -1;
}
#elif __linux__
#include <sys/types.h>
#include <sys/socket.h>

#include <linux/wireless.h>

int
wifiread(void *arg, char *buf, size_t len)
{
	struct ifaddrs *ifa, *ifas;
	struct iw_quality *max_qual, *qual;
	struct iw_statistics stats;
	struct iw_range range;
	struct iwreq wrq;
	int quality = -1;
	int level;
	int ret, fd;

	if (getifaddrs(&ifas) < 0) {
		warn("getifaddrs");
		return -1;
	}
	fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (fd == -1) {
		warn("socket");
		return -1;
	}
	for (ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {
		DPRINTF_S(ifa->ifa_name);
		memset(&wrq, 0, sizeof(wrq));
		strlcpy(wrq.ifr_name, ifa->ifa_name, IFNAMSIZ);
		ret = ioctl(fd, SIOCGIWNAME, &wrq);
		if (ret != 0)
			continue;
		memset(&wrq, 0, sizeof(wrq));
		strlcpy(wrq.ifr_name, ifa->ifa_name, IFNAMSIZ);
		wrq.u.data.pointer = &range;
		wrq.u.data.length = sizeof(range);
		memset(&range, 0, sizeof(range));
		ret = ioctl(fd, SIOCGIWRANGE, &wrq);
		if (ret < 0) {
			warnx("cannot get wifi range");
			continue;
		}
		memset(&wrq, 0, sizeof(wrq));
		strlcpy(wrq.ifr_name, ifa->ifa_name, IFNAMSIZ);
		wrq.u.data.pointer = &stats;
		wrq.u.data.length = sizeof(stats);
		wrq.u.data.flags = 1;
		memset(&stats, 0, sizeof(stats));
		ret = ioctl(fd, SIOCGIWSTATS, &wrq);
		if (ret < 0) {
			warnx("cannot get wifi stats");
			continue;
		}
		max_qual = &range.max_qual;
		qual = &stats.qual;
		DPRINTF_U(max_qual->qual);
		DPRINTF_U(max_qual->level);
		DPRINTF_U(qual->qual);
		DPRINTF_U(qual->level);
		if (max_qual->qual != 0) {
			/* driver provides a quality metric */
			quality = (((float)qual->qual / max_qual->qual) * 100);
		} else if (max_qual->level != 0) {
			/* driver provides signal strength (RSSI) */
			quality = (((float)qual->level / max_qual->level) * 100);
		} else if (max_qual->level == 0) {
			/* driver provides absolute dBm values */
			level = qual->level - 0x100;
			if (level <= -100)
				quality = 0;
			else if (level >= -50)
				quality = 100;
			else
				quality = 2 * (level + 100);
		}
		break;
	}
	close(fd);
	freeifaddrs(ifas);

	DPRINTF_D(quality);
	if (quality == -1)
		return -1;
	wifiprint(buf, len, quality);
	return 0;
}
#endif
