CC = gcc
TARGET = lumina
SRCS = main.c src/collision.c src/createlevel.c src/game.c src/level.c src/menu.c src/text.c \
       src/player.c src/events.c src/render_game.c src/init.c cJSON.c
INCLUDES = -Iinclude $(shell sdl2-config --cflags)
LIBS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf

$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS) $(INCLUDES) $(LIBS)

clean:
	rm -f $(TARGET)
