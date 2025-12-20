# ==========================================
#   Makefile - Space Invaders (MVC)
# ==========================================

# --- 1. Variables de Compilation ---

CC = gcc

# Ajout de -Iexternal/SDL3_ttf/include dans CFLAGS
CFLAGS = -std=c99 -Wall -Wextra -g -Iinclude -Iexternal/SDL3/include -Iexternal/SDL3_image/include -Iexternal/SDL3_ttf/include

# Détection des drapeaux pour SDL3 via pkg-config
SDL_CFLAGS := $(shell pkg-config --cflags sdl3) 
# AJOUT : On ajoute -lSDL3_ttf ici
SDL_LIBS   := $(shell pkg-config --libs sdl3) -lSDL3_image -lSDL3_ttf

LDFLAGS = -lm -lncurses $(SDL_LIBS)

CFLAGS += $(SDL_CFLAGS)

# --- 2. Fichiers et Répertoires ---

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .

TARGET = $(BIN_DIR)/space_invaders

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- 3. Règles de Construction (Targets) ---

all: dirs $(TARGET)

$(TARGET): $(OBJS)
	@echo "🔗 Édition des liens : $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "✅ Compilation terminée avec succès !"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "🔨 Compilation de : $<"
	$(CC) $(CFLAGS) -c $< -o $@

dirs:
	@mkdir -p $(OBJ_DIR)

# --- 4. Commandes Utilitaires ---

clean:
	@echo "🧹 Nettoyage des fichiers de build..."
	rm -rf $(OBJ_DIR) $(TARGET)

run-ncurses: all
	./$(TARGET) ncurses

run-sdl: all
	./$(TARGET) sdl

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) ncurses

.PHONY: all clean run-ncurses run-sdl valgrind dirs