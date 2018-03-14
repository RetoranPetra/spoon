/* delay between each update in seconds */
int delay = 1;

struct ent ents[] = {
	/* reorder/remove these as you see fit */
	{ .fmt = "[%s] ",	.read = mpdread,	.arg = &(struct mpdarg){ .host = NULL, .port = 0 } },
	{ .fmt = "[%s] ",	.read = mixread,	.arg = NULL },
	{ .fmt = "[%s] ",	.read = loadread,	.arg = NULL },
	{ .fmt = "[%s] ",	.read = cpuread,	.arg = NULL },
	{ .fmt = "[%s°] ",	.read = tempread,	.arg = NULL },
	{ .fmt = "%s ",		.read = battread,	.arg = &(struct battarg){ .cap = "/sys/class/power_supply/BAT0/capacity", .ac = "/sys/class/power_supply/AC/online" } },
	{ .fmt = "%s ",		.read = wifiread,	.arg = NULL },
	{ .fmt = "[%s] ",	.read = xkblayoutread,	.arg = NULL },
	{ .fmt = "%s",		.read = keyread,	.arg = &(struct keyarg){ .sym = XK_Caps_Lock, .on = "[caps] ", .off = "" } },
	{ .fmt = "%s ",		.read = fileread,	.arg = "/etc/myname" },
	{ .fmt = "%s",		.read = dateread,	.arg = (char []){"%a %d %b %Y %H:%M %Z"} },
};
