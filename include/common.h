#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

// Dimensions logiques du jeu (indépendantes des pixels)
#define GAME_WIDTH 100
#define GAME_HEIGHT 50

// Limites pour les tableaux statiques (pour éviter malloc à chaque tir)
#define MAX_ENEMIES 50
#define MAX_BULLETS 100

// Vitesse de la boucle de jeu (60 FPS)
#define TARGET_FPS 60
#define FRAME_DELAY (1000 / TARGET_FPS)

#endif // COMMON_H