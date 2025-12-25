# ==========================================
#   Makefile - Space Invaders (MVC)
# ==========================================

# --- 1. Variables de Compilation ---

CC = gcc

# Chemins d'inclusion (Headers)
CFLAGS = -std=c99 -Wall -Wextra -g -Iinclude \
         -Iexternal/SDL3/include \
         -Iexternal/SDL3_image/include \
         -Iexternal/SDL3_ttf/include \
         -Iexternal/SDL3_mixer/include
LDFLAGS = -lm -lncurses $(SDL_LIBS) \
          -Lexternal/SDL3_ttf/build -Wl,-rpath,external/SDL3_ttf/build \
          -Lexternal/SDL3_mixer/install/lib -Wl,-rpath,external/SDL3_mixer/install/lib

# Détection des drapeaux pour SDL3 via pkg-config
SDL_CFLAGS := $(shell pkg-config --cflags sdl3) 

# Bibliothèques SDL (ordre important)
SDL_LIBS   := $(shell pkg-config --libs sdl3) -lSDL3_ttf -lSDL3_image -lSDL3_mixer -lm

# Ajout des chemins de recherche des bibliothèques (.a / .so / .dylib)
# On ajoute SDL3_mixer/build ici !


# --- 2. Fichiers et Répertoires ---

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .
EXTERNAL_DIR = external

TARGET = $(BIN_DIR)/space_invaders

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- 3. Règles de Construction (Targets) ---

# On ajoute check-sdl3-mixer ici
all: dirs check-sdl3-ttf check-sdl3-mixer $(TARGET)

# --- GESTION SDL3_TTF ---
check-sdl3-ttf:
	@if [ ! -d "$(EXTERNAL_DIR)/SDL3_ttf/build" ]; then \
		echo "⚠️  SDL3_ttf n'est pas compilé. Compilation en cours..."; \
		$(MAKE) compile-sdl3-ttf; \
	fi

compile-sdl3-ttf:
	@echo "🔧 Compilation de SDL3_ttf..."
	@cd $(EXTERNAL_DIR)/SDL3_ttf && \
	mkdir -p build && cd build && \
	cmake .. -DCMAKE_BUILD_TYPE=Release \
	         -DSDL3TTF_VENDORED=ON \
	         -DSDL3TTF_SAMPLES=OFF && \
	cmake --build .
	@echo "✅ SDL3_ttf compilé avec succès !"

# --- GESTION SDL3_MIXER (NOUVEAU) ---
check-sdl3-mixer:
	@if [ ! -d "$(EXTERNAL_DIR)/SDL3_mixer/build" ]; then \
		echo "⚠️  SDL3_mixer n'est pas compilé. Compilation en cours..."; \
		$(MAKE) compile-sdl3-mixer; \
	fi

compile-sdl3-mixer:
	@echo "🔧 Compilation de SDL3_mixer..."
	@cd $(EXTERNAL_DIR)/SDL3_mixer && \
	mkdir -p build && cd build && \
	cmake .. -DCMAKE_BUILD_TYPE=Release \
	         -DSDL3MIXER_VENDORED=ON \
	         -DSDL3MIXER_SAMPLES=OFF \
	         -DSDL3MIXER_FLAC=OFF \
	         -DSDL3MIXER_MOD=OFF \
	         -DSDL3MIXER_MIDI=OFF \
	         -DSDL3MIXER_OPUS=OFF && \
	cmake --build .
	@echo "✅ SDL3_mixer compilé avec succès !"
# Note: J'ai désactivé FLAC/MOD/MIDI/OPUS pour simplifier la compilation et éviter des erreurs de dépendances.
# WAV et MP3 marcheront.

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
	@echo "🧹 Nettoyage complet (y compris librairies externes)..."
	rm -rf $(EXTERNAL_DIR)/SDL3_ttf/build
	rm -rf $(EXTERNAL_DIR)/SDL3_mixer/build

run-ncurses: all
	./$(TARGET) ncurses

run-sdl: all
	./$(TARGET) sdl

doc:
	doxygen Doxyfile

.PHONY: all clean doc clean-all run-ncurses run-sdl dirs check-sdl3-ttf compile-sdl3-ttf check-sdl3-mixer compile-sdl3-mixer