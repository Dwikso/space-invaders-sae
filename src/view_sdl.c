#include "view_sdl.h"
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>
#include <SDL3_mixer/SDL_mixer.h>
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
// bullet
#define IMG_BULLET "assets/Projectiles/missile_1.bmp"
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
    SDL_Texture *tex_shields[10];
    SDL_Texture *tex_ufo;        // NOUVEAU
    SDL_Texture *tex_missile[4]; // Pour le joueur (missile_1 à 4)
    SDL_Texture *tex_projectile[4];
    SDL_Texture *tex_expl_enemy; // Pour les aliens normaux
    SDL_Texture *tex_expl_ufo;   // Pour l'UFO
                                 // --- NOUVELLE API AUDIO SDL3 ---
    MIX_Mixer *mixer;            // Le moteur audio principal

    MIX_Audio *snd_shoot; // Remplace Mix_Chunk
    MIX_Audio *snd_killed;
    MIX_Audio *snd_explosion;
    MIX_Audio *snd_ufo;
    MIX_Audio *snd_beat[4];
    MIX_Audio *snd_game_over;
    MIX_Audio *snd_level_up;
    MIX_Audio *snd_select;  // Le "Bip"
    MIX_Audio *snd_menu_bg; // La musique de fond
    MIX_Track *track_menu;
    // Pour l'UFO qui boucle, on a besoin d'une "Piste" (Track)
    MIX_Track *track_ufo;

    int ufo_channel; // Pour pouvoir arrêter le son de l'UFO
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
    // 1. Initialisation SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO))
    {
        fprintf(stderr, "Erreur SDL_Init: %s\n", SDL_GetError());
        return false;
    }

    if (!TTF_Init())
    {
        fprintf(stderr, "Erreur TTF_Init: %s\n", SDL_GetError());
        return false;
    }

    // Initialisation de la LIBRAIRIE Mixer
    if (!MIX_Init())
    {
        fprintf(stderr, "Attention : Erreur MIX_Init : %s\n", SDL_GetError());
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

    SDL_SetRenderLogicalPresentation(ctx.renderer, WIN_WIDTH, WIN_HEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SCALE_X = (float)WIN_WIDTH / GAME_WIDTH;
    SCALE_Y = (float)WIN_HEIGHT / GAME_HEIGHT;

    // --- Chargement des textures ---
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
    ctx.tex_ufo = load_texture(IMG_UFO);

    ctx.tex_missile[0] = IMG_LoadTexture(ctx.renderer, "assets/missiles/missile_1.bmp");
    ctx.tex_missile[1] = IMG_LoadTexture(ctx.renderer, "assets/missiles/missile_2.bmp");
    ctx.tex_missile[2] = IMG_LoadTexture(ctx.renderer, "assets/missiles/missile_3.bmp");
    ctx.tex_missile[3] = IMG_LoadTexture(ctx.renderer, "assets/missiles/missile_4.bmp");

    ctx.tex_projectile[0] = IMG_LoadTexture(ctx.renderer, "assets/projectiles/projectileA_1.bmp");
    ctx.tex_projectile[1] = IMG_LoadTexture(ctx.renderer, "assets/projectiles/projectileA_2.bmp");
    ctx.tex_projectile[2] = IMG_LoadTexture(ctx.renderer, "assets/projectiles/projectileA_3.bmp");
    ctx.tex_projectile[3] = IMG_LoadTexture(ctx.renderer, "assets/projectiles/projectileA_4.bmp");

    ctx.tex_expl_enemy = IMG_LoadTexture(ctx.renderer, "assets/explosions/enemyExplosion.bmp");
    ctx.tex_expl_ufo = IMG_LoadTexture(ctx.renderer, "assets/explosions/ufoExplosion.bmp");

    // --- AUDIO CONFIGURATION ---

    // 1. Création du Mixer
    SDL_AudioSpec spec = {SDL_AUDIO_S16, 2, 44100};
    ctx.mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);

    if (!ctx.mixer)
    {
        printf("Erreur création Mixer: %s\n", SDL_GetError());
        // On initialise à NULL pour éviter les plantages plus tard
        ctx.snd_game_over = NULL;
        ctx.snd_level_up = NULL;
    }
    else
    {
        // 2. Chargement des sons (MAINTENANT QUE LE MIXER EXISTE)

        // --- CORRECTEMENT PLACÉ ICI ---
        ctx.snd_game_over = MIX_LoadAudio(ctx.mixer, "assets/audio/gameOverSound.wav", true);
        ctx.snd_level_up = MIX_LoadAudio(ctx.mixer, "assets/audio/levelUpSound.wav", true);
        // ------------------------------

        ctx.snd_shoot = MIX_LoadAudio(ctx.mixer, "assets/audio/shootSound.wav", true);
        ctx.snd_killed = MIX_LoadAudio(ctx.mixer, "assets/audio/invaderKilledSound.wav", true);
        ctx.snd_explosion = MIX_LoadAudio(ctx.mixer, "assets/audio/explosionSound.wav", true);
        ctx.snd_ufo = MIX_LoadAudio(ctx.mixer, "assets/audio/ufoSound.wav", true);

        ctx.snd_beat[0] = MIX_LoadAudio(ctx.mixer, "assets/audio/fastinvader1.wav", true);
        ctx.snd_beat[1] = MIX_LoadAudio(ctx.mixer, "assets/audio/fastinvader2.wav", true);
        ctx.snd_beat[2] = MIX_LoadAudio(ctx.mixer, "assets/audio/fastinvader3.wav", true);
        ctx.snd_beat[3] = MIX_LoadAudio(ctx.mixer, "assets/audio/fastinvader4.wav", true);
        // --- AJOUTER ICI ---
        ctx.snd_select = MIX_LoadAudio(ctx.mixer, "assets/audio/selectSound.wav", true);
        ctx.snd_menu_bg = MIX_LoadAudio(ctx.mixer, "assets/audio/menuSound.wav", true);

        // Création de la piste pour la musique de menu
        if (ctx.snd_menu_bg)
        {
            ctx.track_menu = MIX_CreateTrack(ctx.mixer);
            if (ctx.track_menu)
            {
                MIX_SetTrackAudio(ctx.track_menu, ctx.snd_menu_bg);
            }
        }
        else
        {
            ctx.track_menu = NULL;
        }
        // Track UFO
        if (ctx.snd_ufo)
        {
            ctx.track_ufo = MIX_CreateTrack(ctx.mixer);
            if (ctx.track_ufo)
            {
                MIX_SetTrackAudio(ctx.track_ufo, ctx.snd_ufo);
            }
        }
        else
        {
            ctx.track_ufo = NULL;
        }
    }

    // Vérifications de chargement (Logs)
    if (!ctx.snd_game_over)
        printf("Attention: gameOverSound.wav non trouvé\n");
    if (!ctx.snd_level_up)
        printf("Attention: levelUpSound.wav non trouvé\n");

    if (!ctx.snd_shoot || !ctx.snd_beat[0])
        printf("Attention: Certains sons manquent dans assets/audio/\n");
    if (!ctx.tex_expl_enemy || !ctx.tex_expl_ufo)
        printf("Attention: Textures d'explosion manquantes.\n");
    if (!ctx.tex_missile[0] || !ctx.tex_projectile[0])
        printf("Attention : Certaines textures de projectiles n'ont pas ete chargees !\n");
    if (!ctx.tex_ufo)
        SDL_Log("Attention: UFO texture manquante !");

    // Chargement Boucliers
    ctx.tex_shields[0] = load_texture("assets/shelter/shelter_full.bmp");
    for (int i = 1; i <= 9; i++)
    {
        char path[64];
        snprintf(path, sizeof(path), "assets/shelter/shelterDamaged_%d.bmp", i);
        ctx.tex_shields[i] = load_texture(path);
        if (!ctx.tex_shields[i])
            SDL_Log("Erreur chargement: %s", path);
    }

    if (!ctx.tex_bg_menu || !ctx.tex_bg_game)
        SDL_Log("Attention : Fonds d'écran non trouvés !");

    ctx.tex_heart_full = load_texture(IMG_HEART_FULL);
    ctx.tex_heart_empty = load_texture(IMG_HEART_EMPTY);

    if (!ctx.tex_heart_full || !ctx.tex_heart_empty)
        SDL_Log("Attention : Images des coeurs manquantes !");

    return true;
}
// 2. Fermeture
static void sdl_close(void)
{
    // Nettoyage Tracks
    if (ctx.track_ufo)
        MIX_DestroyTrack(ctx.track_ufo);

    // Nettoyage Audio
    if (ctx.snd_shoot)
        MIX_DestroyAudio(ctx.snd_shoot);
    if (ctx.snd_killed)
        MIX_DestroyAudio(ctx.snd_killed);
    if (ctx.snd_explosion)
        MIX_DestroyAudio(ctx.snd_explosion);
    if (ctx.snd_ufo)
        MIX_DestroyAudio(ctx.snd_ufo);
    for (int i = 0; i < 4; i++)
        if (ctx.snd_beat[i])
            MIX_DestroyAudio(ctx.snd_beat[i]);
    if (ctx.track_menu)
        MIX_DestroyTrack(ctx.track_menu); // Détruire la piste
    if (ctx.snd_select)
        MIX_DestroyAudio(ctx.snd_select);
    if (ctx.snd_menu_bg)
        MIX_DestroyAudio(ctx.snd_menu_bg);
    // Nettoyage Mixer
    if (ctx.mixer)
        MIX_DestroyMixer(ctx.mixer);
    if (ctx.tex_ufo)
        SDL_DestroyTexture(ctx.tex_ufo);
    // --- NETTOYAGE NOUVEAU ---
    for (int i = 0; i < 4; i++)
    {
        if (ctx.tex_missile[i])
            SDL_DestroyTexture(ctx.tex_missile[i]);
        if (ctx.tex_projectile[i])
            SDL_DestroyTexture(ctx.tex_projectile[i]);
    }
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
    // --- NETTOYAGE ---
    if (ctx.tex_expl_enemy)
        SDL_DestroyTexture(ctx.tex_expl_enemy);
    if (ctx.tex_expl_ufo)
        SDL_DestroyTexture(ctx.tex_expl_ufo);
    // -----------------
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
    if (ctx.tex_ufo)
        SDL_DestroyTexture(ctx.tex_ufo);
    // Dans sdl_close :
    for (int i = 0; i < 10; i++)
    {
        if (ctx.tex_shields[i])
            SDL_DestroyTexture(ctx.tex_shields[i]);
    }
    MIX_Quit();
    SDL_Quit();
}

// src/view_sdl.c

static void draw_game_world(const GameModel *model)
{
    // --- 1. CALCUL DU TREMBLEMENT (SHAKE) ---
    int shake_x = 0;
    int shake_y = 0;

    // ON NE CALCULE LE TREMBLEMENT QUE SI LE JEU TOURNE VRAIMENT
    if (model->state == STATE_PLAYING)
    {
        if (model->hit_timer > 0)
        {
            // On est touché ET on joue : On génère du bruit
            shake_x = (rand() % 11) - 5;
            shake_y = (rand() % 11) - 5;
        }
        else
        {
            // Tout va bien : Pas de tremblement
            shake_x = 0;
            shake_y = 0;
        }
    }
    // --- A. FOND (Avec Shake) ---
    if (ctx.tex_bg_game)
    {
        // ERREUR AVANT : On demandait la taille réelle (SDL_GetRenderOutputSize)
        // CORRECTION : On utilise la taille LOGIQUE du jeu (GAME_WIDTH, GAME_HEIGHT)
        // SDL3 fera le calcul d'étirement tout seul comme pour les menus.

        SDL_FRect bg_rect = {
            (float)shake_x, // Le tremblement
            (float)shake_y,
            (float)GAME_WIDTH * 14,  // 1280 (et pas la taille de la fenêtre !)
            (float)GAME_HEIGHT * 16, // 720
        };

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
            SDL_Texture *t = NULL;

            // --- NOUVEAU : LOGIQUE D'AFFICHAGE ---
            if (model->enemies[i].exploding)
            {
                // Si il explose, on affiche la texture d'explosion
                t = ctx.tex_expl_enemy;
                // Pas besoin de couleur spéciale pour l'explosion
                SDL_SetTextureColorMod(t, 255, 255, 255);
            }
            else
            {
                // Sinon, logique normale d'animation et de couleur
                t = ctx.tex_enemy1[frame];
                if (model->enemies[i].type == ENTITY_ENEMY_TYPE_2)
                {
                    t = ctx.tex_enemy2[frame];
                    SDL_SetTextureColorMod(t, 255, 165, 0);
                }
                else if (model->enemies[i].type == ENTITY_ENEMY_TYPE_3)
                {
                    t = ctx.tex_enemy3[frame];
                    SDL_SetTextureColorMod(t, 255, 50, 50);
                }
                else
                {
                    SDL_SetTextureColorMod(t, 0, 255, 255);
                }
            }
            // ------------------------------------

            draw_entity(t, &model->enemies[i], shake_x, shake_y);

            // Reset couleur par sécurité si c'était un ennemi normal
            if (!model->enemies[i].exploding)
            {
                SDL_SetTextureColorMod(t, 255, 255, 255);
            }
        }
    }
    // --- DESSIN DE L'OVNI ---
    if (model->ufo.active)
    {
        SDL_Texture *t = NULL;

        // --- CHOIX DE LA TEXTURE ---
        if (model->ufo.exploding)
        {
            t = ctx.tex_expl_ufo;
        }
        else
        {
            t = ctx.tex_ufo;
            SDL_SetTextureColorMod(t, 255, 0, 255); // Magenta si normal
        }
        // ---------------------------

        SDL_FRect dst = {
            (model->ufo.x * SCALE_X) + shake_x,
            (model->ufo.y * SCALE_Y) + shake_y,
            model->ufo.width * SCALE_X,
            model->ufo.height * SCALE_Y};
        SDL_RenderTexture(ctx.renderer, t, NULL, &dst);

        SDL_SetTextureColorMod(t, 255, 255, 255); // Reset couleur
    }

    // --- DESSIN DES BOUCLIERS (Avec Shake si besoin) ---
    for (int i = 0; i < MAX_SHIELDS; i++)
    {
        if (model->shields[i].active)
        {

            // Calcul de l'index de texture
            // Ex: Health 10 -> index 0 (Full)
            // Ex: Health 9  -> index 1 (Damage 1)
            int tex_index = 10 - model->shields[i].health;

            // Sécurité pour ne pas dépasser le tableau
            if (tex_index < 0)
                tex_index = 0;
            if (tex_index > 9)
                tex_index = 9;

            SDL_Texture *t = ctx.tex_shields[tex_index];

            // On teinte en vert "Alien" classique (ou blanc si tu préfères)
            SDL_SetTextureColorMod(t, 0, 255, 0);

            // On utilise ton draw_entity modifié (avec shake_x/shake_y)
            // Note: draw_entity attend un "Entity*", mais ici on a un "Shield*".
            // Comme on ne peut pas passer le Shield directement s'il n'est pas "Entity",
            // on fait le dessin manuellement ici (ou tu peux caster si les structures sont identiques).

            // DESSIN MANUEL RAPIDE :
            SDL_FRect dst = {
                (model->shields[i].x * SCALE_X) + shake_x,
                (model->shields[i].y * SCALE_Y) + shake_y,
                model->shields[i].width * SCALE_X,
                model->shields[i].height * SCALE_Y};
            SDL_RenderTexture(ctx.renderer, t, NULL, &dst);
        }
    }

    // --- D. BALLES (AVEC SPRITES ANIMÉS ET SHAKE) ---
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        const Entity *b = &model->bullets[i]; // Pointeur raccourci pour lisibilité

        if (b->active)
        {
            SDL_Texture *textureToDraw = NULL;

            // 1. Choix de la texture selon le type et l'animation (0, 1, 2, 3)
            if (b->type == ENTITY_BULLET_PLAYER)
            {
                textureToDraw = ctx.tex_missile[b->anim_frame];
            }
            else if (b->type == ENTITY_BULLET_ENEMY)
            {
                textureToDraw = ctx.tex_projectile[b->anim_frame];
            }

            // 2. Dessin
            if (textureToDraw)
            {
                // On définit une taille visuelle fixe pour que le sprite soit beau (ex: 12x20)
                // On ajoute le SCALE pour l'écran et le SHAKE pour le tremblement
                SDL_FRect dst = {
                    (b->x * SCALE_X) + shake_x,
                    (b->y * SCALE_Y) + shake_y,
                    SCALE_X, // Largeur visuelle
                    SCALE_Y  // Hauteur visuelle
                };

                // On ne met pas de couleur (SDL_SetTextureColorMod) pour garder les couleurs du dessin bmp
                SDL_RenderTexture(ctx.renderer, textureToDraw, NULL, &dst);
            }
            else
            {
                // FALLBACK : Si l'image n'est pas chargée, on dessine l'ancien carré
                if (b->type == ENTITY_BULLET_ENEMY)
                    SDL_SetTextureColorMod(ctx.tex_bullet, 255, 0, 0);
                else
                    SDL_SetTextureColorMod(ctx.tex_bullet, 255, 255, 0);

                draw_entity(ctx.tex_bullet, b, shake_x, shake_y);
            }
        }
    }
}

// 3. Rendu
static void sdl_render(const GameModel *model)
{
    // Astuce pour modifier les flags
    GameModel *mutable_model = (GameModel *)model;

    if (ctx.mixer)
    {
        // ---------------------------------------------------------
        // 1. GESTION PAUSE GLOBALE (PRIORITÉ ABSOLUE)
        // ---------------------------------------------------------
        bool game_frozen = (model->state == STATE_PAUSED ||
                            model->state == STATE_CONFIRM_QUIT ||
                            model->state == STATE_SAVE_SELECT ||
                            model->state == STATE_SAVE_INPUT);

        if (game_frozen)
        {
            MIX_PauseAllTracks(ctx.mixer); // Coupe TOUT (y compris la musique menu si elle jouait)
        }
        else if (model->state == STATE_PLAYING || model->state == STATE_SAVE_SUCCESS)
        {
            MIX_ResumeAllTracks(ctx.mixer); // Réactive le moteur sonore
        }

        // ---------------------------------------------------------
        // 2. MUSIQUE DE MENU (Seulement si pas en jeu et pas gelé)
        // ---------------------------------------------------------
        if (ctx.track_menu)
        {
            // États où la musique doit tourner
            bool need_menu_music = (model->state == STATE_MENU ||
                                    model->state == STATE_TUTORIAL ||
                                    model->state == STATE_LOAD_MENU ||
                                    model->state == STATE_GAME_OVER);

            if (need_menu_music && !game_frozen)
            {
                if (!MIX_TrackPlaying(ctx.track_menu))
                    MIX_PlayTrack(ctx.track_menu, 0);
                if (MIX_TrackPaused(ctx.track_menu))
                    MIX_ResumeTrack(ctx.track_menu);
            }
            else
            {
                // Si on joue ou qu'on est en pause "technique", on coupe la musique menu
                if (MIX_TrackPlaying(ctx.track_menu))
                    MIX_PauseTrack(ctx.track_menu);
            }
        }

        // ---------------------------------------------------------
        // 3. VOLUME
        // ---------------------------------------------------------
        float gain = model->is_muted ? 0.0f : ((float)model->volume / 100.0f);
        MIX_SetMasterGain(ctx.mixer, gain);

        // ---------------------------------------------------------
        // 4. BRUITAGES (ONE SHOTS) - On les force même si le mixer est chargé
        // ---------------------------------------------------------

        // SON DE VALIDATION (MENU)
        if (model->sounds.play_select_sound)
        {
            if (ctx.snd_select)
                MIX_PlayAudio(ctx.mixer, ctx.snd_select);
            mutable_model->sounds.play_select_sound = false;
        }

        // GAME OVER / LEVEL UP
        if (model->sounds.play_game_over)
        {
            if (ctx.snd_game_over)
                MIX_PlayAudio(ctx.mixer, ctx.snd_game_over);
            mutable_model->sounds.play_game_over = false;
        }
        if (model->sounds.play_level_up)
        {
            if (ctx.snd_level_up)
                MIX_PlayAudio(ctx.mixer, ctx.snd_level_up);
            mutable_model->sounds.play_level_up = false;
        }

        // JEU
        if (!game_frozen && model->state == STATE_PLAYING)
        {
            if (model->sounds.play_shoot && ctx.snd_shoot)
                MIX_PlayAudio(ctx.mixer, ctx.snd_shoot);

            if (model->sounds.play_invader_killed && ctx.snd_killed)
                MIX_PlayAudio(ctx.mixer, ctx.snd_killed);

            if (model->sounds.play_player_explosion && ctx.snd_explosion)
                MIX_PlayAudio(ctx.mixer, ctx.snd_explosion);

            if (model->sounds.play_beat && ctx.snd_beat[model->sounds.beat_index])
                MIX_PlayAudio(ctx.mixer, ctx.snd_beat[model->sounds.beat_index]);

            // Reset flags
            mutable_model->sounds.play_shoot = false;
            mutable_model->sounds.play_invader_killed = false;
            mutable_model->sounds.play_player_explosion = false;
            mutable_model->sounds.play_beat = false;

            // UFO Loop
            if (ctx.track_ufo)
            {
                if (model->sounds.ufo_loopING)
                {
                    if (!MIX_TrackPlaying(ctx.track_ufo))
                        MIX_PlayTrack(ctx.track_ufo, 0);
                    if (MIX_TrackPaused(ctx.track_ufo))
                        MIX_ResumeTrack(ctx.track_ufo);
                }
                else
                {
                    if (MIX_TrackPlaying(ctx.track_ufo))
                        MIX_PauseTrack(ctx.track_ufo);
                }
            }
        }
    }
    // --- JEU (STATE_PLAYING) ---
    if (model->state == STATE_PLAYING)
    {
        if (ctx.tex_blur)
        {
            SDL_DestroyTexture(ctx.tex_blur);
            ctx.tex_blur = NULL;
        }

        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);

        // 1. DESSIN DU MONDE
        draw_game_world(model);

        // 2. HUD
        SDL_Color white = {255, 255, 255, 255};
        char hud[64];
        snprintf(hud, sizeof(hud), "SCORE: %d   NIVEAU: %d", model->score, model->level);
        draw_text(hud, 20, 20, white);

        // Coeurs
        int start_x = WIN_WIDTH - 20;
        int hearts_to_draw = (model->lives > MAX_LIVES_DISPLAY) ? model->lives : MAX_LIVES_DISPLAY;

        for (int i = 0; i < hearts_to_draw; i++)
        {
            int current_x = start_x - ((i + 1) * (HEART_UI_SIZE + HEART_SPACING));
            SDL_Texture *hrt = (i < model->lives) ? ctx.tex_heart_full : ctx.tex_heart_empty;
            SDL_FRect r = {(float)current_x, 20, HEART_UI_SIZE, HEART_UI_SIZE};

            if (hrt)
            {
                if (i >= MAX_LIVES_DISPLAY)
                    SDL_SetTextureColorMod(hrt, 255, 215, 0);
                else
                    SDL_SetTextureColorMod(hrt, 255, 255, 255);

                SDL_RenderTexture(ctx.renderer, hrt, NULL, &r);
            }
        }

        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- MENU PAUSE (Gestion Volume Ajoutée) ---
    // src/view_sdl.c -> sdl_render

    // --- MENU PAUSE ---
    // src/view_sdl.c -> sdl_render

    if (model->state == STATE_PAUSED)
    {
        draw_game_world(model);

        // Fond sombre
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 180);
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        SDL_Color white = {255, 255, 255, 255};
        draw_text_centered("PAUSE", WIN_HEIGHT / 4, white, ctx.font_title);

        // --- C'EST ICI QU'IL FAUT LES 4 OPTIONS ---
        const char *opts[] = {
            "REPRENDRE",
            "VOLUME",
            "SAUVEGARDER ET QUITTER",
            "QUITTER SANS SAUVEGARDER"};
        // ------------------------------------------

        int y_start = WIN_HEIGHT / 2 - 50;

        for (int i = 0; i < 4; i++) // Boucle sur 4
        {
            SDL_Color c = (i == model->menu_selection) ? (SDL_Color){255, 255, 0, 255} : (SDL_Color){150, 150, 150, 255};

            if (i == 1) // VOLUME
            {
                char vol_str[64];
                if (model->is_muted)
                    snprintf(vol_str, sizeof(vol_str), "SON: OFF");
                else
                    snprintf(vol_str, sizeof(vol_str), "SON: < %d%% >", model->volume);
                draw_text_centered(vol_str, y_start + i * 60, c, ctx.font);
            }
            else // AUTRES
            {
                char buf[128];
                if (i == model->menu_selection)
                    snprintf(buf, sizeof(buf), "> %s <", opts[i]);
                else
                    snprintf(buf, sizeof(buf), "%s", opts[i]);
                draw_text_centered(buf, y_start + i * 60, c, ctx.font);
            }
        }
        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- MENU PRINCIPAL (Modifié pour inclure VOLUME) ---
    if (model->state == STATE_MENU)
    {
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);
        if (ctx.tex_bg_menu)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu, NULL, NULL);

        SDL_Color green = {50, 255, 50, 255};
        draw_text_centered("SPACE INVADERS", WIN_HEIGHT / 4, green, ctx.font_title);

        // 5 Options maintenant : Jouer, Tuto, Charger, Volume, Quitter
        const char *opts[] = {"JOUER", "TUTORIEL", "CHARGER", "VOLUME", "QUITTER"};
        int y_start = WIN_HEIGHT / 2;

        for (int i = 0; i < 5; i++)
        {
            SDL_Color c = (i == model->menu_selection) ? (SDL_Color){255, 255, 0, 255} : (SDL_Color){100, 100, 100, 255};

            // Si c'est la ligne VOLUME (Index 3)
            if (i == 3)
            {
                char vol_str[64];
                if (model->is_muted)
                {
                    snprintf(vol_str, sizeof(vol_str), "VOLUME: [MUTE]");
                }
                else
                {
                    // Barre visuelle "|||||-----"
                    char bars[11] = {0};
                    int nb_bars = model->volume / 10;
                    for (int k = 0; k < 10; k++)
                        bars[k] = (k < nb_bars) ? '|' : '-';
                    bars[10] = '\0';

                    snprintf(vol_str, sizeof(vol_str), "VOLUME: [%s] %d%%", bars, model->volume);
                }
                draw_text_centered(vol_str, y_start + i * 50, c, ctx.font);
            }
            else
            {
                // Affichage standard
                char buf[64];
                if (i == model->menu_selection)
                    snprintf(buf, sizeof(buf), "> %s <", opts[i]);
                else
                    snprintf(buf, sizeof(buf), "%s", opts[i]);

                draw_text_centered(buf, y_start + i * 50, c, ctx.font);
            }
        }
        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- RENDU TUTORIEL ---
    if (model->state == STATE_TUTORIAL)
    {
        // ... (Ton code tutoriel inchangé) ...
        // (Pour alléger la réponse, je reprends ta logique exacte)
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255);
        SDL_RenderClear(ctx.renderer);

        if (ctx.tex_bg_menu_1)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);
        else if (ctx.tex_bg_menu)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu, NULL, NULL);

        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        SDL_Color title_col = {0, 255, 255, 255};
        SDL_Color text_col = {255, 255, 255, 255};
        SDL_Color cyan = {0, 255, 255, 255};
        SDL_Color orange = {255, 165, 0, 255};
        SDL_Color red = {255, 50, 50, 255};
        SDL_Color magenta = {255, 0, 255, 255};
        SDL_Color green = {0, 255, 0, 255};

        draw_text_centered("COMMENT JOUER ?", 50, title_col, ctx.font_title);

        int start_y = 200;
        int spacing = 60;
        int icon_x = (GAME_WIDTH / 2) + 20;
        int text_x = (GAME_WIDTH / 2) + 100;

        draw_text_centered("Deplacez-vous avec les fleches", (start_y - 70), text_col, ctx.font);
        draw_text_centered("Tirez avec ESPACE ou ENTREE", (start_y - 20), text_col, ctx.font);
        draw_text_centered("Le but du jeu est de tuer tous les aliens.", start_y + 30, text_col, ctx.font);

        int aliens_start_y = start_y + 120;

        // Alien 1
        SDL_FRect r1 = {(float)icon_x, (float)aliens_start_y, 40, 40};
        if (ctx.tex_enemy1[0])
        {
            SDL_SetTextureColorMod(ctx.tex_enemy1[0], 0, 255, 255);
            SDL_RenderTexture(ctx.renderer, ctx.tex_enemy1[0], NULL, &r1);
            SDL_SetTextureColorMod(ctx.tex_enemy1[0], 255, 255, 255);
        }
        draw_text("= 10 POINTS", text_x, aliens_start_y, cyan);

        // Alien 2
        SDL_FRect r2 = {(float)icon_x, (float)aliens_start_y + spacing, 40, 40};
        if (ctx.tex_enemy2[0])
        {
            SDL_SetTextureColorMod(ctx.tex_enemy2[0], 255, 165, 0);
            SDL_RenderTexture(ctx.renderer, ctx.tex_enemy2[0], NULL, &r2);
            SDL_SetTextureColorMod(ctx.tex_enemy2[0], 255, 255, 255);
        }
        draw_text("= 20 POINTS", text_x, aliens_start_y + spacing, orange);

        // Alien 3
        SDL_FRect r3 = {(float)icon_x, (float)aliens_start_y + (spacing * 2), 40, 40};
        if (ctx.tex_enemy3[0])
        {
            SDL_SetTextureColorMod(ctx.tex_enemy3[0], 255, 50, 50);
            SDL_RenderTexture(ctx.renderer, ctx.tex_enemy3[0], NULL, &r3);
            SDL_SetTextureColorMod(ctx.tex_enemy3[0], 255, 255, 255);
        }
        draw_text("= 30 POINTS", text_x, aliens_start_y + (spacing * 2), red);

        // UFO
        SDL_FRect rUfo = {(float)icon_x - 10, (float)aliens_start_y + (spacing * 3), 60, 30};
        if (ctx.tex_ufo)
        {
            SDL_SetTextureColorMod(ctx.tex_ufo, 255, 0, 255);
            SDL_RenderTexture(ctx.renderer, ctx.tex_ufo, NULL, &rUfo);
            SDL_SetTextureColorMod(ctx.tex_ufo, 255, 255, 255);
        }
        draw_text("= ??? MYSTERE", text_x, aliens_start_y + (spacing * 3), magenta);

        // Bouclier
        SDL_FRect rShield = {(float)icon_x, (float)aliens_start_y + (spacing * 4), 40, 40};
        if (ctx.tex_shields[0])
        {
            SDL_SetTextureColorMod(ctx.tex_shields[0], 0, 255, 0);
            SDL_RenderTexture(ctx.renderer, ctx.tex_shields[0], NULL, &rShield);
            SDL_SetTextureColorMod(ctx.tex_shields[0], 255, 255, 255);
        }
        draw_text("= PROTECTION", text_x, aliens_start_y + (spacing * 4), green);

        draw_text_centered("(Appuyez sur Entree pour retour)", GAME_HEIGHT - 50, (SDL_Color){150, 150, 150, 255}, ctx.font);
        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- GAME OVER ---
    if (model->state == STATE_GAME_OVER)
    {
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderClear(ctx.renderer);

        if (ctx.tex_bg_menu_1)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);

        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        SDL_Color red = {255, 0, 0, 255};
        draw_text_centered("GAME OVER", WIN_HEIGHT / 4, red, ctx.font_title);

        char s_txt[32];
        SDL_Color white = {255, 255, 255, 255};
        snprintf(s_txt, sizeof(s_txt), "Score Final: %d", model->score);
        draw_text_centered(s_txt, WIN_HEIGHT / 2 - 50, white, ctx.font);

        const char *go_opts[] = {"SAUVEGARDER", "REJOUER", "QUITTER"};
        for (int i = 0; i < 3; i++)
        {
            char buf[512];
            SDL_Color c = (i == model->menu_selection) ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){150, 150, 150, 255};

            if (i == model->menu_selection)
                snprintf(buf, sizeof(buf), "> %s <", go_opts[i]);
            else
                snprintf(buf, sizeof(buf), "%s", go_opts[i]);

            draw_text_centered(buf, WIN_HEIGHT / 2 + 30 + i * 60, c, ctx.font);
        }
        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- POP-UP CONFIRMATION ---
    // src/view_sdl.c -> sdl_render

    if (model->state == STATE_CONFIRM_QUIT)
    {
        // 1. CHOIX DU FOND (BACKGROUND)

        // Si on vient du JEU ou de la PAUSE -> On affiche le jeu en fond
        if (model->previous_state == STATE_PLAYING || model->previous_state == STATE_PAUSED)
        {
            draw_game_world(model);

            // Voile noir sombre
            SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 230);
            SDL_RenderFillRect(ctx.renderer, NULL);
            SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);
        }
        // Sinon (Game Over, ou autre) -> On affiche le fond style Tutoriel
        else
        {
            SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderClear(ctx.renderer); // Nettoie l'écran

            // Affiche l'image de fond (Menu 1 ou Menu standard)
            if (ctx.tex_bg_menu_1)
                SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);
            else if (ctx.tex_bg_menu)
                SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu, NULL, NULL);

            // Voile noir plus léger (comme le tuto)
            SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 200);
            SDL_RenderFillRect(ctx.renderer, NULL);
            SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);
        }

        // 2. TEXTES & OPTIONS (Reste identique)
        SDL_Color red = {255, 50, 50, 255};
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gray = {180, 180, 180, 255};

        draw_text_centered("ATTENTION !", (WIN_HEIGHT / 3), red, ctx.font_title);

        // Petit détail : on change le message si on est pas en jeu
        if (model->previous_state == STATE_PLAYING || model->previous_state == STATE_PAUSED)
        {
            draw_text_centered("Progression non sauvegardee !", (WIN_HEIGHT / 3) + 60, white, ctx.font);
        }
        else
        {
            draw_text_centered("Voulez-vous quitter le jeu ?", (WIN_HEIGHT / 3) + 60, white, ctx.font);
        }

        draw_text_centered("Confirmer ?", (WIN_HEIGHT / 3) + 90, gray, ctx.font);

        // LES 3 OPTIONS
        const char *opts[] = {
            "OUI, QUITTER",
            "NON (RETOUR)",
            "SAUVEGARDER ET QUITTER"};

        int y_start = (WIN_HEIGHT / 2) + 50;

        for (int i = 0; i < 3; i++)
        {
            SDL_Color c = (i == model->menu_selection) ? (SDL_Color){255, 215, 0, 255} : (SDL_Color){100, 100, 100, 255};

            char buf[64];
            if (i == model->menu_selection)
                snprintf(buf, sizeof(buf), "> %s <", opts[i]);
            else
                snprintf(buf, sizeof(buf), "%s", opts[i]);

            draw_text_centered(buf, y_start + (i * 60), c, ctx.font);
        }

        SDL_RenderPresent(ctx.renderer);
        return;
    }

    // --- SAUVEGARDE INPUT ---
    if (model->state == STATE_SAVE_INPUT)
    {
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderClear(ctx.renderer);
        if (ctx.tex_bg_menu_1)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        SDL_Color yellow = {255, 255, 0, 255};
        SDL_Color white = {255, 255, 255, 255};

        draw_text_centered("NOM DE LA SAUVEGARDE :", (GAME_HEIGHT / 2) + 20, yellow, ctx.font_title);

        char display_buf[64];
        snprintf(display_buf, sizeof(display_buf), "%s_", model->input_buffer);

        draw_text_centered(display_buf, (GAME_HEIGHT / 2) + 100, white, ctx.font);
        draw_text_centered("(Entree pour valider)", (GAME_HEIGHT / 2) + 150, (SDL_Color){100, 100, 100, 255}, ctx.font);

        SDL_RenderPresent(ctx.renderer);
        return;
    }
    // --- RENDU MENU CHARGEMENT (LOAD) ---
    if (model->state == STATE_LOAD_MENU)
    {
        // 1. Fond (identique à STATE_SAVE_INPUT)
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderClear(ctx.renderer);

        // Afficher le fond de menu
        if (ctx.tex_bg_menu_1)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);

        // Voile noir semi-transparent
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        // 2. Titre
        SDL_Color green = {50, 255, 50, 255};
        draw_text_centered("CHOISIR UNE SAUVEGARDE", 100, green, ctx.font_title);

        // 3. Liste des fichiers
        for (int i = 0; i < model->save_file_count; i++)
        {
            SDL_Color c = (i == model->menu_selection)
                              ? (SDL_Color){255, 255, 255, 255}
                              : (SDL_Color){100, 100, 100, 255};

            char buf[512];

            if (i == model->menu_selection)
                snprintf(buf, sizeof(buf), "> %s <", model->save_files[i]);
            else
                snprintf(buf, sizeof(buf), "%s", model->save_files[i]);

            draw_text_centered(buf, 200 + (i * 40), c, ctx.font);
        }

        // 4. Message si aucun fichier
        if (model->save_file_count == 0)
        {
            draw_text_centered("AUCUN FICHIER TROUVE", GAME_HEIGHT / 2,
                               (SDL_Color){255, 0, 0, 255}, ctx.font);
        }

        SDL_RenderPresent(ctx.renderer);
    }
    if (model->state == STATE_OVERWRITE_CONFIRM)
    {
        // 1. Fond (identique à STATE_LOAD_MENU)
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderClear(ctx.renderer);

        // Afficher le fond de menu
        if (ctx.tex_bg_menu_1)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);

        // Voile noir semi-transparent
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        SDL_Color red = {255, 50, 50, 255};
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color yellow = {255, 215, 0, 255};

        // 2. Message d'alerte
        draw_text_centered("ATTENTION !", (GAME_HEIGHT / 2) + 20, red, ctx.font_title);

        char msg[128];
        snprintf(msg, sizeof(msg), "Le fichier '%s.dat' existe deja.", model->input_buffer);
        draw_text_centered(msg, (GAME_HEIGHT / 2) + 100, yellow, ctx.font);

        draw_text_centered("Voulez-vous l'ecraser ?", (GAME_HEIGHT / 2) + 160, white, ctx.font);

        // 3. Options (OUI / NON)
        const char *opts[] = {"OUI", "NON (DUPLIQUE ET RENOMME)"};

        for (int i = 0; i < 2; i++)
        {
            SDL_Color c = (i == model->menu_selection)
                              ? (SDL_Color){255, 255, 255, 255}
                              : (SDL_Color){100, 100, 100, 255};

            char buf[512];

            if (i == model->menu_selection)
                snprintf(buf, sizeof(buf), "> %s <", opts[i]);
            else
                snprintf(buf, sizeof(buf), "%s", opts[i]);

            draw_text_centered(buf, (GAME_HEIGHT / 2) + 210 + (i * 50), c, ctx.font);
        }

        SDL_RenderPresent(ctx.renderer);
    }
    if (model->state == STATE_SAVE_SELECT)
    {
        // 1. Fond (identique à STATE_SAVE_INPUT)
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_RenderClear(ctx.renderer);

        // Afficher le fond de menu
        if (ctx.tex_bg_menu_1)
            SDL_RenderTexture(ctx.renderer, ctx.tex_bg_menu_1, NULL, NULL);

        // Voile noir semi-transparent
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 200);
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        // 2. Titre
        SDL_Color yellow = {255, 215, 0, 255};
        draw_text_centered("CHOISIR L'EMPLACEMENT", 80, yellow, ctx.font_title);

        // 3. Option "NOUVELLE SAUVEGARDE" (Index 0)
        SDL_Color c_new = (model->menu_selection == 0)
                              ? (SDL_Color){50, 255, 50, 255}
                              : (SDL_Color){150, 150, 150, 255};

        char buf_new[64];
        if (model->menu_selection == 0)
            snprintf(buf_new, sizeof(buf_new), "> CREER UNE NOUVELLE SAUVEGARDE  <");
        else
            snprintf(buf_new, sizeof(buf_new), " CREER UNE NOUVELLE SAUVEGARDE ");

        draw_text_centered(buf_new, 180, c_new, ctx.font);

        // 4. Liste des fichiers existants
        for (int i = 0; i < model->save_file_count; i++)
        {
            // L'index dans le menu est décalé de +1 à cause de l'option "Nouvelle"
            int menu_idx = i + 1;

            SDL_Color c = (menu_idx == model->menu_selection)
                              ? (SDL_Color){255, 255, 255, 255}
                              : (SDL_Color){100, 100, 100, 255};

            char buf[512];

            if (menu_idx == model->menu_selection)
                snprintf(buf, sizeof(buf), "> %s <", model->save_files[i]);
            else
                snprintf(buf, sizeof(buf), "%s", model->save_files[i]);

            draw_text_centered(buf, 230 + (i * 45), c, ctx.font);
        }

        SDL_RenderPresent(ctx.renderer);
    }
    // --- ECRAN SUCCES & FERMETURE ---
    if (model->state == STATE_SAVE_SUCCESS)
    {
        // 1. Fond noir ou flou
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 255); // Fond noir complet pour la fin
        SDL_RenderClear(ctx.renderer);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        SDL_Color green = {50, 255, 50, 255};
        SDL_Color white = {255, 255, 255, 255};

        // 2. Messages
        draw_text_centered("SAUVEGARDE REUSSIE !", (WIN_HEIGHT / 2) - 40, green, ctx.font_title);
        draw_text_centered("Le jeu va se fermer...", (WIN_HEIGHT / 2) + 40, white, ctx.font);

        SDL_RenderPresent(ctx.renderer);
        return;
    }
    // src/view_sdl.c -> sdl_render

    // --- POP-UP CONFIRMATION (QUITTER SANS SAUVER) ---
    if (model->state == STATE_CONFIRM_QUIT)
    {
        // 1. On garde le jeu en fond pour le contexte
        draw_game_world(model);

        // 2. Voile noir assez opaque (pour bien séparer)
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, 230); // Très sombre
        SDL_RenderFillRect(ctx.renderer, NULL);
        SDL_SetRenderDrawBlendMode(ctx.renderer, SDL_BLENDMODE_NONE);

        // 3. Textes d'avertissement
        SDL_Color red = {255, 50, 50, 255};
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color gray = {180, 180, 180, 255};

        draw_text_centered("ATTENTION !", (WIN_HEIGHT / 3), red, ctx.font_title);
        draw_text_centered("Toute progression non sauvegardee sera perdue.", (WIN_HEIGHT / 3) + 60, white, ctx.font);
        draw_text_centered("Voulez-vous vraiment quitter ?", (WIN_HEIGHT / 3) + 90, gray, ctx.font);

        // 4. Les Options (OUI / NON)
        const char *opts[] = {"OUI, QUITTER", "NON, RETOUR"};
        int y_start = (WIN_HEIGHT / 2) + 50;

        for (int i = 0; i < 2; i++)
        {
            // Couleur : Sélectionné = Jaune, Sinon = Gris
            SDL_Color c = (i == model->menu_selection) ? (SDL_Color){255, 215, 0, 255} : (SDL_Color){100, 100, 100, 255};

            char buf[64];
            if (i == model->menu_selection)
                snprintf(buf, sizeof(buf), "> %s <", opts[i]);
            else
                snprintf(buf, sizeof(buf), "%s", opts[i]);

            draw_text_centered(buf, y_start + (i * 60), c, ctx.font);
        }

        SDL_RenderPresent(ctx.renderer);
        return;
    }
}
// 4. Entrées
static GameCommand sdl_get_input(GameModel *model)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // 1. Quitter la fenêtre
        if (event.type == SDL_EVENT_QUIT)
            return CMD_EXIT;

        // ============================================================
        // A. MODE SAISIE DE TEXTE (Tapez le nom)
        // ============================================================
        if (model->state == STATE_SAVE_INPUT && event.type == SDL_EVENT_KEY_DOWN)
        {
            SDL_Keycode key = event.key.key;

            if (key == SDLK_RETURN || key == SDLK_KP_ENTER)
                return CMD_RETURN;
            if (key == SDLK_ESCAPE)
                return CMD_PAUSE;

            // Effacer
            if (key == SDLK_BACKSPACE)
            {
                int len = strlen(model->input_buffer);
                if (len > 0)
                    model->input_buffer[len - 1] = '\0';
            }
            // Écrire (Lettres, Chiffres, -, _)
            else if (strlen(model->input_buffer) < MAX_FILENAME_LEN - 1)
            {
                char c = 0;
                // SDL utilise les codes minuscules pour les lettres
                if (key >= SDLK_A && key <= SDLK_Z)
                    c = (char)key;
                if (key >= SDLK_0 && key <= SDLK_9)
                    c = (char)key;
                if (key == SDLK_SPACE)
                    c = '_';
                if (key == SDLK_MINUS)
                    c = '-';

                if (c != 0)
                {
                    int len = strlen(model->input_buffer);
                    model->input_buffer[len] = c;
                    model->input_buffer[len + 1] = '\0';
                }
            }
            return CMD_NONE;
        }

        // ============================================================
        // B. TOUCHES PRESSÉES (MENUS & JEU)
        // ============================================================
        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            SDL_Keycode key = event.key.key;

            switch (key)
            {
            case SDLK_ESCAPE:
                return CMD_PAUSE;
            case SDLK_Q:
                return CMD_EXIT;
            case SDLK_F11:
            {
                SDL_WindowFlags flags = SDL_GetWindowFlags(ctx.window);
                SDL_SetWindowFullscreen(ctx.window, !(flags & SDL_WINDOW_FULLSCREEN));
                break;
            }

            // --- VALIDATION ---
            case SDLK_RETURN:
                return CMD_RETURN;
            case SDLK_KP_ENTER:
                return CMD_RETURN;
            case SDLK_SPACE:
                return CMD_SHOOT; // Espace valide aussi dans les menus

            // --- NAVIGATION (HAUT / BAS) ---
            case SDLK_UP:
                return CMD_UP;
            case SDLK_DOWN:
                return CMD_DOWN;

            // --- NAVIGATION (GAUCHE / DROITE) ---
            // C'est ICI que ça se joue !
            case SDLK_LEFT:
                // Si on joue -> Move Left. Sinon -> Menu Left
                if (model->state == STATE_PLAYING)
                    return CMD_MOVE_LEFT;
                else
                    return CMD_LEFT;

            case SDLK_RIGHT:
                // Si on joue -> Move Right. Sinon -> Menu Right
                if (model->state == STATE_PLAYING)
                    return CMD_MOVE_RIGHT;
                else
                    return CMD_RIGHT;

            // --- COMMANDES SPÉCIALES JEU ---
            case SDLK_P:
                return CMD_PAUSE;

            default:
                break;
            }
        }
    }

    // ============================================================
    // C. JEU FLUIDE (Maintenir la touche)
    // ============================================================
    if (model->state == STATE_PLAYING)
    {
        const bool *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_LEFT])
            return CMD_MOVE_LEFT;
        if (state[SDL_SCANCODE_RIGHT])
            return CMD_MOVE_RIGHT;
        if (state[SDL_SCANCODE_SPACE])
            return CMD_SHOOT;
    }

    return CMD_NONE;
}

// ============================================================
// C. MOUVEMENT CONTINU (Uniquement pendant le JEU)
// ============================================================
// On empêche cela dans les menus pour ne pas faire défiler trop vite

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