#include <err.h>
#include <stdio.h>

#include <X11/keysym.h>
#include <X11/Xlib.h>

#include "types.h"
#include "util.h"

int
keyread(void *arg, char *buf, size_t len)
{
	Display *dpy;
	XModifierKeymap *map;
	KeyCode keycode;
	Window w1, w2;
	int i1, i2, i3, i4;
	int modmask;
	int keymask;
	struct keyarg *key = arg;
	int on;
	int i;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		warnx("cannot open display");
		return -1;
	}
	keycode = XKeysymToKeycode(dpy, key->sym);
	if (keycode == NoSymbol) {
		warnx("no key code for this symbol");
		XCloseDisplay(dpy);
		return -1;
	}
	map = XGetModifierMapping(dpy);
	for (i = 0; i < 8; i++)
		if (map->modifiermap[map->max_keypermod * i] == keycode)
			keymask = 1 << i;
	XFreeModifiermap(map);
	XQueryPointer(dpy, DefaultRootWindow(dpy),
		      &w1, &w2, &i1, &i2, &i3, &i4, &modmask);
	XCloseDisplay(dpy);
	on = (keymask & modmask) != 0;
	DPRINTF_D(on);
	if (on)
		snprintf(buf, len, "%s", key->on);
	else
		snprintf(buf, len, "%s", key->off);
	return 0;
}
