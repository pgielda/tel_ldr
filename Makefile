UNAME=$(shell uname)

CFLAGS=-m32
LDFLAGS+=-rdynamic
ifeq ($(UNAME),Linux)
	LDFLAGS+=-ldl
	LDFLAGS+=-lstdc++
	LDFLAGS+=-Wl,-Ttext-segment=0x2000000
endif

all: el

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@

el: el.o mac_libc.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	rm -f el *.o
