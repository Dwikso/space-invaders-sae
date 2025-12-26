# ==========================================
#   Makefile - Space Invaders (MVC)
# ==========================================

CC = gcc
EXT_DIR = 3rdParty
TARGET = space_invaders

# --- 1. Flags de Compilation ---
CFLAGS = -std=c99 -Wall -Wextra -g -Iinclude \
         -I$(EXT_DIR)/SDL3/include \
         -I$(EXT_DIR)/SDL3_image/include \
         -I$(EXT_DIR)/SDL3_ttf/include \
         -I$(EXT_DIR)/SDL3_mixer/include

# L'ordre des libs est crucial (les extensions AVANT le coeur SDL3)
LDFLAGS = -lm -lncurses \
          -L$(EXT_DIR)/SDL3_image/build -lSDL3_image \
          -L$(EXT_DIR)/SDL3_ttf/build -lSDL3_ttf \
          -L$(EXT_DIR)/SDL3_mixer/build -lSDL3_mixer \
          -L$(EXT_DIR)/SDL3/build -lSDL3 \
          -Wl,-rpath,$(abspath $(EXT_DIR)/SDL3_image/build) \
          -Wl,-rpath,$(abspath $(EXT_DIR)/SDL3_ttf/build) \
          -Wl,-rpath,$(abspath $(EXT_DIR)/SDL3_mixer/build) \
          -Wl,-rpath,$(abspath $(EXT_DIR)/SDL3/build)

# --- 2. Fichiers ---
SRC_DIR = src
OBJ_DIR = build
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- 3. Règles de Construction ---

all: dirs libs $(TARGET)

# Cibles d'exécution rapide
run-ncurses: all
	@echo "🚀 Lancement en mode NCURSES..."
	./$(TARGET) ncurses

run-sdl: all
	@echo "🚀 Lancement en mode SDL3..."
	./$(TARGET) sdl

# --- 4. Gestion des Dépendances ---

libs:
	@if [ ! -f "$(EXT_DIR)/SDL3/build/libSDL3.so" ]; then $(MAKE) compile-sdl3; fi
	@if [ ! -f "$(EXT_DIR)/SDL3_image/build/libSDL3_image.so" ]; then $(MAKE) compile-sdl3-image; fi
	@if [ ! -f "$(EXT_DIR)/SDL3_ttf/build/libSDL3_ttf.so" ]; then $(MAKE) compile-sdl3-ttf; fi
	@if [ ! -f "$(EXT_DIR)/SDL3_mixer/build/libSDL3_mixer.so" ]; then $(MAKE) compile-sdl3-mixer; fi

compile-sdl3:
	@echo "🔧 Compilation de SDL3 Core..."
	@mkdir -p $(EXT_DIR)/SDL3/build
	@cd $(EXT_DIR)/SDL3/build && cmake .. && cmake --build . --parallel

compile-sdl3-ttf:
	@cd $(EXT_DIR)/SDL3_ttf/build && cmake .. -DSDL3_DIR=$(abspath $(EXT_DIR)/SDL3/build) -DSDL3TTF_VENDORED=ON && cmake --build . --parallel

compile-sdl3-mixer:
	@cd $(EXT_DIR)/SDL3_mixer/build && cmake .. -DSDL3_DIR=$(abspath $(EXT_DIR)/SDL3/build) -DSDL3MIXER_VENDORED=ON && cmake --build . --parallel

compile-sdl3-image:
	@cd $(EXT_DIR)/SDL3_image/build && cmake .. -DSDL3_DIR=$(abspath $(EXT_DIR)/SDL3/build) -DSDL3IMAGE_VENDORED=ON && cmake --build . --parallel

# --- 5. Compilation du Projet ---

$(TARGET): $(OBJS)
	@echo "🔗 Édition des liens : $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

dirs:
	@mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean libs run-ncurses run-sdl compile-sdl3 compile-sdl3-ttf compile-sdl3-mixer compile-sdl3-image