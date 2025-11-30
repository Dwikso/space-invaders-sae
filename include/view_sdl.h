#ifndef VIEW_SDL_H
#define VIEW_SDL_H

#include "view_interface.h"

/* * Expose l'instance de l'interface pour la Vue SDL.
 * L'implémentation réelle (et les includes <SDL3/SDL.h>)
 * sont cachés dans le fichier .c pour ne pas polluer le projet.
 */
extern const ViewInterface view_sdl;

#endif // VIEW_SDL_H