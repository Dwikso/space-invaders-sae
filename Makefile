# ==========================================
#   Makefile - Space Invaders (MVC)
# ==========================================

# --- 1. Variables de Compilation ---

CC = gcc

# Nom du dossier des dépendances
EXT_DIR = 3rdParty

# Chemins d'inclusion (Headers)
# On utilise $(EXT_DIR) pour plus de flexibilité
CFLAGS = -std=c99 -Wall -Wextra -g -Iinclude \
         -I$(EXT_DIR)/SDL3/include \
         -I$(EXT_DIR)/SDL3_image/include \
         -I$(EXT_DIR)/SDL3_ttf/include \
         -I$(EXT_DIR)/SDL3_mixer/include

# Détection des drapeaux pour SDL3 via pkg-config
SDL_CFLAGS := $(shell pkg-config --cflags sdl3) 
# Bibliothèques SDL (ordre important)
SDL_LIBS   := $(shell pkg-config --libs sdl3) -lSDL3_ttf -lSDL3_image -lSDL3_mixer -lm

# LDFLAGS : Mise à jour des chemins vers $(EXT_DIR)
LDFLAGS = -lm -lncurses $(SDL_LIBS) \
          -L$(EXT_DIR)/SDL3_ttf/build -Wl,-rpath,$(EXT_DIR)/SDL3_ttf/build \
          -L$(EXT_DIR)/SDL3_mixer/build -Wl,-rpath,$(EXT_DIR)/SDL3_mixer/build

# --- 2. Fichiers et Répertoires ---

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .

TARGET = $(BIN_DIR)/space_invaders

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- 3. Règles de Construction (Targets) ---

all: dirs check-sdl3-ttf check-sdl3-mixer $(TARGET)

# --- GESTION SDL3_TTF ---
check-sdl3-ttf:
	@if [ ! -d "$(EXT_DIR)/SDL3_ttf/build" ]; then \
		echo "⚠️  SDL3_ttf n'est pas compilé dans $(EXT_DIR). Compilation..."; \
		$(MAKE) compile-sdl3-ttf; \
	fi

compile-sdl3-ttf:
	@echo "🔧 Compilation de SDL3_ttf..."
	@mkdir -p $(EXT_DIR)/SDL3_ttf/build
	@cd $(EXT_DIR)/SDL3_ttf/build && \
	cmake .. -DCMAKE_BUILD_TYPE=Release \
	         -DSDL3TTF_VENDORED=ON \
	         -DSDL3TTF_SAMPLES=OFF && \
	cmake --build . --parallel
	@echo "✅ SDL3_ttf compilé avec succès !"

# --- GESTION SDL3_MIXER ---
check-sdl3-mixer:
	@if [ ! -d "$(EXT_DIR)/SDL3_mixer/build" ]; then \
		echo "⚠️  SDL3_mixer n'est pas compilé dans $(EXT_DIR). Compilation..."; \
		$(MAKE) compile-sdl3-mixer; \
	fi

compile-sdl3-mixer:
	@echo "🔧 Compilation de SDL3_mixer..."
	@mkdir -p $(EXT_DIR)/SDL3_mixer/build
	@cd $(EXT_DIR)/SDL3_mixer/build && \
	cmake .. -DCMAKE_BUILD_TYPE=Release \
	         -DSDL3MIXER_VENDORED=ON \
	         -DSDL3MIXER_SAMPLES=OFF \
	         -DSDL3MIXER_VORBIS_VORBISFILE=OFF \
	         -DSDL3MIXER_VORBIS_STB=ON \
	         -DSDL3MIXER_MP3_DRMP3=ON && \
	cmake --build . --parallel
	@echo "✅ SDL3_mixer compilé avec succès !"

# --- COMPILATION DU JEU ---
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
	@echo "🧹 Nettoyage complet (y compris librairies dans $(EXT_DIR))..."
	rm -rf $(EXT_DIR)/SDL3_ttf/build
	rm -rf $(EXT_DIR)/SDL3_mixer/build
	rm -rf $(EXT_DIR)/SDL3_mixer/install

run-ncurses: all
	./$(TARGET) ncurses

run-sdl: all
	./$(TARGET) sdl

doc:
	doxygen Doxyfile

.PHONY: all clean doc clean-all run-ncurses run-sdl dirs check-sdl3-ttf compile-sdl3-ttf check-sdl3-mixer compile-sdl3-mixer