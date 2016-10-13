#include <stddef.h>

#pragma weak mpdread
int
mpdread(char *buf, size_t len)
{
	return -1;
}

#pragma weak xkblayoutread
int
xkblayoutread(char *buf, size_t len)
{
	return -1;
}
