/**
 * @file view_sdl.h
 * @brief Implémentation de la Vue Graphique (SDL3).
 *
 * Ce module expose l'instance de l'interface utilisant la Simple DirectMedia Layer (SDL).
 * Il contient la définition de la structure de contexte (SDLContext) qui regroupe
 * toutes les ressources graphiques et sonores.
 *
 * @date 2024
 */

#ifndef VIEW_SDL_H
#define VIEW_SDL_H

#include "view_interface.h"

// --- Inclusions nécessaires pour les types SDL ---
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_mixer/SDL_mixer.h>
// constantes
// Couleurs constantes pour éviter de les réécrire partout
static const SDL_Color COL_WHITE = {255, 255, 255, 255};
static const SDL_Color COL_RED = {255, 50, 50, 255};
static const SDL_Color COL_GREEN = {50, 255, 50, 255};
static const SDL_Color COL_YELLOW = {255, 215, 0, 255};
static const SDL_Color COL_GRAY = {150, 150, 150, 255};

// --- Configuration des Ressources ---
#define WIN_WIDTH 1280
#define WIN_HEIGHT 768
#define MAX_LIVES_DISPLAY 3 // Combien de coeurs on affiche au total
#define HEART_UI_SIZE 45    // Taille d'affichage en pixels (carré)
#define HEART_SPACING 5
// Chemins vers vos assets
#define IMG_PLAYER "assets/aliens/space_player.bmp"

// Type 1 (En bas)
#define IMG_ENEMY1_A "assets/aliens/alien_A1.bmp"
#define IMG_ENEMY1_B "assets/aliens/alien_A2.bmp"

// Type 2 (Milieu)
#define IMG_ENEMY2_A "assets/aliens/alien_B1.bmp"
#define IMG_ENEMY2_B "assets/aliens/alien_B2.bmp"

// Type 3 (Haut)
#define IMG_ENEMY3_A "assets/aliens/alien_C1.bmp"
#define IMG_ENEMY3_B "assets/aliens/alien_C2.bmp"
// Backgrounds
#define IMG_BG_MENU "assets/backgrounds/bg_menu_1.bmp"
#define IMG_BG_MENU_1 "assets/backgrounds/bg_menu.bmp"
#define IMG_BG_GAME "assets/backgrounds/background.bmp"
// NOUVEAUX CHEMINS POUR LES COEURS
#define IMG_HEART_FULL "assets/hearts/heart_full.bmp"
#define IMG_HEART_EMPTY "assets/hearts/heart_empty.bmp"

// police et taille de police
#define FONT_PATH "assets/fonts/LibertinusSerifDisplay-Regular.ttf"
#define FONT_SIZE 38
// image explosion
#define IMG_EXPLOSION_A "assets/explosions/playerExplosionA.bmp"
#define IMG_EXPLOSION_B "assets/explosions/playerExplosionB.bmp"
// UFO
#define IMG_UFO "assets/aliens/space_UFO.bmp" // Vérifie le chemin exact !

// ============================================================================
//                          STRUCTURES DE DONNÉES
// ============================================================================

/**
 * @brief Regroupe toutes les textures du jeu.
 * Permet d'organiser les assets par catégories (Ennemis, UI, FX...).
 */
typedef struct
{
    // --- Entités ---
    SDL_Texture *player; ///< Sprite du vaisseau joueur
    SDL_Texture *bullet; ///< Sprite simple de balle (fallback)
    SDL_Texture *ufo;    ///< Sprite de l'OVNI bonus

    /** * @brief Sprites des ennemis.
     * Dimensions : [3 Types] x [2 Frames d'animation]
     */
    SDL_Texture *enemies[3][2];

    // --- Décors ---
    SDL_Texture *bg_menu;   ///< Fond du menu principal
    SDL_Texture *bg_menu_1; ///< Fond des sous-menus
    SDL_Texture *bg_game;   ///< Fond du jeu (Espace)

    // --- Interface (HUD) ---
    SDL_Texture *heart_full;  ///< Icône vie pleine
    SDL_Texture *heart_empty; ///< Icône vie perdue
    SDL_Texture *blur;        ///< Effet de flou (optionnel)

    // --- Effets Visuels (FX) ---
    SDL_Texture *expl_player[2]; ///< Animation explosion joueur (Frame A et B)
    SDL_Texture *expl_enemy;     ///< Explosion unique alien
    SDL_Texture *expl_ufo;       ///< Explosion unique OVNI

    // --- Projectiles & Boucliers ---
    SDL_Texture *missiles[4];    ///< Missiles du joueur animés (4 frames)
    SDL_Texture *projectiles[4]; ///< Tirs des aliens animés (4 frames)
    SDL_Texture *shields[10];    ///< États des boucliers (10 = Full, 0 = Détruit)
} GameTextures;

/**
 * @brief Regroupe tous les sons et musiques du jeu.
 */
typedef struct
{
    // --- SFX (Bruitages) ---
    MIX_Audio *shoot;     ///< Son de tir
    MIX_Audio *killed;    ///< Son de mort alien
    MIX_Audio *explosion; ///< Son de mort joueur
    MIX_Audio *ufo;       ///< Son de l'OVNI (brut)
    MIX_Audio *beat[4];   ///< Les 4 notes du rythme de fond
    MIX_Audio *game_over; ///< Jingle de défaite
    MIX_Audio *level_up;  ///< Jingle de victoire
    MIX_Audio *select;    ///< Bip de validation menu

    // --- Musique & Pistes ---
    MIX_Audio *bg_music_data;  ///< Données audio de la musique de menu
    MIX_Track *bg_music_track; ///< Piste contrôlable pour la musique menu
    MIX_Track *ufo_track;      ///< Piste contrôlable pour la boucle OVNI
} GameAudio;

/**
 * @brief Le Contexte Global SDL.
 * "God Structure" contenant tout ce qui est nécessaire au rendu graphique et audio.
 */
typedef struct
{
    SDL_Window *window;     ///< La fenêtre du système d'exploitation
    SDL_Renderer *renderer; ///< Le moteur de rendu 2D accéléré

    TTF_Font *font;       ///< Police standard (taille moyenne)
    TTF_Font *font_title; ///< Police de titre (grande taille)

    GameTextures tex; ///< Sous-structure contenant toutes les images
    GameAudio sfx;    ///< Sous-structure contenant tous les sons

    MIX_Mixer *mixer; ///< Le moteur de mixage audio SDL3

    int ufo_channel; ///< (Obsolète si Tracks utilisés) ID du canal OVNI
} SDLContext;

/**
 * @brief Instance concrète de l'interface View pour SDL.
 * Définie dans view_sdl.c.
 */
extern const ViewInterface view_sdl;

#endif // VIEW_SDL_H