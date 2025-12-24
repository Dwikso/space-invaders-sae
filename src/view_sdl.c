#include "view_sdl.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
// --- Configuration des Ressources ---
#define WIN_WIDTH 1024
#define WIN_HEIGHT 768

// Chemins vers vos assets
#define IMG_PLAYER "assets/Sprites/Invaders/space_player.bmp"
// Type 1 (En bas)
#define IMG_ENEMY1_A "assets/Sprites/Invaders/space__0000_A1.bmp"
#define IMG_ENEMY1_B "assets/Sprites/Invaders/space__0001_A2.bmp"

// Type 2 (Milieu)
#define IMG_ENEMY2_A "assets/Sprites/Invaders/space__0002_B1.bmp"
#define IMG_ENEMY2_B "assets/Sprites/Invaders/space__0003_B2.bmp"

// Type 3 (Haut)
#define IMG_ENEMY3_A "assets/Sprites/Invaders/space__0004_C1.bmp"
#define IMG_ENEMY3_B "assets/Sprites/Invaders/space__0005_C2.bmp"
#define IMG_BULLET "assets/Sprites/Projectiles/missile_1.bmp"
#define IMG_BG_MENU "assets/bg_menu_1.bmp"
#define IMG_BG_MENU_1 "assets/bg_menu.bmp"

#define IMG_BG_GAME "assets/background.bmp"
// NOUVEAUX CHEMINS POUR LES COEURS
#define IMG_HEART_FULL "assets/Sprites/heart_full.bmp"
#define IMG_HEART_EMPTY "assets/Sprites/heart_empty.bmp"
#define MAX_LIVES_DISPLAY 3 // Combien de coeurs on affiche au total
#define HEART_UI_SIZE 32    // Taille d'affichage en pixels (carré)
#define HEART_SPACING 5

#define FONT_PATH "assets/fonts/LibertinusSerifDisplay-Regular.ttf"
#define FONT_SIZE 28

#define IMG_EXPLOSION_A "assets/Sprites/explosion/playerExplosionA.bmp"
#define IMG_EXPLOSION_B "assets/Sprites/explosion/playerExplosionB.bmp"

// Facteurs de mise à l'échelle (Model -> Pixels)
static float SCALE_X = 1.0f;
static float SCALE_Y = 1.0f;

// --- Contexte Global SDL ---
static struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    TTF_Font *font;       // Police normale (Score, Options)
    TTF_Font *font_title; // <--- NOUVEAU : Grosse police pour le titre

    // Textures
    SDL_Texture *tex_player;
    SDL_Texture *tex_enemy1[2];
    SDL_Texture *tex_enemy2[2];
    SDL_Texture *tex_enemy3[2];
    SDL_Texture *tex_bullet;
    SDL_Texture *tex_bg_menu;   // Fond du menu
    SDL_Texture *tex_bg_menu_1; // Fond du menu
    SDL_Texture *tex_bg_game;   // Fond du jeu
    SDL_Texture *tex_heart_full;
    SDL_Texture *tex_heart_empty;
    SDL_Texture *blur_texture;
    SDL_Texture *tex_blur; // <--- AJOUTE ÇA
    SDL_Texture *tex_expl_a;
    SDL_Texture *tex_expl_b;
} ctx = {0};
// --- Fonctions Utilitaires ---

// Prototype de la fonction
static void draw_text(const char *text, int x, int y, SDL_Color color);
// Fonction pour centrer le texte horizontalement
static void draw_text_centered(const char *text, int y, SDL_Color color, TTF_Font *font_to_use)
{
    if (!font_to_use || !text || text[0] == '\0')
        return;

    SDL_Surface *surface = TTF_RenderText_Blended(font_to_use, text, 0, color);
    if (!surface)
        return;

    SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx.renderer, surface);
    if (tex)
    {
        // Calcul du X pour centrer : (Largeur_Fenetre - Largeur_Texte) / 2
        float x = (WIN_WIDTH - surface->w) / 2.0f;

        SDL_FRect dst_rect = {
            x,
            (float)y,
            (float)surface->w,
            (float)surface->h};

        SDL_RenderTexture(ctx.renderer, tex, NULL, &dst_rect);
        SDL_DestroyTexture(tex);
    }
    SDL_DestroySurface(surface);
}

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
    if (tex)
    {
        SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    }
    else
    {
        fprintf(stderr, "[SDL] Erreur conversion texture %s : %s\n", path, SDL_GetError());
    }
    return tex;
}

static void draw_entity(SDL_Texture *tex, const Entity *e, int offset_x, int offset_y)
{
    if (!e->active)
        return;

    // Conversion Coordonnées Modèle -> Pixels Écran
    SDL_FRect dst_rect = {
        .x = (e->x * SCALE_X) + offset_x, // <--- On ajoute le shake ici
        .y = (e->y * SCALE_Y) + offset_y, // <--- Et ici
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
    ctx.font_title = TTF_OpenFont(FONT_PATH, 64);
    if (!ctx.font || !ctx.font_title)
    {
        fprintf(stderr, "[TTF] Erreur chargement police: %s\n", SDL_GetError());
    }

    ctx.window = SDL_CreateWindow("Space Invaders", WIN_WIDTH, WIN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!ctx.window)
        return false;
    SDL_SetWindowMinimumSize(ctx.window, WIN_WIDTH, WIN_HEIGHT);

    ctx.renderer = SDL_CreateRenderer(ctx.window, NULL);
    if (!ctx.renderer)
        return false;
    // On garde seulement 4 arguments
    SDL_SetRenderLogicalPresentation(ctx.renderer, WIN_WIDTH, WIN_HEIGHT,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SCALE_X = (float)WIN_WIDTH / GAME_WIDTH;
    SCALE_Y = (float)WIN_HEIGHT / GAME_HEIGHT;

    // Chargement des textures
    ctx.tex_enemy1[0] = load_texture(IMG_ENEMY1_A);
    ctx.tex_enemy1[1] = load_texture(IMG_ENEMY1_B);

    ctx.tex_enemy2[0] = load_texture(IMG_ENEMY2_A);
    ctx.tex_enemy2[1] = load_texture(IMG_ENEMY2_B);

    ctx.tex_enemy3[0] = load_texture(IMG_ENEMY3_A);
    ctx.tex_enemy3[1] = load_texture(IMG_ENEMY3_B);

    ctx.tex_player = load_texture(IMG_PLAYER);
    ctx.tex_bullet = load_texture(IMG_BULLET);
    ctx.tex_bg_menu = load_texture(IMG_BG_MENU);
    ctx.tex_bg_menu_1 = load_texture(IMG_BG_MENU_1);

    ctx.tex_bg_game = load_texture(IMG_BG_GAME);

    ctx.tex_expl_a = load_texture(IMG_EXPLOSION_A);
    ctx.tex_expl_b = load_texture(IMG_EXPLOSION_B);
    // Petit warning si ça échoue (optionnel)
    if (!ctx.tex_bg_menu || !ctx.tex_bg_game)
    {
        SDL_Log("Attention : Fonds d'écran non trouvés !");
    }
    ctx.tex_heart_full = load_texture(IMG_HEART_FULL);
    ctx.tex_heart_empty = load_texture(IMG_HEART_EMPTY);

    if (!ctx.tex_heart_full || !ctx.tex_heart_empty)
    {
        SDL_Log("Attention : Images des coeurs manquantes !");
    }

    return true;
}

// 2. Fermeture
static void sdl_close(void)
{
    if (ctx.tex_player)
        SDL_DestroyTexture(ctx.tex_player);

    // Correction ici : On détruit les 3 textures d'ennemis
    if (ctx.tex_enemy1[0])
        SDL_DestroyTexture(ctx.tex_enemy1[0]);
    if (ctx.tex_enemy1[1])
        SDL_DestroyTexture(ctx.tex_enemy1[1]);

    if (ctx.tex_enemy2[0])
        SDL_DestroyTexture(ctx.tex_enemy2[0]);
    if (ctx.tex_enemy2[1])
        SDL_DestroyTexture(ctx.tex_enemy2[1]);

    if (ctx.tex_enemy3[0])
        SDL_DestroyTexture(ctx.tex_enemy3[0]);
    if (ctx.tex_enemy3[1])
        SDL_DestroyTexture(ctx.tex_enemy3[1]);

    if (ctx.tex_bullet)
        SDL_DestroyTexture(ctx.tex_bullet);
    if (ctx.tex_bg_menu)
        SDL_DestroyTexture(ctx.tex_bg_menu);
    if (ctx.tex_bg_game)
        SDL_DestroyTexture(ctx.tex_bg_game); // et non plus tex_bg

    if (ctx.font)
        TTF_CloseFont(ctx.font);
    if (ctx.font_title)
        TTF_CloseFont(ctx.font_title);

    if (ctx.renderer)
        SDL_DestroyRenderer(ctx.renderer);
    if (ctx.window)
        SDL_DestroyWindow(ctx.window);
    if (ctx.tex_heart_full)
        SDL_DestroyTexture(ctx.tex_heart_full);
    if (ctx.tex_heart_empty)
        SDL_DestroyTexture(ctx.tex_heart_empty);

    if (ctx.tex_expl_a)
        SDL_DestroyTexture(ctx.tex_expl_a);
    if (ctx.tex_expl_b)
        SDL_DestroyTexture(ctx.tex_expl_b);

    SDL_Quit();
}

// src/view_sdl.c

static void draw_game_world(const GameModel *model)
{
    // --- 1. CALCUL DU TREMBLEMENT (SHAKE) ---
    int shake_x = 0;
    int shake_y = 0;

    // Si le joueur est en train d'exploser (hit_timer > 0)
    if (model->hit_timer > 0)
    {
        // Génère un nombre aléatoire entre -5 et +5 pixels
        shake_x = (rand() % 10) - 5;
        shake_y = (rand() % 10) - 5;
    }

    // --- A. FOND (Avec Shake) ---
    if (ctx.tex_bg_game)
    {
        int w, h;
        SDL_GetRenderOutputSize(ctx.renderer, &w, &h);
        // On applique le shake au fond aussi pour un effet plus violent
        SDL_FRect bg_rect = {(float)shake_x, (float)shake_y, (float)w, (float)h};
        SDL_RenderTexture(ctx.renderer, ctx.tex_bg_game, NULL, &bg_rect);
    }

    // --- B. JOUEUR (Logique d'Explosion) ---
    SDL_Texture *player_tex = ctx.tex_player;

    if (model->hit_timer > 0)
    {
        // ANIMATION EXPLOSION : On alterne A et B toutes les 0.1 secondes
        // (model->hit_timer * 10) convertit les secondes en dixièmes
        int anim_frame = (int)(model->hit_timer * 10) % 2;

        if (anim_frame == 0)
            player_tex = ctx.tex_expl_a; // Image Explosion A
        else
            player_tex = ctx.tex_expl_b; // Image Explosion B

        // On teinte en rouge/orange pour l'effet dégât
        SDL_SetTextureColorMod(player_tex, 255, 100, 100);
    }
    else
    {
        // Joueur Normal (Vert)
        SDL_SetTextureColorMod(player_tex, 0, 255, 0);
    }

    // Dessin du joueur avec le SHAKE
    draw_entity(player_tex, &model->player, shake_x, shake_y);

    // --- C. ENNEMIS (Avec Shake) ---
    int frame = model->animation_frame;
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (model->enemies[i].active)
        {
            SDL_Texture *t = ctx.tex_enemy1[frame];

            if (model->enemies[i].type == ENTITY_ENEMY_TYPE_2)
            {
                t = ctx.tex_enemy2[frame];
                SDL_SetTextureColorMod(t, 255, 165, 0); // Orange
            }
            else if (model->enemies[i].type == ENTITY_ENEMY_TYPE_3)
            {
                t = ctx.tex_enemy3[frame];
                SDL_SetTextureColorMod(t, 255, 50, 50); // Rouge
            }
            else
            {
                SDL_SetTextureColorMod(t, 0, 255, 255); // Cyan
            }

            // On applique shake_x, shake_y aux ennemis aussi
            draw_entity(t, &model->enemies[i], shake_x, shake_y);

            SDL_SetTextureColorMod(t, 255, 255, 255); // Reset couleur
        }
    }

    // --- D. BALLES (Avec Shake) ---
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (model->bullets[i].active)
        {
            if (model->bullets[i].type == ENTITY_BULLET_ENEMY)
                SDL_SetTextureColorMod(ctx.tex_bullet, 255, 0, 0);
            else
                SDL_SetTextureColorMod(ctx.tex_bullet, 255, 255, 0);

            // On applique shake_x, shake_y aux balles
            draw_entity(ctx.tex_bullet, &model->bullets[i], shake_x, shake_y);
        }
    }
}

// 3. Rendu
static void sdl_render(const GameModel *model)
{
    // --- JEU & PAUSE ---
    if (model->state == STATE_PLAYING || model->state == STATE_PAUSED)
    {
        if (ctx.tex_blur)
        {
            SDL_DestroyTexture(ctx.tex_blur);
            ctx.tex_blur = NULL;
        }

        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);

        // 1. DESSIN DU MONDE (Inclus Explosion & Shake)
        draw_game_world(model);

        // 2. HUD
        SDL_Color white = {255, 255, 255, 255};
        char hud[64];
        snprintf(hud, sizeof(hud), "SCORE: %d", model->score);
        draw_text(hud, 20, 20, white);

        // Coeurs
        int start_x = WIN_WIDTH - (3 * 37) - 20;
        for (int i = 0; i < 3; i++)
        {
            SDL_Texture *hrt = (i < model->lives) ? ctx.tex_heart_full : ctx.tex_heart_empty;
            SDL_FRect r = {(float)(start_x + i * 37), 20, 32, 32};
            if (hrt)
            {
                SDL_SetTextureColorMod(hrt, 255, 255, 255); // Reset couleur
                SDL_RenderTexture(ctx.renderer, hrt, NULL, &r);
            }
        }

        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- MENU PRINCIPAL ---
    if (model->state == STATE_MENU)
    {
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);
        if (ctx.tex_bg_menu)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu, NULL, NULL);

        SDL_Color green = {50, 255, 50, 255};
        draw_text_centered("SPACE INVADERS", WIN_HEIGHT / 4, green, ctx.font_title);

        const char *opts[] = {"JOUER", "CHARGER", "QUITTER"};
        for (int i = 0; i < 3; i++)
        {
            char buf[64];
            SDL_Color c = (i == model->menu_selection) ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){100, 100, 100, 255};
            snprintf(buf, sizeof(buf), (i == model->menu_selection) ? "> %s <" : "%s", opts[i]);
            draw_text_centered(buf, WIN_HEIGHT / 2 + i * 60, c, ctx.font);
        }
        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- GAME OVER ---
    // --- GAME OVER (FOND MENU FLOU + 3 OPTIONS) ---
    if (model->state == STATE_GAME_OVER)
    {
        // A. Génération du flou (une seule fois au début du Game Over)
        if (!ctx.tex_blur)
        {
            // On crée une texture toute petite (1/8ème de l'écran)
            ctx.tex_blur = SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_TARGET, WIN_WIDTH / 8, WIN_HEIGHT / 8);
            if (ctx.tex_blur)
            {
                SDL_SetTextureScaleMode(ctx.tex_blur, SDL_SCALEMODE_LINEAR); // Pour l'effet flou

                // On redirige le dessin vers la texture
                SDL_SetRenderTarget(ctx.renderer, ctx.tex_blur);

                // --- MODIFICATION ICI ---
                // Au lieu de dessiner le jeu (draw_game_world), on dessine bg_menu
                if (ctx.tex_bg_menu_1)
                {
                    // NULL, NULL = Remplir toute la texture avec l'image
                    SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);
                }
                else
                {
                    // Sécurité : Fond noir si l'image manque
                    SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
                    SDL_RenderClear(ctx.renderer);
                }
                // ------------------------

                // On remet le dessin vers l'écran
                SDL_SetRenderTarget(ctx.renderer, NULL);
            }
        }

        // B. Rendu à l'écran
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);

        // 1. On affiche la texture (qui contient bg_menu en petit) étirée -> FLOU
        if (ctx.tex_blur)
            SDL_RenderTexture(ctx.renderer, ctx.tex_blur, NULL, NULL);

        // 2. Voile Noir (Optionnel, pour lisibilité)
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 100); // Un peu moins sombre qu'avant
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        // 3. Titre
        SDL_Color red = {255, 0, 0, 255};
        draw_text_centered("GAME OVER", WIN_HEIGHT / 4, red, ctx.font_title);

        // 4. Score
        char s_txt[32];
        SDL_Color white = {255, 255, 255, 255};
        snprintf(s_txt, sizeof(s_txt), "Score Final: %d", model->score);
        draw_text_centered(s_txt, WIN_HEIGHT / 2 - 50, white, ctx.font);

        // 5. Menu 3 Options
        const char *go_opts[] = {"SAUVEGARDER", "REJOUER", "QUITTER"};
        for (int i = 0; i < 3; i++)
        {
            char buf[64];
            // Couleur sélectionnée (Blanc) vs Normale (Gris)
            SDL_Color c = (i == model->menu_selection) ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){150, 150, 150, 255};
            snprintf(buf, sizeof(buf), (i == model->menu_selection) ? "> %s <" : "%s", go_opts[i]);
            draw_text_centered(buf, WIN_HEIGHT / 2 + 30 + i * 60, c, ctx.font);
        }

        SDL_RenderPresent(ctx.renderer);
    }
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

                // --- PLEIN ÉCRAN (F11) ---
            case SDLK_F11:
            {
                // On récupère les flags actuels de la fenêtre
                SDL_WindowFlags flags = SDL_GetWindowFlags(ctx.window);

                // Si on est déjà en Fullscreen, on passe en fenêtré (false)
                // Sinon on passe en Fullscreen (true)
                bool is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN);
                SDL_SetWindowFullscreen(ctx.window, !is_fullscreen);
                break;
            }
            // --- Navigation Menu ---
            case SDLK_UP:
                return CMD_UP;
            case SDLK_DOWN:
                return CMD_DOWN;
            case SDLK_RETURN:
                return CMD_SHOOT; // Entrée valide aussi
            case SDLK_SPACE:
                return CMD_SHOOT;
                // -----------------------

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
    if (!ctx.font || !text || text[0] == '\0')
        return;

    // Création de la surface
    SDL_Surface *surface = TTF_RenderText_Blended(ctx.font, text, 0, color);
    if (!surface)
        return;

    // Création de la texture
    SDL_Texture *tex = SDL_CreateTextureFromSurface(ctx.renderer, surface);
    if (tex)
    {
        // Définir la zone de destination (on utilise les dimensions de la surface)
        SDL_FRect dst_rect = {
            (float)x,
            (float)y,
            (float)surface->w,
            (float)surface->h};

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