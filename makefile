CC=gcc
CFLAGS= -O3 -Wall -std=c11 -Werror
CFLAGSPEDANTIC = -Wextra -Wpedantic
all: main.c
	$(CC) -o loesung $(CFLAGS) main.c
pedantic: main.c
	$(CC) -o loesung $(CFLAGS) $(CFLAGSPEDANTIC) main.c
