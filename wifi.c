#include <err.h>
#include <stdio.h>

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

#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>

int
wifiread(char *buf, size_t len)
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
#else
int
wifiread(char *buf, size_t len)
{
	return -1;
}
#endif
