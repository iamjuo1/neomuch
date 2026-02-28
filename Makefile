.PHONY: clean

BIN      = $(shell basename *.c .c)
CC       = gcc
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Wshadow
CPPFLAGS = -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=700
LDFLAGS  = -pie
LDLIBS   = -lnotmuch

$(BIN): $(BIN).o

$(BIN).o: termbox2.h config.h

config.h:
	cp config.def.h config.h

termbox2.h:
	curl -O https://raw.githubusercontent.com/termbox/termbox2/refs/heads/master/termbox2.h

clean:
	rm $(BIN) *.o
