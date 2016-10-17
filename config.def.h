/* delay between each update in seconds */
int delay = 1;

struct ent ents[] = {
	/* reorder/remove these as you see fit */
	{ .fmt = "[%s] ",	.read = mpdread,	.arg = &(struct mpdarg){ .host = NULL, .port = 0 } },
	{ .fmt = "[%s] ",	.read = mixread,	.arg = NULL },
	{ .fmt = "[%s] ",	.read = loadread,	.arg = NULL },
	{ .fmt = "[%s] ",	.read = cpuread,	.arg = NULL },
	{ .fmt = "[%s] ",	.read = tempread,	.arg = NULL },
	{ .fmt = "%s ",		.read = battread,	.arg = NULL },
	{ .fmt = "%s ",		.read = wifiread,	.arg = NULL },
	{ .fmt = "[%s] ",	.read = xkblayoutread,	.arg = NULL },
	{ .fmt = "%s",		.read = dateread,	.arg = (char []){"%a %d %b %Y %H:%M %Z"} },
};
