CFLAGS=-Wall -std=c99
INCLUDE=../libraries/SROFS

all:
	gcc $(CFLAGS) -I$(INCLUDE) -o mksrofs -O2 mksrofs.c

debug:
	gcc $(CFLAGS) -g -I$(INCLUDE) -o mksrofs mksrofs.c
