LDFLAGS += -lixp

PREFIX?=/usr/local

CFLAGS+=-Wall -Wextra -Wpedantic -Wno-unused-parameter

all: wmii-clock

wmii-clock: main.o
	cc -o $@ $+ $(CFLAGS) $(LDFLAGS)

install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	install -m755 wmii-clock $(DESTDIR)$(PREFIX)/bin

.PHONY: all install
