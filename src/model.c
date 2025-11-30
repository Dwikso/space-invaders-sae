#include "model.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // Pour memset

// --- Constantes de Gameplay ---
#define PLAYER_SPEED 40.0f
#define BULLET_SPEED 60.0f
#define ENEMY_SPEED_BASE 10.0f
#define ENEMY_DROP_HEIGHT 2.0f

#define PLAYER_WIDTH 5
#define PLAYER_HEIGHT 3
#define ENEMY_WIDTH 4
#define ENEMY_HEIGHT 3
#define BULLET_WIDTH 1
#define BULLET_HEIGHT 1

// --- Fonctions Internes (Privées) ---

// Vérifie la collision AABB (Axis-Aligned Bounding Box)
static bool check_collision(Entity *a, Entity *b)
{
    if (!a->active || !b->active)
        return false;

    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

static void spawn_bullet(GameModel *model, float x, float y, float dy, EntityType type)
{
    // Cherche un slot vide dans le tableau de balles
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!model->bullets[i].active)
        {
            model->bullets[i].active = true;
            model->bullets[i].x = x;
            model->bullets[i].y = y;
            model->bullets[i].width = BULLET_WIDTH;
            model->bullets[i].height = BULLET_HEIGHT;
            model->bullets[i].dx = 0;
            model->bullets[i].dy = dy;
            model->bullets[i].type = type;
            return; // Balle créée, on sort
        }
    }
}

static void init_enemies(GameModel *model)
{
    int rows = 3;
    int cols = 8;
    int count = 0;

    // Vide la liste d'abord
    for (int i = 0; i < MAX_ENEMIES; i++)
        model->enemies[i].active = false;

    // Création de la grille d'ennemis
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (count >= MAX_ENEMIES)
                break;

            Entity *e = &model->enemies[count];
            e->active = true;
            e->type = (r == 0) ? ENTITY_ENEMY_FAST : ENTITY_ENEMY_BASIC;
            e->width = ENEMY_WIDTH;
            e->height = ENEMY_HEIGHT;
            // Position initiale (centrée un peu en haut)
            e->x = 10 + c * (ENEMY_WIDTH + 2);
            e->y = 5 + r * (ENEMY_HEIGHT + 2);
            e->dx = ENEMY_SPEED_BASE;
            e->dy = 0;

            count++;
        }
    }
    model->enemy_speed_mult = 1.0f;
    model->direction_enemies = 1; // 1 = Droite, -1 = Gauche
}

// --- Fonctions Publiques ---

GameModel *model_init(void)
{
    GameModel *model = malloc(sizeof(GameModel));
    if (!model)
        return NULL;

    // 1. Setup Général
    model->state = STATE_MENU; // Commence sur le menu
    model->score = 0;
    model->lives = 3;
    model->level = 1;

    // 2. Setup Joueur
    model->player.active = true;
    model->player.type = ENTITY_PLAYER;
    model->player.width = PLAYER_WIDTH;
    model->player.height = PLAYER_HEIGHT;
    model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f; // Centré
    model->player.y = GAME_HEIGHT - PLAYER_HEIGHT - 1;    // En bas
    model->player.dx = 0;
    model->player.dy = 0;

    // 3. Setup Projectiles (tous inactifs au début)
    for (int i = 0; i < MAX_BULLETS; i++)
        model->bullets[i].active = false;

    // 4. Setup Ennemis
    init_enemies(model);

    return model;
}

void model_free(GameModel *model)
{
    if (model)
        free(model);
}

void model_handle_input(GameModel *model, GameCommand cmd)
{
    if (model->state == STATE_MENU)
    {
        if (cmd == CMD_SHOOT)
            model->state = STATE_PLAYING;
        if (cmd == CMD_EXIT)
            model->state = STATE_GAME_OVER; // Hack pour quitter
        return;
    }

    // Gestion du mouvement (Input instantané)
    // Note : Dans un vrai jeu, on gérerait mieux les key_up / key_down
    // Ici on simplifie : si on reçoit la commande, on bouge
    const float move_amount = 2.0f; // Sensibilité clavier

    switch (cmd)
    {
    case CMD_MOVE_LEFT:
        model->player.dx = -PLAYER_SPEED;
        break;
    case CMD_MOVE_RIGHT:
        model->player.dx = PLAYER_SPEED;
        break;
    case CMD_SHOOT:
        // Tirer une balle qui monte (dy négatif)
        // On centre la balle par rapport au vaisseau
        spawn_bullet(model,
                     model->player.x + model->player.width / 2 - BULLET_WIDTH / 2,
                     model->player.y - 1,
                     -BULLET_SPEED,
                     ENTITY_BULLET_PLAYER);
        break;
    case CMD_PAUSE:
        if (model->state == STATE_PLAYING)
            model->state = STATE_PAUSED;
        else if (model->state == STATE_PAUSED)
            model->state = STATE_PLAYING;
        break;
    case CMD_NONE:
        model->player.dx = 0; // Arrêt si aucune touche (simplifié)
        break;
    default:
        break;
    }
}

void model_update(GameModel *model, double dt)
{
    if (model->state != STATE_PLAYING)
        return;

    // --- 1. Mise à jour Joueur ---
    model->player.x += model->player.dx * dt;

    // Bornes de l'écran (Clamp)
    if (model->player.x < 0)
        model->player.x = 0;
    if (model->player.x > GAME_WIDTH - model->player.width)
        model->player.x = GAME_WIDTH - model->player.width;

    // --- 2. Mise à jour Ennemis ---
    bool touch_edge = false;
    int active_enemies = 0;

    // Vérifier les bords et compter les vivants
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!model->enemies[i].active)
            continue;
        active_enemies++;

        if ((model->enemies[i].x <= 0 && model->direction_enemies == -1) ||
            (model->enemies[i].x >= GAME_WIDTH - ENEMY_WIDTH && model->direction_enemies == 1))
        {
            touch_edge = true;
        }
    }

    // Victoire ?
    if (active_enemies == 0)
    {
        // Logique de niveau suivant ici (reset ennemis, vitesse++, etc.)
        init_enemies(model);
        model->level++;
        model->enemy_speed_mult += 0.2f;
    }

    // Si un ennemi touche un bord, tout le monde descend et change de direction
    if (touch_edge)
    {
        model->direction_enemies *= -1;
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (model->enemies[i].active)
            {
                model->enemies[i].y += ENEMY_DROP_HEIGHT;
                // Game Over si trop bas
                if (model->enemies[i].y + ENEMY_HEIGHT >= model->player.y)
                {
                    model->state = STATE_GAME_OVER;
                }
            }
        }
    }
    else
    {
        // Sinon déplacement normal latéral
        float current_speed = ENEMY_SPEED_BASE * model->enemy_speed_mult * model->direction_enemies;
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (model->enemies[i].active)
            {
                model->enemies[i].x += current_speed * dt;
            }
        }
    }

    // --- 3. Mise à jour Balles ---
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        Entity *b = &model->bullets[i];
        if (!b->active)
            continue;

        b->y += b->dy * dt;

        // Sortie d'écran ?
        if (b->y < 0 || b->y > GAME_HEIGHT)
        {
            b->active = false;
            continue;
        }

        // Collisions
        if (b->type == ENTITY_BULLET_PLAYER)
        {
            // Contre Ennemis
            for (int e = 0; e < MAX_ENEMIES; e++)
            {
                if (check_collision(b, &model->enemies[e]))
                {
                    model->enemies[e].active = false;
                    b->active = false; // Détruire la balle
                    model->score += 10;
                    break;
                }
            }
        }
        // TODO: Ajouter collision Balle Ennemi vs Joueur ici
    }
}