VERSION = 0.2
PREFIX = /usr/local
SRC = spoon.c batt.c wifi.c cpu.c temp.c mix.c date.c load.c\
      strlcpy.c strlcat.c stub.c xkblayout.c mpd.c
OBJ = spoon.o batt.o wifi.o cpu.o temp.o mix.o date.o load.o\
      strlcpy.o strlcat.o stub.o
BIN = spoon
DISTFILES = $(SRC) types.h util.h config.def.h Makefile LICENSE configure

include config.mk

CPPFLAGS_OpenBSD = -I/usr/X11R6/include -I/usr/local/include
LDFLAGS_OpenBSD = -L/usr/X11R6/lib -L/usr/local/lib
LDLIBS_OpenBSD = -lX11
CPPFLAGS_Linux =\
    -I/usr/local/include\
    -DPATH_BAT_CAP=\"/sys/class/power_supply/BAT0/capacity\"\
    -DPATH_AC_ONLINE=\"/sys/class/power_supply/AC/online\"\
    -DPATH_TEMP=\"/sys/class/hwmon/hwmon0/temp1_input\"\
    -DPATH_CPU_FREQ=\"/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq\"
LDFLAGS_Linux = -L/usr/local/lib
LDLIBS_Linux = -lX11 -lasound
CPPFLAGS = $(CPPFLAGS_$(UNAME))
LDFLAGS = $(LDFLAGS_$(UNAME))
LDLIBS = $(LDLIBS_$(UNAME))

# To remove extra compile time dependencies for unwanted plugins
# comment out the following sections.  The stub implementations
# from stub.c will be used instead.
OBJ += xkblayout.o
LDLIBS += -lxkbfile

OBJ += mpd.o
LDLIBS += -lmpdclient

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
