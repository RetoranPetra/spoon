VERSION = 0.2
PREFIX = /usr/local
CPPFLAGS = -I/usr/X11R6/include -I/usr/local/include
LDFLAGS = -L/usr/X11R6/lib -L/usr/local/lib
LDLIBS = -lxkbfile -lX11 -lmpdclient
DISTFILES = spoon.c batt.c strlcpy.c strlcat.c util.h config.def.h\
            Makefile LICENSE
OBJ = spoon.o batt.o strlcpy.o strlcat.o
BIN = spoon

# Linux
#CPPFLAGS += -DPATH_BAT_CAP=\"/sys/class/power_supply/BAT0/capacity\"
#CPPFLAGS += -DPATH_AC_ONLINE=\"/sys/class/power_supply/AC/online\"

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
