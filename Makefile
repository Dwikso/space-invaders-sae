# ==========================================
#   Makefile - Space Invaders (MVC)
# ==========================================

# --- 1. Variables de Compilation ---

CC = gcc

# Chemins d'inclusion
CFLAGS = -std=c99 -Wall -Wextra -g -Iinclude \
         -Iexternal/SDL3/include \
         -Iexternal/SDL3_image/include \
         -Iexternal/SDL3_ttf/include

# Détection des drapeaux pour SDL3 via pkg-config
SDL_CFLAGS := $(shell pkg-config --cflags sdl3) 

# Bibliothèques SDL (ordre important : SDL3_ttf avant SDL3_image)
SDL_LIBS   := $(shell pkg-config --libs sdl3) -lSDL3_ttf -lSDL3_image

# Ajout du chemin de recherche des bibliothèques si SDL3_ttf est dans external/
LDFLAGS = -lm -lncurses $(SDL_LIBS) -Lexternal/SDL3_ttf/build -Wl,-rpath,external/SDL3_ttf/build

CFLAGS += $(SDL_CFLAGS)

# --- 2. Fichiers et Répertoires ---

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .
EXTERNAL_DIR = external

TARGET = $(BIN_DIR)/space_invaders

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- 3. Règles de Construction (Targets) ---

all: dirs check-sdl3-ttf $(TARGET)

# Vérifie que SDL3_ttf est compilé
check-sdl3-ttf:
	@if [ ! -d "$(EXTERNAL_DIR)/SDL3_ttf/build" ]; then \
		echo "⚠️  SDL3_ttf n'est pas compilé. Compilation en cours..."; \
		$(MAKE) compile-sdl3-ttf; \
	fi

# Compile SDL3_ttf si nécessaire
compile-sdl3-ttf:
	@echo "🔧 Compilation de SDL3_ttf..."
	@cd $(EXTERNAL_DIR)/SDL3_ttf && \
	mkdir -p build && cd build && \
	cmake .. -DCMAKE_BUILD_TYPE=Release \
	         -DSDL3TTF_VENDORED=ON \
	         -DSDL3TTF_SAMPLES=OFF && \
	cmake --build .
	@echo "✅ SDL3_ttf compilé avec succès !"

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

clean-all: clean
	@echo "🧹 Nettoyage complet (y compris SDL3_ttf)..."
	rm -rf $(EXTERNAL_DIR)/SDL3_ttf/build

run-ncurses: all
	./$(TARGET) ncurses

run-sdl: all
	./$(TARGET) sdl

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) ncurses

.PHONY: all clean clean-all run-ncurses run-sdl valgrind dirs check-sdl3-ttf compile-sdl3-ttf