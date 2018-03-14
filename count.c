#include <dirent.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

int
countread(void *arg, char *buf, size_t len)
{
        char *path = arg;
        unsigned long count = 0;
        struct dirent *e;
        DIR *d;

        if ((d = opendir(path)) == NULL) {
                warn("open %s", path);
                return -1;
        }

        while ((e = readdir(d)) != NULL) {
                if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
                        continue;
                ++count;
        }
        closedir(d);

        snprintf(buf, len, "%lu", count);

        return 0;
}
