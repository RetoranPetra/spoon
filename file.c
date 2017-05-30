#include <err.h>
#include <fcntl.h>
#include <unistd.h>

ssize_t
readn(int fd, void *buf, size_t nbytes)
{
	size_t nleft = nbytes;
	ssize_t n;

	do {
		n = read(fd, buf, nleft);
		if (n == 0)
			break;
		else if (n == -1)
			return -1;
		nleft -= n;
		buf += n;
	} while (nleft > 0);
	return (nbytes - nleft);
}

int
fileread(void *arg, char *buf, size_t len)
{
	char *path = arg;
	ssize_t n;
	int fd;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		warn("open %s", path);
		return -1;
	}
	n = readn(fd, buf, len);
	close(fd);
	if (n == -1 || n == 0)
		return -1;
	else
		buf[n - 1] = '\0';
	return 0;
}
