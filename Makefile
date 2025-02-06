CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
RAYLIB_FLAGS = -lraylib -DRAYGUI_IMPLEMENTATION -lGL -lm -lpthread -ldl -lrt -lX11

all: app-launcher

app-launcher: main.o
	$(CC) $(CFLAGS) $(RAYLIB_FLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^
