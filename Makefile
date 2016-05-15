PREFIX = /usr/local
CFLAGS = -I/usr/X11R6/include -I/usr/local/include
LDFLAGS = -L/usr/X11R6/lib -L/usr/local/lib
LDLIBS = -lxkbfile -lX11 -lmpdclient
OBJ = spoon.o
BIN = spoon

all: $(BIN)

clean:
	rm -f $(OBJ) $(BIN)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

.PHONY: all clean install uninstall

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS) $(LDLIBS)
