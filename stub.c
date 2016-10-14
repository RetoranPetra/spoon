#include <stddef.h>

#pragma weak battread
int
battread(char *buf, size_t len)
{
	return -1;
}

#pragma weak cpuread
int
cpuread(char *buf, size_t len)
{
	return -1;
}

#pragma weak mixread
int
mixread(char *buf, size_t len)
{
	return -1;
}

#pragma weak mpdread
int
mpdread(char *buf, size_t len)
{
	return -1;
}

#pragma weak tempread
int
tempread(char *buf, size_t len)
{
	return -1;
}

#pragma weak wifiread
int
wifiread(char *buf, size_t len)
{
	return -1;
}

#pragma weak xkblayoutread
int
xkblayoutread(char *buf, size_t len)
{
	return -1;
}
