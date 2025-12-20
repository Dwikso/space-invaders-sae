#include "view_sdl.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>


// --- Configuration des Ressources ---
#define WIN_WIDTH 1024
#define WIN_HEIGHT 768

// Chemins vers vos assets
#define IMG_PLAYER "assets/Sprites/Invaders/space__0000_A1.bmp"
#define IMG_ENEMY "assets/Sprites/Invaders/space__0002_B1.bmp"
#define IMG_BULLET "assets/Sprites/Projectiles/missile_1.bmp"
#define IMG_BG "assets/background.bmp"

#define FONT_PATH "assets/fonts/LibertinusSerifDisplay-Regular.ttf"
#define FONT_SIZE 28

// Facteurs de mise à l'échelle (Model -> Pixels)
static float SCALE_X = 1.0f;
static float SCALE_Y = 1.0f;

// --- Contexte Global SDL (Privé à ce fichier) ---
static struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;

    // Textures
    SDL_Texture *tex_player;
    SDL_Texture *tex_enemy;
    SDL_Texture *tex_bullet;
    SDL_Texture *tex_bg;
} ctx = {0};

// --- Fonctions Utilitaires ---

// Prototype de la fonction
static void draw_text(const char *text, int x, int y, SDL_Color color);

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
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) || !TTF_Init())
    {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    ctx.font = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (!ctx.font) {
        fprintf(stderr, "[TTF] Erreur chargement police: %s\n", SDL_GetError());
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
    // --- 1. NETTOYAGE DE L'ÉCRAN ---
    // On choisit la couleur de fond selon l'état
    if (model->state == STATE_MENU) {
        SDL_SetRenderDrawColor(ctx.renderer, 20, 20, 50, 255); // Bleu nuit
    } else if (model->state == STATE_GAME_OVER) {
        SDL_SetRenderDrawColor(ctx.renderer, 50, 0, 0, 255);   // Rouge sombre
    } else {
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);    // Noir
    }
    SDL_RenderClear(ctx.renderer);

    // --- 2. DESSIN DES ENTITÉS (SPRITES) ---
    // Le joueur
    draw_entity(ctx.tex_player, &model->player);

    // Les ennemis
    for (int i = 0; i < MAX_ENEMIES; i++) {
        draw_entity(ctx.tex_enemy, &model->enemies[i]);
    }

    // Les projectiles
    for (int i = 0; i < MAX_BULLETS; i++) {
        draw_entity(ctx.tex_bullet, &model->bullets[i]);
    }

    // --- 3. MISE À JOUR DU TITRE (Barre de fenêtre) ---
    char title[128];
    snprintf(title, sizeof(title), "Space Invaders - Score: %d", model->score);
    SDL_SetWindowTitle(ctx.window, title);

    // --- 4. AFFICHAGE DU TEXTE (INTERFACE / HUD) ---
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    // Score en haut à droite
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "SCORE: %d", model->score);
    draw_text(score_str, WIN_WIDTH - 200, 20, white);

    // Vies en haut à gauche
    char lives_str[32];
    snprintf(lives_str, sizeof(lives_str), "VIES: %d", model->lives);
    draw_text(lives_str, 20, 20, white);

    // Messages centraux selon l'état
    if (model->state == STATE_MENU) {
        draw_text("MENU PRINCIPAL", WIN_WIDTH / 2 - 100, WIN_HEIGHT / 2 - 40, yellow);
        draw_text("ESPACE POUR JOUER", WIN_WIDTH / 2 - 130, WIN_HEIGHT / 2 + 10, white);
    } 
    else if (model->state == STATE_PAUSED) {
        draw_text("PAUSE", WIN_WIDTH / 2 - 40, WIN_HEIGHT / 2, white);
    }
    else if (model->state == STATE_GAME_OVER) {
        draw_text("GAME OVER", WIN_WIDTH / 2 - 70, WIN_HEIGHT / 2 - 20, yellow);
        draw_text("SCORE FINAL: ", WIN_WIDTH / 2 - 80, WIN_HEIGHT / 2 + 20, white);
        draw_text(score_str, WIN_WIDTH / 2 + 60, WIN_HEIGHT / 2 + 20, white);
    }

    // --- 5. ENVOI AU GPU ---
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

static void draw_text(const char *text, int x, int y, SDL_Color color)
{
    if (!ctx.font || !text || text[0] == '\0') return;

    // Création de la surface
    SDL_Surface *surface = TTF_RenderText_Blended(ctx.font, text, 0, color);
    if (!surface) return;

    // Création de la texture
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx.renderer, surface);
    if (tex) {
        // Définir la zone de destination (on utilise les dimensions de la surface)
        SDL_FRect dst_rect = { 
            (float)x, 
            (float)y, 
            (float)surface->w, 
            (float)surface->h 
        };

        // Dessiner la texture
        SDL_RenderTexture(ctx.renderer, tex, NULL, &dst_rect);
        
        // Nettoyage immédiat pour éviter les fuites mémoire
        SDL_DestroyTexture(tex);
    }
    SDL_DestroySurface(surface);
}

// L'interface exposée (Définie à la fin pour voir toutes les fonctions)
const ViewInterface view_sdl = {
    .init = sdl_init, // On pointe bien vers notre wrapper, pas SDL_Init direct
    .close = sdl_close,
    .render = sdl_render,
    .get_input = sdl_get_input};