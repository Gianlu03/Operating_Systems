CC=gcc
cflags=-Wall -std=c17

all:main mycat

main: myCatConFork.c 
	$(CC) $(cflags) -o $@ $<

mycat: mycat.c
	$(CC) $(cflags) -o $@ $<