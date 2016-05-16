VERSION = 0.1
PREFIX = /usr/local
CPPFLAGS = -I/usr/X11R6/include -I/usr/local/include
LDFLAGS = -L/usr/X11R6/lib -L/usr/local/lib
LDLIBS = -lxkbfile -lX11 -lmpdclient
DISTFILES = spoon.c Makefile LICENSE
OBJ = spoon.o
BIN = spoon

all: $(BIN)

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
