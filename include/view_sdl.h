/**
 * @file view_sdl.h
 * @brief Implémentation de la Vue Graphique (SDL3).
 *
 * Ce module expose l'instance de l'interface utilisant la Simple DirectMedia Layer (SDL).
 * Il gère l'affichage des sprites, le son, et les fenêtres graphiques.
 * Note : Les headers SDL ne sont pas inclus ici pour éviter de polluer le reste du projet.
 *
 * @date 2024
 */

#ifndef VIEW_SDL_H
#define VIEW_SDL_H

#include "view_interface.h"

/**
 * @brief Instance concrète de l'interface View pour SDL.
 *
 * Cette variable globale est définie dans view_sdl.c.
 * Elle contient les pointeurs vers les fonctions spécifiques à SDL
 * (sdl_init, sdl_render, etc.).
 */
extern const ViewInterface view_sdl;

#endif // VIEW_SDL_H