CC = gcc
CFLAGS = -c -O0 -g
LD = gcc
LDFLAGS = -O0 -g


all: arithmatic_machine

arithmatic_machine: main.c
	$(CC) $(CFLAGS) main.c -o main.o
	$(LD) $(LDFLAGS) main.o -o $@

clean:
	rm -f *.o
