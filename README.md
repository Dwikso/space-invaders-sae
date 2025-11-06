# 🚀 Space Invaders MVC

> Implémentation du classique Space Invaders en C avec architecture MVC et double interface (ncurses/SDL3)

## 🎯 Description

Jeu Space Invaders développé en C (ISO C99) avec une architecture Modèle-Vue-Contrôleur propre. Le projet supporte deux modes d'affichage interchangeables :
- **ncurses** : Interface texte ASCII dans le terminal
- **SDL3** : Interface graphique moderne

## 📦 Installation rapide (Fedora)

```bash
# Installation des dépendances
sudo dnf install gcc make ncurses-devel SDL3-devel

# Cloner et compiler
git clone https://github.com/VOTRE_USERNAME/space-invaders-mvc.git
cd space-invaders-mvc
make
```

## 🎮 Lancement

```bash
# Version terminal (ncurses)
make run-ncurses

# Version graphique (SDL3)
make run-sdl
```

## 🕹️ Commandes

| Touche | Action |
|--------|--------|
| **← →** | Déplacer |
| **ESPACE** | Tirer |
| **P** | Pause |
| **Q** / **ESC** | Quitter |

## 🏗️ Architecture

```
src/
├── model/       # Logique du jeu (état, collisions, score)
├── view/        # Rendu (ncurses + SDL3)
├── controller/  # Gestion des entrées
└── main.c       # Point d'entrée
```

**Principe MVC** : Le modèle gère la logique, les vues affichent le jeu, le contrôleur capture les inputs.

## 🔧 Commandes Make

```bash
make              # Compile tout
make ncurses      # Compile version ncurses
make sdl          # Compile version SDL3
make clean        # Nettoie les fichiers compilés
make test         # Lance les tests
make valgrind     # Vérifie les fuites mémoire
```

## 🐛 Problèmes courants

**Erreur SDL3 non trouvée ?**
```bash
# SDL3 n'est peut-être pas dans les dépôts Fedora
# Compiler depuis les sources ou utiliser uniquement ncurses
make ncurses
```

**Terminal trop petit ?**
```bash
# Minimum 80x24 caractères requis pour ncurses
resize -s 24 80
```

## 📜 Licence

MIT License - Voir [LICENSE](LICENSE)

## 👤 Auteur

**[VOTRE NOM]**
- GitHub: [@VOTRE_USERNAME](https://github.com/VOTRE_USERNAME)
- Email: votre.email@example.com

---

⭐ **Star le projet si ça te plaît !**# space-invaders
