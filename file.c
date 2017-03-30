#include <err.h>
#include <fcntl.h>
#include <unistd.h>

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
	n = read(fd, buf, len);
	close(fd);
	if (n == -1 || n == 0)
		return -1;
	else
		buf[n - 1] = '\0';
	return 0;
}
