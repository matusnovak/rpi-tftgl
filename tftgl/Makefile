CC=gcc
AR=ar
DISPLAY?=ERROR
CFLAGS=-I/opt/vc/include -I. -Iinclude -DDISPLAY="$(DISPLAY)" -O3
prefix?=/usr/local

.PHONY: default all clean

default: tftgl
all: default

tftgl: libtftgl.a

libtftgl.a: src/tftgl.o
	$(AR) rcs libtftgl.a src/tftgl.o

src/tftgl.o: src/tftgl.c src/tftgl_ssd1963.h src/tftgl_ads7843.h
	$(CC) -c src/tftgl.c -o src/tftgl.o $(CFLAGS)
	
install: tftgl
	install -m 0755 libtftgl.a $(prefix)/lib
	install -m 0644 include/tftgl.h $(prefix)/include
	
clean:
	-rm -f src/*.o
	-rm -f libtftgl.a
