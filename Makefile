# ==========================================
#   Makefile - Space Invaders (MVC)
# ==========================================

# --- 1. Variables de Compilation ---

# Compilateur
CC = gcc

# Options de compilation :
# -std=c99   : Standard C99
# -Wall      : Active tous les avertissements
# -Wextra    : Avertissements supplémentaires
# -g         : Ajoute les infos de débogage (indispensable pour Valgrind/GDB)
# -Iinclude  : Indique où trouver les fichiers .h
CFLAGS = -std=c99 -Wall -Wextra -g -Iinclude -Iexternal/SDL3/include -Iexternal/SDL3_image/include 
# Détection des drapeaux pour SDL3 via pkg-config
# (Assurez-vous d'avoir installé libsdl3-dev)
SDL_CFLAGS := $(shell pkg-config --cflags sdl3) 
SDL_LIBS   := $(shell pkg-config --libs sdl3) -lSDL3_image 

# Bibliothèques à lier (Linker flags)
# -lm        : Bibliothèque mathématique standard
# -lncurses  : Bibliothèque Ncurses
LDFLAGS = -lm -lncurses $(SDL_LIBS)

# Ajout des flags SDL aux CFLAGS généraux pour que les #include <SDL3/...> fonctionnent
CFLAGS += $(SDL_CFLAGS)

# --- 2. Fichiers et Répertoires ---

# Répertoires
SRC_DIR = src
OBJ_DIR = build
BIN_DIR = .

# Nom de l'exécutable final
TARGET = $(BIN_DIR)/space_invaders

# Trouve automatiquement tous les fichiers .c dans src/
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Génère la liste des fichiers objets .o correspondants dans build/
# (Ex: src/main.c devient build/main.o)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# --- 3. Règles de Construction (Targets) ---

# Cible par défaut (ce qui s'exécute quand on tape juste "make")
all: dirs $(TARGET)

# Crée l'exécutable en liant tous les objets
$(TARGET): $(OBJS)
	@echo "🔗 Édition des liens : $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "✅ Compilation terminée avec succès !"

# Compile chaque fichier source .c en fichier objet .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "🔨 Compilation de : $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Crée le répertoire build s'il n'existe pas
dirs:
	@mkdir -p $(OBJ_DIR)

# --- 4. Commandes Utilitaires ---

# Nettoyage des fichiers compilés
clean:
	@echo "🧹 Nettoyage des fichiers de build..."
	rm -rf $(OBJ_DIR) $(TARGET)

# Lancer en mode Ncurses
run-ncurses: all
	./$(TARGET) ncurses

# Lancer en mode SDL
run-sdl: all
	./$(TARGET) sdl

# Vérification des fuites mémoire avec Valgrind
valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) ncurses

# Marque ces cibles comme n'étant pas des fichiers
.PHONY: all clean run-ncurses run-sdl valgrind dirs