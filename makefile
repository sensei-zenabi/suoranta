CC = gcc
CFLAGS = -std=c11 -D_POSIX_C_SOURCE=200112L `sdl2-config --cflags` -lSDL2_ttf
LDFLAGS = `sdl2-config --libs` -lSDL2_image -lSDL2_ttf 

SRC = main.c $(wildcard *.c)
OBJ = $(SRC:.c=.o)
TARGET = game

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJ)
