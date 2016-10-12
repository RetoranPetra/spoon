VERSION = 0.2
PREFIX = /usr/local
DISTFILES = spoon.c batt.c wifi.c strlcpy.c strlcat.c util.h config.def.h\
            Makefile LICENSE configure
OBJ = spoon.o batt.o wifi.o strlcpy.o strlcat.o
BIN = spoon

include config.mk

CPPFLAGS_OpenBSD = -I/usr/X11R6/include -I/usr/local/include
LDFLAGS_OpenBSD = -L/usr/X11R6/lib -L/usr/local/lib
CPPFLAGS_Linux = -DPATH_BAT_CAP=\"/sys/class/power_supply/BAT0/capacity\"\
                 -DPATH_AC_ONLINE=\"/sys/class/power_supply/AC/online\"
CPPFLAGS = $(CPPFLAGS_$(UNAME))
LDFLAGS = $(LDFLAGS_$(UNAME))
LDLIBS = -lxkbfile -lX11 -lmpdclient

all: $(BIN)

spoon.o: config.h

config.h:
	cp config.def.h $@

clean:
	rm -f $(OBJ) $(BIN) $(BIN)-$(VERSION).tar.gz

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

dist:
	mkdir -p $(BIN)-$(VERSION)
	cp $(DISTFILES) $(BIN)-$(VERSION)
	tar -cf $(BIN)-$(VERSION).tar $(BIN)-$(VERSION)
	gzip $(BIN)-$(VERSION).tar
	rm -rf $(BIN)-$(VERSION)

.PHONY: all clean install uninstall dist

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)
