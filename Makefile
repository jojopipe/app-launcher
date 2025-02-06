CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g
RAYLIB_FLAGS = -lraylib -DRAYGUI_IMPLEMENTATION -lGL -lm -lpthread -ldl -lrt -lX11

all: app-launcher

app-launcher: main.c
	$(CC) $(CFLAGS) $(RAYLIB_FLAGS) *.c -o app-launcher