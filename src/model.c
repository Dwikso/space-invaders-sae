#include "model.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// --- Constantes de Gameplay (Tes valeurs) ---
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

// --- Fonctions Internes ---

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
            return;
        }
    }
}

static void init_enemies(GameModel *model)
{
    int rows = 5;
    int cols = 11;
    int count = 0;

    // Reset
    for (int i = 0; i < MAX_ENEMIES; i++)
        model->enemies[i].active = false;

    // Création de la grille
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            if (count >= MAX_ENEMIES)
                break;

            Entity *e = &model->enemies[count];
            e->active = true;

            // Types
            if (r == 0)
                e->type = ENTITY_ENEMY_TYPE_3; // Haut
            else if (r < 3)
                e->type = ENTITY_ENEMY_TYPE_2; // Milieu
            else
                e->type = ENTITY_ENEMY_TYPE_1; // Bas

            e->width = ENEMY_WIDTH;
            e->height = ENEMY_HEIGHT;

            e->x = 5 + c * (ENEMY_WIDTH + 2);
            e->y = 5 + r * (ENEMY_HEIGHT + 2);

            count++;
        }
    }

    model->enemy_speed_mult = 1.0f;
    model->direction_enemies = 1;

    // YO-YO INIT
    model->drop_direction = 1;
    model->drop_step_count = 0;
}

// --- Fonctions Publiques ---

GameModel *model_init(void)
{
    GameModel *model = malloc(sizeof(GameModel));
    if (!model)
        return NULL;

    model->state = STATE_MENU;
    model->menu_selection = 0;
    model->score = 0;
    model->lives = 3;
    model->level = 1;
    model->hit_timer = 0.0f;

    model->player.active = true;
    model->player.type = ENTITY_PLAYER;
    model->player.width = PLAYER_WIDTH;
    model->player.height = PLAYER_HEIGHT;
    model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
    model->player.y = GAME_HEIGHT - PLAYER_HEIGHT - 1;
    model->player.dx = 0;
    model->player.dy = 0;

    for (int i = 0; i < MAX_BULLETS; i++)
        model->bullets[i].active = false;

    model->animation_frame = 0;
    model->animation_timer = 0;

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
    // --- 1. MENU ---
    if (model->state == STATE_MENU)
    {
        if (cmd == CMD_UP)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = 2;
        }
        else if (cmd == CMD_DOWN)
        {
            model->menu_selection++;
            if (model->menu_selection > 2)
                model->menu_selection = 0;
        }
        else if (cmd == CMD_SHOOT)
        {
            if (model->menu_selection == 0)
            { // JOUER
                model->score = 0;
                model->lives = 3;
                model->level = 1;
                init_enemies(model);
                model->state = STATE_PLAYING;
            }
            else if (model->menu_selection == 2)
            { // QUITTER
                exit(0);
            }
        }
        return;
    }

    // --- 2. GAME OVER ---
    if (model->state == STATE_GAME_OVER)
    {
        if (model->game_over_timer < 1.0f)
            return;
        if (cmd == CMD_EXIT)
            exit(0);
        if (cmd == CMD_UP)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = 2;
        }
        else if (cmd == CMD_DOWN)
        {
            model->menu_selection++;
            if (model->menu_selection > 2)
                model->menu_selection = 0;
        }
        else if (cmd == CMD_SHOOT || cmd == CMD_RETURN)
        {
            if (model->menu_selection == 0)
            {
                // SAUVEGARDER
                printf("Sauvegarde...\n"); // TODO: Ta fonction de save
            }
            else if (model->menu_selection == 1)
            {
                // REJOUER
                model->score = 0;
                model->lives = 3;
                model->level = 1;
                init_enemies(model);
                model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
                model->state = STATE_PLAYING;
            }
            else
            {
                // QUITTER
                exit(0);
            }
        }
        return;
    }

    // --- 3. JEU ---
    if (model->state == STATE_PAUSED)
    {
        if (cmd == CMD_PAUSE)
            model->state = STATE_PLAYING;
        return;
    }

    if (model->state == STATE_PLAYING)
    {
        if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PAUSED;
            return;
        }

        switch (cmd)
        {
        case CMD_MOVE_LEFT:
            model->player.dx = -PLAYER_SPEED;
            break;
        case CMD_MOVE_RIGHT:
            model->player.dx = PLAYER_SPEED;
            break;
        case CMD_SHOOT:
            spawn_bullet(model,
                         model->player.x + model->player.width / 2.0f - BULLET_WIDTH / 2.0f,
                         model->player.y - 1,
                         -BULLET_SPEED,
                         ENTITY_BULLET_PLAYER);
            break;
        case CMD_NONE:
            model->player.dx = 0;
            break;
        default:
            break;
        }
    }
}

void model_update(GameModel *model, double dt)
{
    if (model->state == STATE_GAME_OVER)
    {
        model->game_over_timer += dt; // <--- CETTE LIGNE EST CRUCIALE !
        return;                       // On arrête ici pour ne pas bouger les ennemis
    }
    if (model->state != STATE_PLAYING)
        return;

    if (model->hit_timer > 0)
    {
        model->hit_timer -= dt;
        // Pendant l'explosion, on peut empêcher le joueur de bouger si tu veux :
        // model->player.dx = 0;
    }
    // --- A. Mise à jour Joueur ---
    model->player.x += model->player.dx * dt;
    if (model->player.x < 0)
        model->player.x = 0;
    if (model->player.x > GAME_WIDTH - model->player.width)
        model->player.x = GAME_WIDTH - model->player.width;

    // --- B. Animation ---
    model->animation_timer += dt;
    if (model->animation_timer >= 0.5f)
    {
        model->animation_frame = !model->animation_frame;
        model->animation_timer = 0;
    }

    // --- C. Gestion Ennemis (Mouvement) ---
    bool touch_edge = false;
    int active_enemies = 0;

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

    if (active_enemies == 0)
    {
        init_enemies(model);
        model->level++;
        model->enemy_speed_mult += 0.2f;
        return;
    }

    if (touch_edge)
    {
        model->direction_enemies *= -1;

        // Logique YO-YO
        float move_y = 0;
        if (model->drop_direction == 1)
        { // Descente
            move_y = ENEMY_DROP_HEIGHT;
            model->drop_step_count++;
            if (model->drop_step_count >= 4)
                model->drop_direction = -1;
        }
        else
        { // Montée
            move_y = -ENEMY_DROP_HEIGHT;
            model->drop_step_count--;
            if (model->drop_step_count <= 0)
                model->drop_direction = 1;
        }

        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (model->enemies[i].active)
            {
                model->enemies[i].y += move_y;
                model->enemies[i].x += model->direction_enemies * 2.0f;
            }
        }
        model->enemy_speed_mult += 0.05f;
    }
    else
    {
        float speed = ENEMY_SPEED_BASE * model->enemy_speed_mult * model->direction_enemies;
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (model->enemies[i].active)
                model->enemies[i].x += speed * dt;
        }
    }

    // --- D. Tir Ennemi ---
    int fire_chance = 100;
    if (utils_random_int(0, fire_chance) < model->level + 1)
    {
        for (int k = 0; k < 10; k++)
        {
            int idx = utils_random_int(0, MAX_ENEMIES - 1);
            if (model->enemies[idx].active)
            {
                spawn_bullet(model,
                             model->enemies[idx].x + ENEMY_WIDTH / 2,
                             model->enemies[idx].y + ENEMY_HEIGHT,
                             BULLET_SPEED * 0.6f,
                             ENTITY_BULLET_ENEMY);
                break;
            }
        }
    }

    // --- E. Mise à jour Balles ---
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        Entity *b = &model->bullets[i];
        if (!b->active)
            continue;

        b->y += b->dy * dt;

        if (b->y < 0 || b->y > GAME_HEIGHT)
        {
            b->active = false;
            continue;
        }

        if (b->type == ENTITY_BULLET_PLAYER)
        {
            for (int e = 0; e < MAX_ENEMIES; e++)
            {
                if (check_collision(b, &model->enemies[e]))
                {
                    model->enemies[e].active = false;
                    b->active = false;

                    int pts = 10;
                    if (model->enemies[e].type == ENTITY_ENEMY_TYPE_2)
                        pts = 20;
                    if (model->enemies[e].type == ENTITY_ENEMY_TYPE_3)
                        pts = 40;
                    model->score += pts;
                    break;
                }
            }
        }
        else if (b->type == ENTITY_BULLET_ENEMY)
        {
            if (check_collision(b, &model->player))
            {
                b->active = false;

                // SI ON N'EST PAS DÉJÀ EN TRAIN D'EXPLOSER
                if (model->hit_timer <= 0)
                {
                    model->lives--;

                    if (model->lives <= 0)
                    {
                        model->state = STATE_GAME_OVER;
                        model->menu_selection = 0;
                        model->game_over_timer = 0.0f;
                    }
                    else
                    {
                        // ON DÉCLENCHE L'EXPLOSION (pendant 1 seconde)
                        model->hit_timer = 1.0f;

                        // Optionnel : Reset position au centre
                        // model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
                    }
                }
            }
        }
    }
}