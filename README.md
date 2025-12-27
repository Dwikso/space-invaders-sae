# 👾 Space Invaders

<div align="center">

![Status](https://img.shields.io/badge/Status-Completed-success)
![Language](https://img.shields.io/badge/Language-C-blue)
![License](https://img.shields.io/badge/License-MIT-green)

*Une réimplémentation robuste et modulaire du classique arcade Space Invaders*

[Fonctionnalités](#-fonctionnalités) • [Installation](#-installation) • [Utilisation](#-utilisation) • [Architecture](#-architecture) • [Documentation](#-documentation)

</div>

---

## 📖 À propos

Ce projet est une réimplémentation complète du jeu **Space Invaders** en **C pur**, mettant l'accent sur une architecture logicielle propre et modulaire. Le jeu implémente le pattern **MVC** (Modèle-Vue-Contrôleur) et supporte **deux moteurs de rendu** interchangeables :

- 🖥️ **SDL3** : Graphismes pixel-art avec animations fluides et audio complet
- 📟 **Ncurses** : Mode terminal ASCII rétro, jouable directement dans votre console

## ✨ Fonctionnalités

### 🎮 Gameplay Authentique

- **Trois types d'aliens** avec comportements et animations distincts
- **OVNI mystère** apparaissant aléatoirement pour des bonus
- **Quatre bunkers destructibles** avec système de dégradation progressive (10 états)
- **Difficulté progressive** : les ennemis accélèrent et descendent au fur et à mesure
- **Système de vies et high-score** classique

### 💾 Système de Sauvegarde

- Sauvegarde et chargement de parties multiples
- Noms de fichiers personnalisables
- Confirmation d'écrasement avec interface intuitive
- Persistance du high-score

### 🎵 Moteur Audio (SDL_Mixer)

- Musique de fond immersive
- Effets sonores : tir, explosions, OVNI, destruction
- Contrôle du volume et fonction mute intégrés au menu

### 🏗️ Architecture Technique

- **Pattern MVC** avec séparation stricte des responsabilités
- **Polymorphisme en C** via l'interface `ViewInterface`
- **Gestion mémoire optimisée** : allocation statique, pas de `malloc` en boucle de jeu
- **Code documenté** avec Doxygen
- **Portabilité** : Linux (facilement adaptable à d'autres plateformes)

## 🚀 Installation

### Prérequis

Vous aurez besoin des outils et bibliothèques suivants :

**Outils de compilation :**
```bash
sudo apt update
sudo apt install build-essential git make
```

**Bibliothèques de développement :**
```bash
# Pour la version graphique (SDL3)
sudo apt install libsdl3-dev libsdl3-image-dev libsdl3-ttf-dev libsdl3-mixer-dev

# Pour la version terminal (Ncurses)
sudo apt install libncurses5-dev

# Pour générer la documentation (optionnel)
sudo apt install doxygen graphviz
```

> **Note :** SDL3 étant très récent, vous devrez peut-être le compiler depuis les sources officielles si les paquets ne sont pas disponibles dans votre distribution. Consultez [libsdl.org](https://libsdl.org/) pour plus d'informations.

### Compilation

```bash
# Cloner le dépôt
git clone <votre-repo-url>
cd space_invaders

# Compiler le projet
make

# Lancer le jeu
./space_invaders
```

### Commandes Make

```bash
make          # Compile le projet
make clean    # Supprime les fichiers objets
make doc      # Génère la documentation Doxygen
make fclean   # Nettoyage complet
make re       # Recompile tout
```

## 🕹️ Utilisation

### Contrôles

| Action | Touche |
|--------|--------|
| Se déplacer | `←` `→` (Flèches) |
| Tirer | `Espace` |
| Pause / Menu | `Échap` |
| Valider | `Entrée` |
| Quitter | `Q` (dans le menu) |
| Plein écran | `F11` (SDL uniquement) |
| Saisie texte | `A-Z`, `0-9`, `Backspace` |

### Menu Principal

Au lancement, vous pouvez :
- **Nouvelle partie** : Commencer une nouvelle session
- **Charger** : Reprendre une partie sauvegardée
- **Options** : Régler le volume et l'affichage
- **Quitter** : Fermer le jeu

### Sauvegarde

Appuyez sur `Échap` pendant le jeu pour accéder au menu pause, puis choisissez "Sauvegarder". Vous pourrez nommer votre sauvegarde et la reprendre plus tard.

## 📂 Structure du Projet

```
space_invaders/
├── assets/              # Ressources multimédia
│   ├── images/          # Sprites et textures (.bmp)
│   ├── fonts/           # Polices TrueType (.ttf)
│   └── sounds/          # Effets sonores et musique (.wav)
├── include/             # Fichiers d'en-tête
│   ├── common.h         # Constantes et macros globales
│   ├── model.h          # Structures de données du jeu
│   ├── view_interface.h # Interface abstraite pour les vues
│   ├── view_sdl.h       # Implémentation SDL3
│   ├── view_ncurses.h   # Implémentation Ncurses
│   └── controller.h     # Gestion des entrées utilisateur
├── src/                 # Code source
│   ├── main.c           # Point d'entrée et boucle principale
│   ├── model.c          # Logique métier du jeu
│   ├── controller.c     # Traitement des inputs
│   ├── view_sdl.c       # Rendu graphique SDL
│   └── view_ncurses.c   # Rendu terminal Ncurses
├── docs/                # Documentation générée (Doxygen)
├── Doxyfile             # Configuration Doxygen
├── Makefile             # Script de compilation
└── README.md            # Ce fichier
```

## 🏛️ Architecture

### Pattern MVC

Le projet suit rigoureusement le pattern **Modèle-Vue-Contrôleur** :

- **Modèle** (`model.c`/`model.h`) : Contient toute la logique du jeu (entités, collisions, score, état)
- **Vue** (`view_*.c`) : Responsable du rendu visuel, sans logique métier
- **Contrôleur** (`controller.c`) : Gère les entrées utilisateur et met à jour le modèle

### Pattern Strategy (Vues Polymorphes)

L'interface `ViewInterface` définit un contrat avec des pointeurs de fonctions :

```c
typedef struct {
    void (*init)(void);
    void (*close)(void);
    void (*render)(GameState* state);
    Input (*get_input)(void);
} ViewInterface;
```

Cela permet de **changer de moteur de rendu** en modifiant une seule ligne dans `main.c`, sans toucher à la logique :

```c
// Utiliser SDL
ViewInterface* view = get_sdl_view();

// Ou utiliser Ncurses
ViewInterface* view = get_ncurses_view();
```

### Gestion Mémoire

Pour garantir **stabilité** et **prévisibilité** :

- Allocation statique pour les entités (tableaux de taille fixe)
- Chargement des ressources au démarrage uniquement
- Aucun `malloc`/`free` dans la boucle de jeu
- Libération propre des ressources à la fermeture

## 📚 Documentation

La documentation technique complète est générée avec **Doxygen** :

```bash
make doc
```

Ouvrez ensuite `docs/html/index.html` dans votre navigateur pour parcourir l'API, les structures de données et les diagrammes.

## 🎯 Objectifs Pédagogiques

Ce projet démontre :

- ✅ **Architecture logicielle** : Séparation des responsabilités, interfaces abstraites
- ✅ **Programmation C avancée** : Pointeurs de fonctions, structures, modularité
- ✅ **Gestion de ressources** : Chargement/libération propre, pas de fuites mémoire
- ✅ **Portabilité** : Abstraction du système de rendu
- ✅ **Bonnes pratiques** : Documentation, nommage cohérent, code lisible

## 🤝 Contribution

Les contributions sont les bienvenues ! N'hésitez pas à :

- Signaler des bugs via les Issues
- Proposer des améliorations
- Soumettre des Pull Requests

## 📝 Licence

Ce projet est sous licence **MIT**. Voir le fichier `LICENSE` pour plus de détails.

## 👤 Auteur

Projet réalisé en C avec passion pour démontrer une architecture logicielle propre et modulaire.

---

<div align="center">

**Amusez-vous bien à sauver la galaxie !** 🚀👾

*Si vous aimez ce projet, n'oubliez pas de lui donner une ⭐ !*

</div>