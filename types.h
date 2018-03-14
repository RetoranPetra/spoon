struct battarg {
	char *cap;
	char *ac;
};

struct cpuarg {
	char *freq;
};

struct mpdarg {
	char *host;
	unsigned int port;
};

struct keyarg {
	int sym;
	char *on;
	char *off;
};
