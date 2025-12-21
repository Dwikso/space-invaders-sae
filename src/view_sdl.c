#include "view_sdl.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

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
#define IMG_BG_GAME "assets/background.bmp"
// NOUVEAUX CHEMINS POUR LES COEURS
#define IMG_HEART_FULL "assets/Sprites/heart_full.bmp"
#define IMG_HEART_EMPTY "assets/Sprites/heart_empty.bmp"
#define MAX_LIVES_DISPLAY 3 // Combien de coeurs on affiche au total
#define HEART_UI_SIZE 32    // Taille d'affichage en pixels (carré)
#define HEART_SPACING 5

#define FONT_PATH "assets/fonts/LibertinusSerifDisplay-Regular.ttf"
#define FONT_SIZE 28

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
    SDL_Texture *tex_bg_menu; // Fond du menu
    SDL_Texture *tex_bg_game; // Fond du jeu
    SDL_Texture *tex_heart_full;
    SDL_Texture *tex_heart_empty;
    SDL_Texture *blur_texture;
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
    ctx.font_title = TTF_OpenFont(FONT_PATH, 64);
    if (!ctx.font || !ctx.font_title)
    {
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
    ctx.tex_enemy1[0] = load_texture(IMG_ENEMY1_A);
    ctx.tex_enemy1[1] = load_texture(IMG_ENEMY1_B);

    ctx.tex_enemy2[0] = load_texture(IMG_ENEMY2_A);
    ctx.tex_enemy2[1] = load_texture(IMG_ENEMY2_B);

    ctx.tex_enemy3[0] = load_texture(IMG_ENEMY3_A);
    ctx.tex_enemy3[1] = load_texture(IMG_ENEMY3_B);

    ctx.tex_player = load_texture(IMG_PLAYER);
    ctx.tex_bullet = load_texture(IMG_BULLET);
    ctx.tex_bg_menu = load_texture(IMG_BG_MENU);
    ctx.tex_bg_game = load_texture(IMG_BG_GAME);

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

    SDL_Quit();
}

// 3. Rendu
static void sdl_render(const GameModel *model)
{
    // 1. Nettoyage écran
    SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255); // Fond noir pur
    SDL_RenderClear(ctx.renderer);

    // --- GESTION DU MENU ---
    if (model->state == STATE_MENU)
    {
        if (ctx.tex_bg_menu)
        {
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu, NULL, NULL);
        }
        // A. Le Titre (Vert Alien)
        SDL_Color color_title = {50, 255, 50, 255};
        draw_text_centered("SPACE INVADERS", WIN_HEIGHT / 4, color_title, ctx.font_title);

        // B. Les Options
        const char *options[] = {"JOUER", "CHARGER", "QUITTER"};
        SDL_Color col_selected = {255, 255, 255, 255}; // Blanc brillant
        SDL_Color col_normal = {255, 255, 255, 255};   // Gris foncé

        for (int i = 0; i < 3; i++)
        {
            char buffer[64];
            SDL_Color c;
            int y_pos = (WIN_HEIGHT / 2) + (i * 60);

            if (i == model->menu_selection)
            {
                snprintf(buffer, sizeof(buffer), ">  %s  <", options[i]);
                c = col_selected;
            }
            else
            {
                snprintf(buffer, sizeof(buffer), "%s", options[i]);
                c = col_normal;
            }
            draw_text_centered(buffer, y_pos, c, ctx.font);
        }

        SDL_Color col_help = {0, 150, 255, 255};
        draw_text_centered("Fleches: Naviguer  |  Espace: Valider", WIN_HEIGHT - 50, col_help, ctx.font);

        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- GESTION DU JEU (Si pas Menu) ---

    // 2. Dessin Joueur -> EN VERT (0, 255, 0)
    if (ctx.tex_bg_game)
    {
        SDL_RenderTexture(ctx.renderer, ctx.tex_bg_game, NULL, NULL);
    }
    SDL_SetTextureColorMod(ctx.tex_player, 0, 255, 0);
    draw_entity(ctx.tex_player, &model->player);

    // --- DESSIN DES ENNEMIS (VERSION ANIMÉE) ---
    // On récupère le frame actuel (0 ou 1)
    int frame = model->animation_frame;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (model->enemies[i].active)
        {
            SDL_Texture *current_tex = NULL;

            if (model->enemies[i].type == ENTITY_ENEMY_TYPE_2)
            {
                current_tex = ctx.tex_enemy2[frame];
                // Type 2 (Milieu) : ORANGE / JAUNE
                SDL_SetTextureColorMod(current_tex, 255, 165, 0);
            }
            else if (model->enemies[i].type == ENTITY_ENEMY_TYPE_3)
            {
                current_tex = ctx.tex_enemy3[frame];
                // Type 3 (Haut - Squid) : ROUGE / ROSE
                SDL_SetTextureColorMod(current_tex, 255, 50, 50);
            }
            else
            {
                current_tex = ctx.tex_enemy1[frame];
                // Type 1 (Bas - Type par défaut) : CYAN / BLEU CLAIR
                SDL_SetTextureColorMod(current_tex, 0, 255, 255);
            }

            draw_entity(current_tex, &model->enemies[i]);

            // IMPORTANT : On remet la couleur à BLANC (Normal) après dessin
            // pour ne pas teinter les autres si on réutilise la texture ailleurs
            SDL_SetTextureColorMod(current_tex, 255, 255, 255);
        }
    }

    // 4. Dessin Balles -> BLANC PUR (ou Rouge pour les ennemis ?)
    // On peut différencier les balles joueur/ennemi si on veut
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (model->bullets[i].active)
        {
            if (model->bullets[i].type == ENTITY_BULLET_ENEMY)
            {
                SDL_SetTextureColorMod(ctx.tex_bullet, 255, 0, 0); // Balle ennemie ROUGE
            }
            else
            {
                SDL_SetTextureColorMod(ctx.tex_bullet, 255, 255, 0); // Balle joueur JAUNE
            }
            draw_entity(ctx.tex_bullet, &model->bullets[i]);
        }
    }
    // 4. Interface en jeu (HUD)
    SDL_Color white = {255, 255, 255, 255};
    char hud_text[64];

    snprintf(hud_text, sizeof(hud_text), "SCORE: %d", model->score);
    draw_text(hud_text, 20, 20, white);

    // Calcul de la position de départ (Haut Droite)
    int total_width = (MAX_LIVES_DISPLAY * HEART_UI_SIZE) + ((MAX_LIVES_DISPLAY - 1) * HEART_SPACING);
    int start_x = WIN_WIDTH - total_width - 20; // Marge de 20px à droite
    int start_y = 20;                           // Marge de 20px en haut

    for (int i = 0; i < MAX_LIVES_DISPLAY; i++)
    {
        SDL_Texture *tex_to_draw;

        // Logique : Si l'index 'i' est inférieur au nombre de vies restantes, c'est un coeur plein.
        // Sinon (si on a perdu cette vie), c'est un coeur vide.
        // Ex: 2 vies restantes -> i=0 (Plein), i=1 (Plein), i=2 (Vide)
        if (i < model->lives)
        {
            tex_to_draw = ctx.tex_heart_full;
        }
        else
        {
            tex_to_draw = ctx.tex_heart_empty;
        }

        // Définition de la zone de dessin sur l'écran (UI)
        SDL_FRect dst_rect = {
            (float)(start_x + i * (HEART_UI_SIZE + HEART_SPACING)), // Position X qui se décale
            (float)start_y,
            (float)HEART_UI_SIZE, // Largeur forcée
            (float)HEART_UI_SIZE  // Hauteur forcée
        };

        // On dessine directement (pas besoin de draw_entity car c'est de l'UI, pas du jeu)
        if (tex_to_draw)
        {
            // Optionnel : remettre la couleur à blanc si tu as joué avec les couleurs avant
            SDL_SetTextureColorMod(tex_to_draw, 255, 255, 255);
            SDL_RenderTexture(ctx.renderer, tex_to_draw, NULL, &dst_rect);
        }
    }

    // 5. Game Over
    if (model->state == STATE_GAME_OVER)
{
    // 1. GÉNÉRATION DU FLOU (une seule fois au moment du Game Over)
    if (ctx.blur_texture == NULL)
    {
        // Création d'une texture 8x plus petite que la fenêtre
        ctx.blur_texture = SDL_CreateTexture(ctx.renderer, SDL_PIXELFORMAT_RGBA8888, 
                                            SDL_TEXTUREACCESS_TARGET, 
                                            WIN_WIDTH / 8, WIN_HEIGHT / 8);
        
        if (ctx.blur_texture) {
            // Activer le filtrage linéaire pour l'effet de flou lors du redimensionnement
            SDL_SetTextureScaleMode(ctx.blur_texture, SDL_SCALEMODE_LINEAR);

            // Rediriger le rendu vers la texture de flou
            SDL_SetRenderTarget(ctx.renderer, ctx.blur_texture);
            
            // DESSINER ICI LE MONDE DU JEU (sans l'UI de Game Over)
            // Exemple : draw_game_world(model, &ctx); 
            
            // Revenir au rendu principal (l'écran)
            SDL_SetRenderTarget(ctx.renderer, NULL);
        }
    }

    // 2. AFFICHAGE DU FLOU
    if (ctx.blur_texture) {
        // On dessine la petite texture sur tout l'écran (SDL3 gère l'étirement flou)
        SDL_RenderTexture(ctx.renderer, ctx.blur_texture, NULL, NULL);
    }

    // 3. VOILE SOMBRE (Overlay)
    // Pour que le texte ressorte bien sur le flou
    SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 160); // Noir à 60% d'opacité
    SDL_RenderFillRect(ctx.renderer, NULL);

    // 4. TITRE "GAME OVER"
    SDL_Color red = {255, 0, 0, 255};
    draw_text_centered("GAME OVER", WIN_HEIGHT / 2 - 80, red, ctx.font_title);

    // 5. SCORE
    char score_final[32];
    SDL_Color white = {255, 255, 255, 255};
    snprintf(score_final, sizeof(score_final), "Score Final: %d", model->score);
    draw_text_centered(score_final, WIN_HEIGHT / 2 - 10, white, ctx.font);

    // 6. MENU REJOUER / QUITTER
    const char *go_options[] = {"REJOUER", "QUITTER"};
    SDL_Color col_sel = {255, 255, 255, 255};
    SDL_Color col_norm = {150, 150, 150, 255};

    for (int i = 0; i < 2; i++)
    {
        char buffer[64];
        SDL_Color c;
        int y_pos = (WIN_HEIGHT / 2) + 60 + (i * 50);

        if (i == model->menu_selection)
        {
            snprintf(buffer, sizeof(buffer), "> %s <", go_options[i]);
            c = col_sel;
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "%s", go_options[i]);
            c = col_norm;
        }
        draw_text_centered(buffer, y_pos, c, ctx.font);
    }
}

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