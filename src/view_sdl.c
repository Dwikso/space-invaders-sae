#include "view_sdl.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>

// --- Configuration des Ressources ---
#define WIN_WIDTH 1024
#define WIN_HEIGHT 768

// Chemins vers vos assets
#define IMG_PLAYER "assets/Sprites/Invaders/space__0000_A1.bmp"
#define IMG_ENEMY "assets/Sprites/Invaders/space__0002_B1.bmp"
#define IMG_BULLET "assets/Sprites/Projectiles/missile_1.bmp"
#define IMG_BG "assets/background.bmp"

// Facteurs de mise à l'échelle (Model -> Pixels)
static float SCALE_X = 1.0f;
static float SCALE_Y = 1.0f;

// --- Contexte Global SDL (Privé à ce fichier) ---
static struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    // Textures
    SDL_Texture *tex_player;
    SDL_Texture *tex_enemy;
    SDL_Texture *tex_bullet;
    SDL_Texture *tex_bg;
} ctx = {0};

// --- Fonctions Utilitaires ---

static SDL_Texture *load_texture(const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
    {
        fprintf(stderr, "[SDL] Attention: Impossible de charger %s : %s\n", path, SDL_GetError());
        return NULL;
    }

    // --- CORRECTION SDL3 POUR LA TRANSPARENCE ---
    // Récupération des détails du format
    const SDL_PixelFormatDetails *details = SDL_GetPixelFormatDetails(surface->format);

    // Mapping de la couleur NOIRE (0,0,0) pour la transparence
    Uint32 key = SDL_MapRGB(details, NULL, 0, 0, 0);

    // Application de la transparence (Color Key)
    SDL_SetSurfaceColorKey(surface, true, key);
    // ---------------------------------------------

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx.renderer, surface);
    SDL_DestroySurface(surface); // Libération de la surface temporaire

    if (!tex)
    {
        fprintf(stderr, "[SDL] Erreur conversion texture %s : %s\n", path, SDL_GetError());
    }
    return tex;
}

static void draw_entity(SDL_Texture *tex, const Entity *e)
{
    if (!e->active)
        return;

    // Conversion Coordonnées Modèle -> Pixels Écran
    SDL_FRect dst_rect = {
        .x = e->x * SCALE_X,
        .y = e->y * SCALE_Y,
        .w = e->width * SCALE_X,
        .h = e->height * SCALE_Y};

    if (tex)
    {
        SDL_RenderTexture(ctx.renderer, tex, NULL, &dst_rect);
    }
    else
    {
        // Fallback : Carré magenta si pas d'image
        SDL_SetRenderDrawColor(ctx.renderer, 255, 0, 255, 255);
        SDL_RenderFillRect(ctx.renderer, &dst_rect);
    }
}

// --- Implémentation de l'Interface ---

// 1. Initialisation
static bool sdl_init(void)
{
    // SDL_Init retourne true en cas de succès dans SDL3 (wrapper booléen)
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    ctx.window = SDL_CreateWindow("Space Invaders (SDL3)", WIN_WIDTH, WIN_HEIGHT, 0);
    if (!ctx.window)
        return false;

    ctx.renderer = SDL_CreateRenderer(ctx.window, NULL);
    if (!ctx.renderer)
        return false;

    SCALE_X = (float)WIN_WIDTH / GAME_WIDTH;
    SCALE_Y = (float)WIN_HEIGHT / GAME_HEIGHT;

    // Chargement des textures
    ctx.tex_player = load_texture(IMG_PLAYER);
    ctx.tex_enemy = load_texture(IMG_ENEMY);
    ctx.tex_bullet = load_texture(IMG_BULLET);
    // ctx.tex_bg = load_texture(IMG_BG); // Décommentez si vous avez un fond

    return true;
}

// 2. Fermeture
static void sdl_close(void)
{
    if (ctx.tex_player)
        SDL_DestroyTexture(ctx.tex_player);
    if (ctx.tex_enemy)
        SDL_DestroyTexture(ctx.tex_enemy);
    if (ctx.tex_bullet)
        SDL_DestroyTexture(ctx.tex_bullet);
    if (ctx.tex_bg)
        SDL_DestroyTexture(ctx.tex_bg);

    if (ctx.renderer)
        SDL_DestroyRenderer(ctx.renderer);
    if (ctx.window)
        SDL_DestroyWindow(ctx.window);
    SDL_Quit();
}

// 3. Rendu
static void sdl_render(const GameModel *model)
{
    // 1. Gestion de la couleur de fond pour indiquer l'état
    if (model->state == STATE_MENU)
    {
        // Fond Bleu Foncé pour le Menu
        SDL_SetRenderDrawColor(ctx.renderer, 20, 20, 50, 255);
    }
    else if (model->state == STATE_GAME_OVER)
    {
        // Fond Rouge Foncé pour Game Over
        SDL_SetRenderDrawColor(ctx.renderer, 50, 0, 0, 255);
    }
    else
    {
        // Fond Noir pour le Jeu
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
    }
    SDL_RenderClear(ctx.renderer);

    // ... (Code d'affichage de l'image de fond si vous en avez une) ...

    // 2. Mise à jour du Titre de la fenêtre (C'est votre interface textuelle pour l'instant)
    char title[256];
    switch (model->state)
    {
    case STATE_MENU:
        snprintf(title, sizeof(title), ">>> MENU PRINCIPAL <<< Appuyez sur [ESPACE] pour Jouer !");
        break;
    case STATE_PLAYING:
        snprintf(title, sizeof(title), "Space Invaders - Score: %d | Vies: %d | Niveau: %d",
                 model->score, model->lives, model->level);
        break;
    case STATE_PAUSED:
        snprintf(title, sizeof(title), "--- PAUSE --- [P] pour reprendre");
        break;
    case STATE_GAME_OVER:
        snprintf(title, sizeof(title), "GAME OVER ! Score: %d - Appuyez sur [Q] pour quitter", model->score);
        break;
    default:
        snprintf(title, sizeof(title), "Space Invaders");
    }
    SDL_SetWindowTitle(ctx.window, title);

    // ... (Le reste de la fonction avec draw_entity...)
    // Assurez-vous de garder tout le reste du code de dessin !

    // 3. Dessiner les entités
    draw_entity(ctx.tex_player, &model->player);
    for (int i = 0; i < MAX_ENEMIES; i++)
        draw_entity(ctx.tex_enemy, &model->enemies[i]);
    for (int i = 0; i < MAX_BULLETS; i++)
        draw_entity(ctx.tex_bullet, &model->bullets[i]);

    // ... (Affichage des vies) ...

    SDL_RenderPresent(ctx.renderer);
}

// 4. Entrées
static GameCommand sdl_get_input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            return CMD_EXIT;

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            switch (event.key.key)
            {
            case SDLK_ESCAPE:
                return CMD_EXIT;
            case SDLK_Q:
                return CMD_EXIT;
            case SDLK_SPACE:
                return CMD_SHOOT;
            case SDLK_P:
                return CMD_PAUSE;
            case SDLK_LEFT:
                return CMD_MOVE_LEFT;
            case SDLK_RIGHT:
                return CMD_MOVE_RIGHT;
            default:
                break;
            }
        }
    }

    const bool *state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_LEFT])
        return CMD_MOVE_LEFT;
    if (state[SDL_SCANCODE_RIGHT])
        return CMD_MOVE_RIGHT;

    return CMD_NONE;
}

// L'interface exposée (Définie à la fin pour voir toutes les fonctions)
const ViewInterface view_sdl = {
    .init = sdl_init, // On pointe bien vers notre wrapper, pas SDL_Init direct
    .close = sdl_close,
    .render = sdl_render,
    .get_input = sdl_get_input};