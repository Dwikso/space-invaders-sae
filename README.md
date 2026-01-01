# 🚀 Space Invaders MVC

> Implémentation moderne du classique Space Invaders en C avec architecture MVC et double interface (ncurses/SDL3)

![C](https://img.shields.io/badge/Language-C99-blue)
![SDL3](https://img.shields.io/badge/Graphics-SDL3-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

---

## 📋 Table des matières

- [Description](#-description)
- [Caractéristiques](#-caractéristiques)
- [Architecture MVC](#-architecture-mvc)
- [Prérequis](#-prérequis)
- [Installation](#-installation)
- [Compilation](#-compilation)
- [Lancement du jeu](#-lancement-du-jeu)
- [Commandes](#-commandes)
- [Structure du projet](#-structure-du-projet)
- [Système de sauvegarde](#-système-de-sauvegarde)
- [Dépannage](#-dépannage)
- [Contribution](#-contribution)
- [Licence](#-licence)

---

## 🎯 Description

Space Invaders MVC est une réimplémentation fidèle du jeu d'arcade classique **Space Invaders** (1978), développée en C ISO C99 avec une architecture **Modèle-Vue-Contrôleur** stricte.

Le projet propose **deux modes d'affichage interchangeables** :

- **ncurses** : Interface texte ASCII jouable directement dans le terminal
- **SDL3** : Interface graphique moderne avec sprites, animations et effets sonores

### Objectifs pédagogiques

- Démonstration d'une architecture MVC propre en C
- Séparation stricte entre logique métier et rendu graphique
- Gestion mémoire sans malloc pendant la boucle de jeu (allocation statique)
- Double implémentation d'interface via polymorphisme en C

---

## ✨ Caractéristiques

### Gameplay

- ✅ **3 types d'ennemis** avec scores différents (10/20/40 points)
- ✅ **OVNI mystère** apparaissant aléatoirement (60 points + vie bonus)
- ✅ **4 boucliers destructibles** avec 10 états de dégradation
- ✅ **Système de vies** avec indicateur visuel (cœurs)
- ✅ **Progression par niveaux** avec augmentation de difficulté
- ✅ **Animations** : sprites alternés, explosions, tremblements d'écran
- ✅ **Rythme alien** : son accélérant avec la tension

### Technique

- 🎮 **Double interface** : ncurses (terminal) et SDL3 (graphique)
- 💾 **Système de sauvegarde** complet avec gestion de fichiers
- 🎵 **Audio SDL_mixer** : bruitages, musiques, boucles
- 🎨 **Sprites animés** : 4 frames pour projectiles, 2 pour ennemis
- ⚡ **Boucle de jeu fixe** à 60 FPS avec delta time
- 🔧 **Responsive** : Adaptation automatique à la taille de la fenêtre/terminal

---

## 🏗️ Architecture MVC

Le projet suit strictement le pattern **Modèle-Vue-Contrôleur** :

```
┌─────────────────────────────────────────────┐
│              MAIN (main.c)                  │
│         Boucle de jeu principale            │
└────────┬─────────────┬──────────────────────┘
         │             │
    ┌────▼─────┐  ┌───▼──────┐
    │ MODÈLE   │  │   VUE    │
    │(model.c) │  │(view_*.c)│
    │          │  │          │
    │ Logique  │  │  Rendu   │
    │ Métier   │  │ Graphique│
    └────▲─────┘  └───▲──────┘
         │            │
    ┌────┴────────────┴─────┐
    │    CONTRÔLEUR         │
    │   (controller.h)      │
    │  Commandes abstraites │
    └───────────────────────┘
```

### 📂 Répartition des responsabilités

#### **Modèle** (`src/model.c`)

- État du jeu (score, vies, niveau)
- Physique des entités (positions, vitesses, collisions)
- Règles métier (game over, level up, spawning)
- Système de sauvegarde/chargement
- **⚠️ Ne connaît PAS la vue** : aucune dépendance graphique

#### **Vue** (`src/view_ncurses.c` / `src/view_sdl.c`)

- Rendu à l'écran (sprites, textes, HUD)
- Gestion audio (bruitages, musiques)
- Capture des entrées clavier/souris
- Conversion coordonnées logiques → pixels/caractères
- **⚠️ Ne modifie PAS le modèle** : lecture seule

#### **Contrôleur** (`include/controller.h`)

- Définition des **commandes abstraites** (`CMD_MOVE_LEFT`, `CMD_SHOOT`, etc.)
- Interface entre périphériques bruts et logique métier
- Permet de changer les touches sans modifier le modèle

---

## 📦 Prérequis

### Versions recommandées

| Dépendance     | Version minimale | Version testée | Obligatoire    |
| -------------- | ---------------- | -------------- | -------------- |
| **gcc**        | 9.0+             | 13.2.0         | ✅             |
| **make**       | 4.0+             | 4.4.1          | ✅             |
| **cmake**      | 3.20+            | 3.28.0         | ✅ (SDL)       |
| **ncurses**    | 6.0+             | 6.4            | ✅             |
| **SDL3**       | 3.2.0+           | 3.2.0          | ❌ (optionnel) |
| **SDL3_image** | 3.0+             | 3.0.0          | ❌ (SDL only)  |
| **SDL3_ttf**   | 3.0+             | 3.0.0          | ❌ (SDL only)  |
| **SDL3_mixer** | 3.0+             | 3.0.0          | ❌ (SDL only)  |

### Systèmes supportés

- ✅ **Linux** (Ubuntu, Fedora, Debian, Arch)
- ✅ **macOS** (via Homebrew)
- ⚠️ **Windows** Il faut être fou pour utiliser fou pour utiliser Windows

---

## 🔧 Installation

### Ubuntu/Debian

```bash
# Dépendances de base + ncurses
sudo apt update
sudo apt install -y build-essential make cmake git \
    libncurses-dev pkg-config

# Dépendances SDL3 (pour mode graphique)
sudo apt install -y \
    libx11-dev libxext-dev libwayland-dev libxkbcommon-dev \
    libasound2-dev libpulse-dev libudev-dev libdbus-1-dev \
    libfreetype6-dev libharfbuzz-dev \
    libpng-dev libjpeg-dev libwebp-dev libtiff-dev
```

### Fedora/RHEL/Rocky

```bash
# Dépendances de base + ncurses
sudo dnf groupinstall -y "Development Tools"
sudo dnf install -y cmake git ncurses-devel pkg-config

# Dépendances SDL3 (pour mode graphique)
sudo dnf install -y \
    libX11-devel libXext-devel wayland-devel libxkbcommon-devel \
    alsa-lib-devel pulseaudio-libs-devel systemd-devel dbus-devel \
    freetype-devel harfbuzz-devel \
    libpng-devel libjpeg-turbo-devel libwebp-devel libtiff-devel
```

### Arch Linux

```bash
sudo pacman -Sy --needed base-devel cmake git ncurses \
    libx11 libxext wayland libxkbcommon \
    alsa-lib libpulse systemd dbus \
    freetype2 harfbuzz libpng libjpeg-turbo libwebp libtiff
```

### macOS

```bash
# Installation via Homebrew
brew install gcc make cmake ncurses pkg-config \
    sdl3 sdl3_image sdl3_ttf sdl3_mixer
```

---

## 🔨 Compilation

### Clonage du dépôt

```bash
git clone https://github.com/ch/space-invaders-mvc.git
cd space-invaders-mvc
```

### Option 1 : Makefile (recommandé)

```bash
# Compilation automatique (détecte et compile SDL3_ttf/mixer si nécessaire)
make

# Nettoyage
make clean

# Nettoyage complet (inclut les bibliothèques externes)
make clean-all
```

### Option 2 : Script build.sh (multi-distro)

```bash
# Rendre le script exécutable
chmod +x build.sh

# Installation des dépendances système (1ère fois uniquement)
./build.sh --install-deps

# Compilation
./build.sh --build

# Compilation + Exécution
./build.sh --run
```

### Compilation manuelle (debug)

```bash
# Créer les dossiers
mkdir -p build

# Compiler les sources
gcc -std=c99 -Wall -Wextra -g -Iinclude \
    src/*.c \
    -o build/space_invaders \
    -lm -lncurses \
    $(pkg-config --cflags --libs sdl3) \
    -lSDL3_image -lSDL3_ttf -lSDL3_mixer
```

---

## 🎮 Lancement du jeu

### Mode Terminal (ncurses)

```bash
# Via Makefile
make run-ncurses

# Ou directement
./space_invaders ncurses
```

**⚠️ Taille minimale requise :** 80 colonnes × 24 lignes

### Mode Graphique (SDL3)

```bash
# Via Makefile
make run-sdl

# Ou directement
./space_invaders sdl
```

**💡 Astuce :** Appuyez sur **F11** pour basculer en plein écran.

---

## 🕹️ Commandes

### Menu & Navigation

| Touche                  | Action                               |
| ----------------------- | ------------------------------------ |
| **↑ / ↓**               | Naviguer dans les menus              |
| **← / →**               | Ajuster le volume / Changer d'option |
| **ESPACE** / **ENTRÉE** | Valider la sélection                 |
| **P** / **ÉCHAP**       | Pause / Retour                       |
| **Q**                   | Quitter (avec confirmation)          |
| **F11**                 | Plein écran (SDL uniquement)         |

### En jeu

| Touche     | Action               |
| ---------- | -------------------- |
| **← →**    | Déplacer le vaisseau |
| **ESPACE** | Tirer                |
| **P**      | Pause                |
| **ÉCHAP**  | Menu pause           |

### Saisie de texte (sauvegarde)

| Touche        | Action                     |
| ------------- | -------------------------- |
| **A-Z / 0-9** | Caractères alphanumériques |
| **ESPACE**    | Remplacé par `_`           |
| **-**         | Tiret (autorisé)           |
| **BACKSPACE** | Effacer                    |
| **ENTRÉE**    | Valider                    |

---

## 📁 Structure du projet

```
space-invaders-mvc/
│
├── assets/                    # Ressources graphiques/audio
│   ├── aliens/               # Sprites ennemis (A1/A2/A3, UFO)
│   ├── backgrounds/          # Fonds d'écran (menu, jeu)
│   ├── explosions/           # Animations d'explosion
│   ├── fonts/                # Police TTF (LibertinusSerif)
│   ├── hearts/               # Icônes de vie (full/empty)
│   ├── missiles/             # Projectiles joueur (4 frames)
│   ├── projectiles/          # Projectiles ennemis (4 frames)
│   ├── shelter/              # Boucliers (10 états de dégâts)
│   └── audio/                # Sons (shoot, explosion, UFO, beats)
│
├── external/                 # Bibliothèques externes
│   ├── SDL3/                # SDL3 principal (via pkg-config)
│   ├── SDL3_image/          # Chargement BMP/PNG
│   ├── SDL3_ttf/            # Rendu de texte
│   └── SDL3_mixer/          # Gestion audio
│
├── include/                  # Headers (.h)
│   ├── common.h             # Constantes globales (GAME_WIDTH, FPS...)
│   ├── controller.h         # Commandes abstraites (GameCommand)
│   ├── model.h              # Structures de données (GameModel, Entity...)
│   ├── utils.h              # Utilitaires (temps, aléatoire)
│   ├── view_interface.h     # Interface abstraite (ViewInterface)
│   ├── view_ncurses.h       # Implémentation ncurses
│   └── view_sdl.h           # Implémentation SDL3
│
├── src/                      # Sources (.c)
│   ├── main.c               # Point d'entrée + boucle de jeu
│   ├── model.c              # Logique métier (update, collisions, IA)
│   ├── utils.c              # Implémentation utilitaires
│   ├── view_ncurses.c       # Rendu terminal
│   └── view_sdl.c           # Rendu graphique SDL3
│
├── sauvegardes/             # Dossier des sauvegardes (.dat)
├── build/                   # Fichiers compilés (.o, exécutable)
│
├── Makefile                 # Script de compilation
├── build.sh                 # Script multi-distributions
├── Doxyfile                 # Configuration Doxygen (doc)
└── README.md                # Ce fichier
```

---

## 💾 Système de sauvegarde

### Fonctionnement

Les sauvegardes sont stockées dans le dossier `sauvegardes/` au format binaire (`.dat`).

**Contenu sauvegardé :**

- Score actuel
- Nombre de vies
- Niveau en cours
- Position des entités (joueur, ennemis, projectiles)
- État des boucliers
- Timers (cooldowns, invulnérabilité)

### Utilisation

#### Créer une sauvegarde

1. Appuyez sur **P** en jeu → Menu Pause
2. Sélectionnez **"Sauvegarder et Quitter"**
3. Choisissez **"Créer nouvelle sauvegarde"**
4. Tapez un nom (ex: `partie1`)
5. Validation → Fichier `sauvegardes/partie1.dat` créé

#### Charger une sauvegarde

1. Menu principal → **"Charger"**
2. Sélectionnez le fichier avec **↑ / ↓**
3. Validation → Le jeu reprend exactement où vous l'aviez quitté

#### Écraser une sauvegarde

- Le système détecte les noms existants et demande confirmation
- Option alternative : **"NON (dupliquer et renommer)"**

---

## 🐛 Dépannage

### Problème : `SDL3 non trouvé`

**Cause :** SDL3 n'est pas installé ou pkg-config ne le trouve pas.

**Solution :**

```bash
# Vérifier la présence de SDL3
pkg-config --modversion sdl3

# Si absent, compiler depuis les sources (external/SDL3/)
# Ou utiliser uniquement ncurses :
make clean
make run-ncurses
```

### Problème : `Terminal trop petit` (ncurses)

**Cause :** La fenêtre terminal fait moins de 80×24 caractères.

**Solution :**

```bash
# Redimensionner manuellement ou forcer :
resize -s 24 80

# Ou agrandir la fenêtre du terminal
```

### Problème : `Son coupé / pas de musique` (SDL)

**Cause :** SDL_mixer non initialisé ou fichiers audio absents.

**Solution :**

```bash
# Vérifier la présence des sons
ls -l assets/audio/

# Recompiler avec logs :
make clean
make 2>&1 | grep -i "audio\|mixer"
```

### Problème : `Sprites noirs / transparence cassée`

**Cause :** Le Color Key n'est pas appliqué (SDL3).

**Solution :** Vérifiez que vos BMP ont un fond **noir pur** (RGB 0,0,0). Le code applique automatiquement la transparence sur cette couleur.

### Problème : `Erreur de segmentation`

**Cause :** Accès mémoire invalide (dépassement de tableau, pointeur NULL).

**Solution :**

```bash
# Compiler en mode debug
make clean
gcc -g -O0 src/*.c -o space_invaders -lncurses -lm $(pkg-config --libs sdl3) -lSDL3_ttf -lSDL3_mixer

# Exécuter avec Valgrind
valgrind --leak-check=full ./space_invaders sdl
```

### Problème : `Makefile: command not found`

**Cause :** Make n'est pas installé.

**Solution :**

```bash
# Ubuntu/Debian
sudo apt install make

# Fedora
sudo dnf install make
```

---

## 🤝 Contribution

Les contributions sont les bienvenues ! Voici comment participer :

### 1. Fork le projet

```bash
git clone https://github.com/VOTRE_USERNAME/space-invaders-mvc.git
cd space-invaders-mvc
git checkout -b feature/ma-nouvelle-fonctionnalite
```

### 2. Respecter la structure MVC

- ❌ **Ne PAS** mélanger logique métier et rendu
- ✅ **OUI** : Créer des fonctions dédiées dans le bon module
- ✅ **OUI** : Documenter avec Doxygen (`/** @brief ... */`)

### 3. Tester les deux interfaces

```bash
# Tester ncurses
make run-ncurses

# Tester SDL
make run-sdl
```

### 4. Soumettre une Pull Request

- Titre clair (ex: `[FIX] Correction collision boucliers`)
- Description détaillée des changements
- Captures d'écran si modifications visuelles

---

## 📜 Licence

Ce projet est sous licence **MIT**. Voir le fichier [LICENSE](LICENSE) pour plus de détails.

```
MIT License

Copyright (c) 2024 [Chamseddine Adaadour & Quentin Pellosse]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 👤 Auteur

**[Chamseddine Adaadour & Quentin Pellosse]**

- 🌐 GitHub: [@Dwikso](https://github.com/Dwikso) & [@chamse22ine](https://github.com/chamse22ine)

---

## 🎓 Remerciements

- **Tomohiro Nishikado** - Créateur original de Space Invaders (1978)
- **SDL Team** - Pour la bibliothèque SDL3
- **Communauté ncurses** - Pour la gestion terminal

---

## 📚 Ressources

### Documentation technique

- [SDL3 Wiki](https://wiki.libsdl.org/SDL3/FrontPage)
- [Ncurses HOWTO](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/)
- [Architecture MVC en C](https://en.wikipedia.org/wiki/Model%E2%80%93view%E2%80%93controller)

### Tutoriels Space Invaders

- [Guide Gameplay Original](https://en.wikipedia.org/wiki/Space_Invaders)
- [Analyse Technique (Patterns)](https://www.gamedeveloper.com/design/space-invaders-design-patterns)
