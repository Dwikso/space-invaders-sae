# Compilateur et flags
CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags sdl3` `pkg-config --cflags SDL3_image`
LDFLAGS = `pkg-config --libs sdl3` `pkg-config --libs SDL3_image` -lm

# Nom de l'exécutable
TARGET = space-invaders

# Fichiers sources
SRCS = src/model/window.c
OBJS = $(SRCS:.c=.o)

# Règle par défaut
all: $(TARGET)

# Compilation de l'exécutable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compilation des fichiers objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f $(OBJS) $(TARGET)

# Exécution
run: $(TARGET)
	./$(TARGET)

# Recompilation complète
re: clean all

.PHONY: all clean run re
