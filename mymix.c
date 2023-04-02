#include <stdio.h>

int
mymixread(void *arg, char *buf, size_t len) {
	FILE *fp;
	static char outBuffer[16] = {};
	//Open pulsemixer file.
	fp = popen("/usr/bin/pulsemixer --get-volume", "r");
	if (fp == NULL) {
		return -1;
	}
	fgets(outBuffer, sizeof(outBuffer), fp);
	//Checking for 100 100
	//						 0123456
	//Checking for 40 40
	for (int i = 0;i<sizeof(outBuffer);i++) {
		outBuffer[i] = (outBuffer[i] == '\n') ? '\0' : outBuffer[i];
	}
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
	fflush(fp);
	snprintf(buf,len,"%s",outBuffer);
	return 0;
}
