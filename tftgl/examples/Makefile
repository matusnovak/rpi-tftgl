CC=gcc
AR=ar
CFLAGS=-I/opt/vc/include -I.
LDFLAGS=-L/opt/vc/lib -L. -lEGL -lGLESv2 -ltftgl -lbcm2835

.PHONY: default all clean

default: init triangle nano calibrate
all: default
	
init: init.o
	$(CC) -o init init.o $(LDFLAGS)
	
triangle: triangle.o
	$(CC) -o triangle triangle.o $(LDFLAGS)
	
nano: nano.o
	$(CC) -o nano nano.o $(LDFLAGS) -lnanovg -lm
	
calibrate: calibrate.o
	$(CC) -o calibrate calibrate.o $(LDFLAGS) -lnanovg -lm
	
%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
clean:
	-rm -f init init.o
	-rm -f triangle triangle.o
	-rm -f nano nano.o
	-rm -f calibrate calibrate.o