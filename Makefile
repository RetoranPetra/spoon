VERSION = 0.5
PREFIX = /usr/local
SRC = spoon.c batt.c wifi.c cpu.c temp.c date.c load.c file.c key.c\
      netspeed.c strlcpy.c strlcat.c stub.c mix.c xkblayout.c mpd.c
OBJ = spoon.o batt.o wifi.o cpu.o temp.o date.o load.o file.o key.o\
      netspeed.o strlcpy.o strlcat.o stub.o
BIN = spoon
DISTFILES = $(SRC) types.h util.h config.def.h Makefile LICENSE configure

include config.mk

CPPFLAGS_OpenBSD = -I/usr/X11R6/include -I/usr/local/include
LDFLAGS_OpenBSD = -L/usr/X11R6/lib -L/usr/local/lib
CPPFLAGS_Linux =\
    -I/usr/local/include\
    -DPATH_BAT_CAP=\"/sys/class/power_supply/BAT0/capacity\"\
    -DPATH_AC_ONLINE=\"/sys/class/power_supply/AC/online\"\
    -DPATH_TEMP=\"/sys/class/hwmon/hwmon1/temp1_input\"\
    -DPATH_CPU_FREQ=\"/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq\"
CPPFLAGS = $(CPPFLAGS_$(UNAME))
LDFLAGS = $(LDFLAGS_$(UNAME))
LDLIBS = -lX11

# To remove extra compile time dependencies for unwanted plugins
# comment out the following sections.  The stub implementations
# from stub.c will be used instead.
OBJ += mix.o
# if ALSA
LDLIBS_Linux_mix = -lasound
CPPFLAGS += -DUSE_TINYALSA=0
# else TinyALSA
#LDLIBS_Linux_mix = -ltinyalsa
#CPPFLAGS += -DUSE_TINYALSA=1
LDLIBS += $(LDLIBS_$(UNAME)_mix)

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
