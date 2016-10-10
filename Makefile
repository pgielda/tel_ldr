UNAME=$(shell uname)

CFLAGS=-m32 -g -Wall -W -ansi
LDFLAGS+=-rdynamic
ifeq ($(UNAME),Linux)
	LDFLAGS+=-ldl
	LDFLAGS+=-Wl,-Ttext-segment=0x2000000
endif

all: el

%.o: %.c Makefile
	$(CC) $(CFLAGS) -ansi -c $< -o $@

el: el.o
	$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@

clean:
	rm -f el *.o
