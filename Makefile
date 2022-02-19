CC=gcc
CFLAGS=-Wall -Wextra -Werror -pedantic -std=c11

MAIN=main

.PHONY: all clean

all:
	$(CC) $(MAIN).c $(CFLAGS) -o hinfosvc 

clean:
	rm -f *.o hinfosvc