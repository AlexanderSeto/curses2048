CC = gcc
CFLAGS = -g -Wall -lcurses
TARGET = curses2048

curses2048: main.c
	$(CC) $(CFLAGS) -o curses2048 main.c

clean:
	rm curses2048
