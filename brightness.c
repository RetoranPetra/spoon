#include <err.h>
#include <stdio.h>

#ifdef __OpenBSD__
#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <dev/wscons/wsconsio.h>

int
brightnessread(void *arg, char *buf, size_t len)
{
	struct wsdisplay_param dpyp;
	int fd;

	if ((fd = open("/dev/ttyC0", O_RDONLY)) == -1) {
		warn("couldn't open /dev/ttyC0");
		return -1;
	}
	dpyp.param = WSDISPLAYIO_PARAM_BRIGHTNESS;
	if (ioctl(fd, WSDISPLAYIO_GETPARAM, &dpyp) == -1) {
		warn("WSDISPLAYIO_PARAM_BRIGHTNESS ioctl");
		return -1;
	}
	close(fd);
	snprintf(buf, len, "%3d%%",
	    100 * (dpyp.curval - dpyp.min) / (dpyp.max - dpyp.min));

	return 0;
}
#elif __linux__
int
brightnessread(void *arg, char *buf, size_t len)
{
	return -1;
}
#endif
