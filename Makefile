CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g

all: app-launcher

app-launcher:
	$(CC) $(CFLAGS) *.c -o app-launcher