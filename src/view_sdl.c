#include "view_sdl.h"
#include <stdio.h>

// Implémentations vides pour l'instant
static bool sdl_init(void)
{
    printf("SDL non implémenté pour le moment !\n");
    return false; // Force l'erreur si on essaie de lancer SDL
}
static void sdl_close(void) {}
static void sdl_render(const GameModel *m) { (void)m; }
static GameCommand sdl_get_input(void) { return CMD_NONE; }

const ViewInterface view_sdl = {
    .init = sdl_init,
    .close = sdl_close,
    .render = sdl_render,
    .get_input = sdl_get_input};