#include <stddef.h>

#pragma weak battread
int
battread(void *arg, char *buf, size_t len)
{
	return -1;
}

#pragma weak cpuread
int
cpuread(void *arg, char *buf, size_t len)
{
	return -1;
}

#pragma weak mixread
int
mixread(void *arg, char *buf, size_t len)
{
	return -1;
}

#pragma weak mpdread
int
mpdread(void *arg, char *buf, size_t len)
{
	return -1;
}

#pragma weak tempread
int
tempread(void *arg, char *buf, size_t len)
{
	return -1;
}

#pragma weak wifiread
int
wifiread(void *arg, char *buf, size_t len)
{
	return -1;
}

#pragma weak xkblayoutread
int
xkblayoutread(void *arg, char *buf, size_t len)
{
	return -1;
}
