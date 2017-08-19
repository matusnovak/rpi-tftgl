CC=gcc
AR=ar
DISPLAY?=ERROR
CFLAGS=-I/opt/vc/include -I. -Iinclude -O3
LDFLAGS=-L/opt/vc/lib -L. -lEGL -lGLESv2
prefix?=/usr/local

.PHONY: default all clean install

default: nanovg
all: default

nanovg: libnanovg.a

libnanovg.a: src/nanovg.o
	$(AR) rcs libnanovg.a src/nanovg.o

src/nanovg.o: src/nanovg.c
	$(CC) -c src/nanovg.c -o src/nanovg.o $(CFLAGS)
	
install: nanovg
	install -m 0755 libnanovg.a $(prefix)/lib
	install -m 0644 src/nanovg.h $(prefix)/include
	install -m 0644 src/nanovg_gl.h $(prefix)/include
	install -m 0644 src/nanovg_gl_utils.h $(prefix)/include
	
clean:
	-rm -f src/nanovg.o
	-rm -f libnanovg.a