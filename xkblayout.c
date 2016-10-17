#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

#include "util.h"

int
xkblayoutread(void *arg, char *buf, size_t len)
{
	Display *dpy;
	XkbStateRec state;
	XkbRF_VarDefsRec vd;
	char *tmp = NULL, *str, *tok;
	int i, ret = 0;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		warnx("cannot open display");
		return -1;
	}
	XkbGetState(dpy, XkbUseCoreKbd, &state);
	if (XkbRF_GetNamesProp(dpy, &tmp, &vd) == 0){
		warnx("cannot extract keyboard properties");
		ret = -1;
		goto out0;
	}
	str = strdup(vd.layout);
	if (str == NULL) {
		ret = -1;
		goto out1;
	}
	tok = strtok(str, ",");
	for (i = 0; i < state.group; i++) {
		tok = strtok(NULL, ",");
		if (tok == NULL) {
			warnx("cannot extract layout");
			ret = -1;
			goto out2;
		}
	}
	strlcpy(buf, tok, len);
out2:
	free(str);
out1:
	free(tmp);
	XFree(vd.options);
out0:
	XCloseDisplay(dpy);
	return ret;
}
