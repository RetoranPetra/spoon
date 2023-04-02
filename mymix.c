#include <err.h>
#include <stdio.h>

#include "types.h"
#include "util.h"

int
mymixread(void *arg, char *buf, size_t len) {
	FILE *fp;
	static char outBuffer[8] = {};
	//Open pulsemixer file.
	
	fp = popen("/usr/bin/pulsemixer --get-volume", "r");
	if (fp == NULL) {
		warn("popen fail!");
		return -1;
	}
	fgets(outBuffer, sizeof(outBuffer), fp);
	warn("OutBuffer: %s",outBuffer);
	//snprintf(buf,len,"A!\n");
/*
	int i = 0;
	while(fgets(outBuffer, sizeof(outBuffer), fp) != NULL) {
		if (i==0) {
			snprintf(buf,len,"%s",outBuffer);
		}
		i++;
	}
*/
	pclose(fp);
	snprintf(buf,len,"%s",outBuffer);
	fflush(fp);
	
	return 0;
}
