struct battarg {
	char *cap;
	char *ac;
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

struct datearg {
	char *fmt;
	char *tz;
};
