#include "model.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h> // Pour mkdir sur Linux/Mac
#include <sys/types.h>
#ifdef _WIN32
#include <direct.h> // Pour _mkdir sur Windows
#endif
#include <dirent.h> // Pour lire le dossier
#include <string.h> // Pour strlen, strcpy...
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
            model->bullets[i].anim_timer = 0.0f;
            model->bullets[i].anim_frame = 0;
            model->bullets[i].width = BULLET_WIDTH;
            model->bullets[i].height = BULLET_HEIGHT;
            model->bullets[i].dx = 0;
            model->bullets[i].dy = dy;
            model->bullets[i].type = type;
            return;
        }
    }
}
static bool save_file_exists(const char *filename)
{
    char path[128];
    snprintf(path, sizeof(path), "sauvegardes/%s", filename);

    FILE *f = fopen(path, "rb");
    if (f)
    {
        fclose(f); // Il existe, on le referme tout de suite
        return true;
    }
    return false;
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
            // --- AJOUTE CES 2 LIGNES ICI ---
            e->exploding = false;
            e->explode_timer = 0.0f;
            // -------------------------------
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
            e->y = 7 + r * (ENEMY_HEIGHT + 2);

            count++;
        }
    }

    model->enemy_speed_mult = 1.0f;
    model->direction_enemies = 1;

    // YO-YO INIT
    model->drop_direction = 1;
    model->drop_step_count = 0;

    // --- RESET UFO POUR LE NIVEAU ---
    model->ufo.active = false;
    model->ufo.hasSpawnedThisLevel = false;
    model->ufo.width = UFO_WIDTH;
    model->ufo.height = UFO_HEIGHT;
    // La position Y est fixe : tout en haut
    model->ufo.y = 4; // Juste au-dessus de la ligne d'ennemis du haut
}

// Fonction pour faire apparaître l'OVNI
static void spawn_ufo(GameModel *model)
{
    model->ufo.active = true;
    model->ufo.hasSpawnedThisLevel = true;
    // --- AJOUTE CES 2 LIGNES ICI ---
    model->ufo.exploding = false;
    model->ufo.explode_timer = 0.0f;
    // -------------------------------

    // Pile ou face : vient de gauche ou de droite ?
    if (rand() % 2 == 0)
    {
        // Commence à gauche, va à droite
        model->ufo.x = -model->ufo.width;
        model->ufo.dx = UFO_SPEED;
    }
    else
    {
        // Commence à droite, va à gauche
        model->ufo.x = GAME_WIDTH;
        model->ufo.dx = -UFO_SPEED;
    }
    // Optionnel : petit log pour le debug
    // printf("UFO Spawned!\n");
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
    model->normal_max_lives = MAX_LIVES_NORMAL; // <-- AJOUTE CA
    model->player.active = true;
    model->player.shoot_timer = 0.0f; // <--- Initialisation importante !
    model->player.type = ENTITY_PLAYER;
    model->player.width = PLAYER_WIDTH;
    model->player.height = PLAYER_HEIGHT;
    model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
    model->player.y = GAME_HEIGHT - PLAYER_HEIGHT - 1;
    model->player.dx = 0;
    model->player.dy = 0;
    // --- INIT VOLUME ---
    model->volume = 30; // 30% au début
    model->is_muted = false;
    // Init sons
    model->sounds.play_shoot = false;
    model->sounds.play_invader_killed = false;
    model->sounds.play_player_explosion = false;
    model->sounds.play_beat = false;
    model->sounds.beat_index = 0;
    model->sounds.ufo_loopING = false;

    for (int i = 0; i < MAX_BULLETS; i++)
        model->bullets[i].active = false;

    model->animation_frame = 0;
    model->animation_timer = 0;

    init_enemies(model);
    // --- SETUP BOUCLIERS ---
    float shield_w = 8.0f; // Largeur arbitraire (adapte selon ton image)
    float shield_h = 6.0f;

    // Espace total disponible divisé par (nombre de boucliers + 1) pour centrer
    float spacing = GAME_WIDTH / (MAX_SHIELDS + 1);

    for (int i = 0; i < MAX_SHIELDS; i++)
    {
        model->shields[i].active = true;
        model->shields[i].health = SHIELD_MAX_HEALTH; // 10
        model->shields[i].width = shield_w;
        model->shields[i].height = shield_h;

        // Position X : 1er tiers, 2eme tiers, etc. - la moitié de la largeur pour centrer
        model->shields[i].x = (spacing * (i + 1)) - (shield_w / 2);

        // Position Y : Juste au-dessus du joueur (ajuste le -15 selon tes besoins)
        model->shields[i].y = GAME_HEIGHT - PLAYER_HEIGHT - 9;
    }

    return model;
}
static void reset_game(GameModel *model)
{
    // 1. Reset Stats
    model->score = 0;
    model->lives = 3;
    model->level = 1;
    model->hit_timer = 0;

    // 2. Reset Difficulté
    model->enemy_speed_mult = 1.0f;
    model->direction_enemies = 1;
    model->drop_direction = 1;
    model->drop_step_count = 0;

    // 3. Nettoyer les balles
    for (int k = 0; k < MAX_BULLETS; k++)
    {
        model->bullets[k].active = false;
    }

    // 4. Reset UFO
    model->ufo.active = false;
    model->ufo.hasSpawnedThisLevel = false;

    // 5. Replacer le joueur
    model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
    model->player.dx = 0;

    // 6. RECRÉER ENNEMIS ET BOUCLIERS
    // Cette fonction doit contenir la boucle des boucliers !
    init_enemies(model);

    // --- SÉCURITÉ : RESET BOUCLIERS EXPLICITE ---
    // Si jamais init_enemies ne le fait pas, on le force ici :
    float shield_w = 6.0f;
    float shield_h = 4.0f;
    float spacing = GAME_WIDTH / (MAX_SHIELDS + 1);

    for (int i = 0; i < MAX_SHIELDS; i++)
    {
        model->shields[i].active = true; // ON RÉACTIVE !
        model->shields[i].health = 10;   // ON RÉPARE !
        model->shields[i].width = shield_w;
        model->shields[i].height = shield_h;
        model->shields[i].x = (spacing * (i + 1)) - (shield_w / 2);
        model->shields[i].y = GAME_HEIGHT - PLAYER_HEIGHT - 6;
    }
    // ---------------------------------------------

    // 7. Relancer le jeu
    model->state = STATE_PLAYING;
}
void model_free(GameModel *model)
{
    if (model)
        free(model);
}

// src/model.c

// src/model.c -> update_bullets

static void update_bullets(GameModel *model, double dt)
{
    // A. Mise à jour du Timer de tir (Cooldown)
    if (model->player.shoot_timer > 0.0f)
    {
        model->player.shoot_timer -= dt;
    }

    // B. Boucle sur les balles
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (model->bullets[i].active)
        {
            // 1. Mouvement
            model->bullets[i].x += model->bullets[i].dx * dt * 60.0f;
            model->bullets[i].y += model->bullets[i].dy * dt * 60.0f;

            // 2. Sortie d'écran (Haut ou Bas)
            if (model->bullets[i].y < 0 || model->bullets[i].y > GAME_HEIGHT)
            {
                model->bullets[i].active = false;
                continue;
            }

            // 3. Collision : BALLE JOUEUR vs ENNEMIS
            if (model->bullets[i].type == ENTITY_BULLET_PLAYER)
            {
                for (int j = 0; j < MAX_ENEMIES; j++)
                {
                    if (model->enemies[j].active)
                    {
                        if (check_collision(&model->bullets[i], &model->enemies[j]))
                        {
                            model->enemies[j].active = false; // Ennemi meurt
                            model->bullets[i].active = false; // Balle meurt (1 coup = 1 mort)

                            // GESTION DES POINTS SELON LE TYPE
                            if (model->enemies[j].type == ENTITY_ENEMY_TYPE_1)
                                model->score += 10; // Vert
                            else if (model->enemies[j].type == ENTITY_ENEMY_TYPE_2)
                                model->score += 20; // Cyan
                            else if (model->enemies[j].type == ENTITY_ENEMY_TYPE_3)
                                model->score += 30; // Magenta

                            break; // IMPORTANT : On sort de la boucle ennemis pour ne pas en tuer 2 d'un coup
                        }
                    }
                }
            }
            // 4. Collision : BALLE ENNEMIE vs JOUEUR
            else if (model->bullets[i].type == ENTITY_BULLET_ENEMY)
            {
                if (model->player.active && check_collision(&model->bullets[i], &model->player))
                {
                    if (model->hit_timer <= 0)
                    { // Si pas déjà invulnérable
                        model->lives--;
                        model->bullets[i].active = false;
                        model->hit_timer = 2.0f; // 2 sec d'invincibilité
                        if (model->lives <= 0)
                            model->state = STATE_GAME_OVER;
                    }
                }
            }
        }
    }
}
// src/model.c

// src/model.c

void model_handle_input(GameModel *model, GameCommand cmd)
{
    // =================================================
    // 1. MENU PRINCIPAL
    // =================================================
    if (model->state == STATE_MENU)
    {
        if (cmd == CMD_PAUSE || cmd == CMD_EXIT)
            exit(0);

        // Navigation
        if (cmd == CMD_UP)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = 4;
        }
        else if (cmd == CMD_DOWN)
        {
            model->menu_selection++;
            if (model->menu_selection > 4)
                model->menu_selection = 0;
        }

        // Gestion Volume (Option 3)
        if (model->menu_selection == 3)
        {
            if (cmd == CMD_LEFT || cmd == CMD_MOVE_LEFT)
            {
                model->volume -= 10;
                if (model->volume < 0)
                    model->volume = 0;
                model->is_muted = false;
            }
            else if (cmd == CMD_RIGHT || cmd == CMD_MOVE_RIGHT)
            {
                model->volume += 10;
                if (model->volume > 100)
                    model->volume = 100;
                model->is_muted = false;
            }
            else if (cmd == CMD_SHOOT || cmd == CMD_RETURN)
            {
                model->is_muted = !model->is_muted;
            }
        }
        // Validation autres options
        else
        {
            if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
            {
                model->sounds.play_select_sound = true; // SON OK

                if (model->menu_selection == 0)
                { // JOUER
                    init_enemies(model);
                    model->score = 0;
                    model->lives = 3;
                    model->level = 1;
                    model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
                    model->state = STATE_PLAYING;
                }
                else if (model->menu_selection == 1)
                { // TUTORIEL
                    model->state = STATE_TUTORIAL;
                }
                else if (model->menu_selection == 2)
                { // CHARGER
                    model_scan_saves(model);
                    if (model->save_file_count > 0)
                    {
                        model->state = STATE_LOAD_MENU;
                        model->menu_selection = 0;
                    }
                    else
                    {
                        printf("Aucune sauvegarde trouvée !\n");
                    }
                }
                else if (model->menu_selection == 4)
                { // QUITTER
                    exit(0);
                }
            }
        }
        return;
    }

    // =================================================
    // 2. MENU CHARGEMENT (LOAD)
    // =================================================
    if (model->state == STATE_LOAD_MENU)
    {
        if (cmd == CMD_PAUSE || cmd == CMD_EXIT)
        {
            model->state = STATE_MENU;
            model->menu_selection = 1;
            return;
        }

        if (cmd == CMD_UP || cmd == CMD_LEFT || cmd == CMD_MOVE_LEFT)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = model->save_file_count - 1;
        }
        else if (cmd == CMD_DOWN || cmd == CMD_RIGHT || cmd == CMD_MOVE_RIGHT)
        {
            model->menu_selection++;
            if (model->menu_selection >= model->save_file_count)
                model->menu_selection = 0;
        }
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            if (model->save_file_count > 0)
            {
                model->sounds.play_select_sound = true; // SON OK
                char *file_to_load = model->save_files[model->menu_selection];
                if (model_load_named(model, file_to_load))
                {
                    // Le jeu reprend automatiquement
                }
            }
        }
        return;
    }

    // =================================================
    // 3. TUTORIEL
    // =================================================
    if (model->state == STATE_TUTORIAL)
    {
        // On appuie sur n'importe quoi pour revenir au menu
        if (cmd == CMD_PAUSE || cmd == CMD_EXIT || cmd == CMD_SHOOT || cmd == CMD_RETURN)
        {
            model->sounds.play_select_sound = true; // SON AJOUTÉ ICI
            model->state = STATE_MENU;
            model->menu_selection = 1;
        }
        return;
    }

    // =================================================
    // 4. GAME OVER
    // =================================================
    if (model->state == STATE_GAME_OVER)
    {
        if (model->game_over_timer < 1.0f)
            return;

        if (cmd == CMD_EXIT) // Touche raccourci
        {
            model->previous_state = STATE_GAME_OVER; // <--- AJOUT IMPORTANT
            model->state = STATE_CONFIRM_QUIT;
            model->menu_selection = 1;
            return;
        }

        if (cmd == CMD_UP || cmd == CMD_LEFT || cmd == CMD_MOVE_LEFT)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = 2;
        }
        else if (cmd == CMD_DOWN || cmd == CMD_RIGHT || cmd == CMD_MOVE_RIGHT)
        {
            model->menu_selection++;
            if (model->menu_selection > 2)
                model->menu_selection = 0;
        }
        else if (cmd == CMD_SHOOT || cmd == CMD_RETURN)
        {
            model->sounds.play_select_sound = true; // SON OK

            if (model->menu_selection == 0)
            { // SAUVEGARDER
                model_scan_saves(model);
                model->state = STATE_SAVE_SELECT;
                model->menu_selection = 0;
            }
            else if (model->menu_selection == 1)
            { // REJOUER
                reset_game(model);
            }
            else
            {                                            // Option QUITTER du menu
                model->previous_state = STATE_GAME_OVER; // <--- AJOUT IMPORTANT
                model->state = STATE_CONFIRM_QUIT;
                model->menu_selection = 1;
            }
        }
        return;
    }

    // =================================================
    // 5. MENU PAUSE
    // =================================================
    if (model->state == STATE_PAUSED)
    {
        if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PLAYING;
            return;
        }

        // Navigation
        if (cmd == CMD_UP)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = 3;
        }
        else if (cmd == CMD_DOWN)
        {
            model->menu_selection++;
            if (model->menu_selection > 3)
                model->menu_selection = 0;
        }

        // Option Volume
        if (model->menu_selection == 1)
        {
            if (cmd == CMD_LEFT || cmd == CMD_MOVE_LEFT)
            {
                model->volume -= 10;
                if (model->volume < 0)
                    model->volume = 0;
                model->is_muted = false;
            }
            else if (cmd == CMD_RIGHT || cmd == CMD_MOVE_RIGHT)
            {
                model->volume += 10;
                if (model->volume > 100)
                    model->volume = 100;
                model->is_muted = false;
            }
            else if (cmd == CMD_SHOOT || cmd == CMD_RETURN)
            {
                model->is_muted = !model->is_muted;
            }
        }
        // Validation Autres Options
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            // Son OK sauf sur Volume (géré plus haut)
            if (model->menu_selection != 1)
            {
                model->sounds.play_select_sound = true; // SON OK
            }

            if (model->menu_selection == 0)
            { // REPRENDRE
                model->state = STATE_PLAYING;
            }
            else if (model->menu_selection == 2)
            { // SAUVEGARDER ET QUITTER
                model_scan_saves(model);
                model->state = STATE_SAVE_SELECT;
                model->menu_selection = 0;
            }
            else if (model->menu_selection == 3)
            {                                         // QUITTER SANS SAUVER
                model->previous_state = STATE_PAUSED; // <--- AJOUT IMPORTANT
                model->state = STATE_CONFIRM_QUIT;
                model->menu_selection = 1;
            }
        }
        return;
    }

    // =================================================
    // 6. SÉLECTION DU SLOT DE SAUVEGARDE
    // =================================================
    if (model->state == STATE_SAVE_SELECT)
    {
        if (cmd == CMD_PAUSE || cmd == CMD_EXIT)
        {
            model->state = STATE_PAUSED;
            return;
        }

        int total_options = model->save_file_count + 1;
        if (cmd == CMD_UP || cmd == CMD_LEFT || cmd == CMD_MOVE_LEFT)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = total_options - 1;
        }
        else if (cmd == CMD_DOWN || cmd == CMD_RIGHT || cmd == CMD_MOVE_RIGHT)
        {
            model->menu_selection++;
            if (model->menu_selection >= total_options)
                model->menu_selection = 0;
        }
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true; // SON OK

            if (model->menu_selection == 0)
            { // CREER NOUVELLE
                model->state = STATE_SAVE_INPUT;
                model->input_buffer[0] = '\0';
            }
            else
            { // ECRASER EXISTANTE
                int file_index = model->menu_selection - 1;
                char *selected_file = model->save_files[file_index];
                size_t len = strlen(selected_file);

                if (len > 4)
                    strncpy(model->input_buffer, selected_file, len - 4);
                else
                    strcpy(model->input_buffer, selected_file);
                model->input_buffer[len - 4] = '\0';

                model->state = STATE_OVERWRITE_CONFIRM;
                model->menu_selection = 0;
            }
        }
        return;
    }

    // =================================================
    // 7. SAISIE NOM SAUVEGARDE
    // =================================================
    if (model->state == STATE_SAVE_INPUT)
    {
        if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PAUSED;
            model->pending_quit = false;
            return;
        }
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            if (strlen(model->input_buffer) > 0)
            {
                model->sounds.play_select_sound = true; // SON OK

                char filename[64];
                snprintf(filename, sizeof(filename), "%s.dat", model->input_buffer);

                if (save_file_exists(filename))
                {
                    model->state = STATE_OVERWRITE_CONFIRM;
                    model->menu_selection = 1;
                }
                else
                {
                    model_save_named(model, model->input_buffer);
                    model->state = STATE_SAVE_SUCCESS;
                    model->save_success_timer = 2.0f;
                }
            }
        }
        else if (cmd == CMD_BACKSPACE)
        {
            int len = strlen(model->input_buffer);
            if (len > 0)
                model->input_buffer[len - 1] = '\0';
        }
        return;
    }

    // =================================================
    // 8. CONFIRMATION QUITTER
    // =================================================
    if (model->state == STATE_CONFIRM_QUIT)
    {
        if (cmd == CMD_UP || cmd == CMD_LEFT || cmd == CMD_MOVE_LEFT)
        {
            model->menu_selection--;
            if (model->menu_selection < 0)
                model->menu_selection = 2;
        }
        else if (cmd == CMD_DOWN || cmd == CMD_RIGHT || cmd == CMD_MOVE_RIGHT)
        {
            model->menu_selection++;
            if (model->menu_selection > 2)
                model->menu_selection = 0;
        }
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true; // SON OK

            if (model->menu_selection == 0)
                exit(0);
            else if (model->menu_selection == 1)
            {
                model->state = STATE_PAUSED;
                model->menu_selection = 3;
            }
            else if (model->menu_selection == 2)
            {
                model_scan_saves(model);
                model->state = STATE_SAVE_SELECT;
                model->menu_selection = 0;
            }
        }
        else if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PAUSED;
        }
        return;
    }

    // =================================================
    // 9. CONFIRMATION ÉCRASEMENT
    // =================================================
    if (model->state == STATE_OVERWRITE_CONFIRM)
    {
        if (cmd == CMD_LEFT || cmd == CMD_RIGHT || cmd == CMD_UP || cmd == CMD_DOWN)
        {
            model->menu_selection = !model->menu_selection;
        }
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true; // SON AJOUTÉ ICI

            if (model->menu_selection == 0)
            { // OUI
                model_save_named(model, model->input_buffer);
                model->state = STATE_SAVE_SUCCESS;
                model->save_success_timer = 2.0f;
            }
            else
            { // NON
                model->state = STATE_SAVE_INPUT;
            }
        }
        return;
    }

    // =================================================
    // 10. JEU (EN COURS)
    // =================================================
    if (model->state == STATE_PLAYING)
    {
        if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PAUSED;
            model->menu_selection = 0;
            return;
        }

        switch (cmd)
        {
        case CMD_MOVE_LEFT:
        case CMD_LEFT:
            model->player.dx = -PLAYER_SPEED;
            break;
        case CMD_MOVE_RIGHT:
        case CMD_RIGHT:
            model->player.dx = PLAYER_SPEED;
            break;
        case CMD_SHOOT:
            if (model->player.shoot_timer <= 0.0f)
            {
                spawn_bullet(model,
                             model->player.x + model->player.width / 2.0f - BULLET_WIDTH / 2.0f,
                             model->player.y - 1,
                             -BULLET_SPEED,
                             ENTITY_BULLET_PLAYER);
                model->player.shoot_timer = 0.5f;
                model->sounds.play_shoot = true; // SON TIR OK
            }
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
    // 1. Etats speciaux
    if (model->state == STATE_SAVE_SUCCESS)
    {
        model->save_success_timer -= dt;
        if (model->save_success_timer <= 0)
            exit(0);
        return;
    }
    if (model->state == STATE_GAME_OVER)
    {
        model->game_over_timer += dt;
        return;
    }
    if (model->state != STATE_PLAYING)
        return;

    // 2. Timers
    if (model->player.shoot_timer > 0.0f)
        model->player.shoot_timer -= dt;
    if (model->hit_timer > 0)
        model->hit_timer -= dt;

    // 3. Animation Rythme
    float beat_delay = 0.5f - (model->level * 0.05f);
    if (beat_delay < 0.05f)
        beat_delay = 0.05f;
    model->animation_timer += dt;
    if (model->animation_timer >= beat_delay)
    {
        model->animation_frame = !model->animation_frame;
        model->animation_timer = 0;
        model->sounds.play_beat = true;
        model->sounds.beat_index = (model->sounds.beat_index + 1) % 4;
    }

    // 4. Joueur
    if (model->player.active)
    {
        model->player.x += model->player.dx * dt;
        if (model->player.x < 0)
            model->player.x = 0;
        if (model->player.x > GAME_WIDTH - model->player.width)
            model->player.x = GAME_WIDTH - model->player.width;
    }

    // 5. UFO (Audio + Physique)
    if (model->ufo.active && !model->ufo.exploding)
        model->sounds.ufo_loopING = true;
    else
        model->sounds.ufo_loopING = false;

    if (model->ufo.active)
    {
        if (model->ufo.exploding)
        {
            model->ufo.explode_timer -= dt;
            if (model->ufo.explode_timer <= 0)
                model->ufo.active = false;
        }
        else
        {
            model->ufo.x += model->ufo.dx * dt;
            if ((model->ufo.dx > 0 && model->ufo.x > GAME_WIDTH) ||
                (model->ufo.dx < 0 && model->ufo.x + model->ufo.width < 0))
            {
                model->ufo.active = false;
            }
        }
    }
    else
    {
        // Spawn simple logic
        bool enemies_alive = false;
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (model->enemies[i].active)
                enemies_alive = true;
        if (!model->ufo.hasSpawnedThisLevel && enemies_alive && rand() % 150 == 0)
            spawn_ufo(model);
    }

    // 6. Ennemis et Level Up
    int active_enemies = 0;
    bool touch_edge = false;
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!model->enemies[i].active)
            continue;
        if (model->enemies[i].exploding)
        {
            model->enemies[i].explode_timer -= dt;
            if (model->enemies[i].explode_timer <= 0)
                model->enemies[i].active = false;
            continue;
        }
        active_enemies++;
        if ((model->enemies[i].x <= 0 && model->direction_enemies == -1) ||
            (model->enemies[i].x >= GAME_WIDTH - ENEMY_WIDTH && model->direction_enemies == 1))
            touch_edge = true;
    }

    if (active_enemies == 0)
    {
        model->level++;
        model->sounds.play_level_up = true;
        init_enemies(model);
        model->drop_direction = 1;
        model->drop_step_count = 0;
        model->direction_enemies = 1;
        model->ufo.hasSpawnedThisLevel = false;
        model->enemy_speed_mult = 1.0f + ((model->level - 1) * 0.2f);
        return;
    }

    // Mouvements Ennemis
    if (touch_edge)
    {
        model->direction_enemies *= -1;
        float move_y = (model->drop_direction == 1) ? ENEMY_DROP_HEIGHT : -ENEMY_DROP_HEIGHT;
        if (model->drop_direction == 1)
        {
            if (++model->drop_step_count >= 3)
                model->drop_direction = -1;
        }
        else
        {
            if (--model->drop_step_count <= 0)
                model->drop_direction = 1;
        }
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (model->enemies[i].active && !model->enemies[i].exploding)
            {
                model->enemies[i].y += move_y;
                model->enemies[i].x += model->direction_enemies * 2.0f;
            }
    }
    else
    {
        float speed = ENEMY_SPEED_BASE * model->enemy_speed_mult * model->direction_enemies;
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (model->enemies[i].active && !model->enemies[i].exploding)
                model->enemies[i].x += speed * dt;
    }

    // Tirs Ennemis
    if (utils_random_int(0, 100) < model->level + 1)
    {
        for (int k = 0; k < 10; k++)
        {
            int idx = utils_random_int(0, MAX_ENEMIES - 1);
            if (model->enemies[idx].active && !model->enemies[idx].exploding)
            {
                spawn_bullet(model, model->enemies[idx].x + ENEMY_WIDTH / 2, model->enemies[idx].y + ENEMY_HEIGHT, BULLET_SPEED * 0.6f, ENTITY_BULLET_ENEMY);
                break;
            }
        }
    }

    // 7. Balles & Collisions (Sans fonction externe pour éviter l'erreur)
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        Entity *b = &model->bullets[i];
        if (!b->active)
            continue;
        b->y += b->dy * dt;
        b->anim_timer += dt;
        if (b->anim_timer >= 0.05f)
        {
            b->anim_timer = 0;
            b->anim_frame = (b->anim_frame + 1) % 4;
        }
        if (b->y < 0 || b->y > GAME_HEIGHT)
        {
            b->active = false;
            continue;
        }

        // Boucliers
        bool hit_shield = false;
        for (int s = 0; s < MAX_SHIELDS; s++)
        {
            Shield *sh = &model->shields[s];
            if (sh->active)
            {
                // Collision Simple AABB directe (plus d'erreur implicit declaration)
                if (b->x < sh->x + sh->width && b->x + b->width > sh->x &&
                    b->y < sh->y + sh->height && b->y + b->height > sh->y)
                {
                    b->active = false;
                    hit_shield = true;
                    sh->health--;
                    if (sh->health <= 0)
                        sh->active = false;
                    break;
                }
            }
        }
        if (hit_shield)
            continue;

        // Joueur vs ...
        if (b->type == ENTITY_BULLET_PLAYER)
        {
            // Vs UFO
            if (model->ufo.active && !model->ufo.exploding)
            {
                if (check_collision(b, (Entity *)&model->ufo))
                { // Cast safe ici
                    b->active = false;
                    model->ufo.exploding = true;
                    model->sounds.play_invader_killed = true;
                    model->ufo.explode_timer = 0.2f;
                    model->score += 60;
                    model->lives++;
                    continue;
                }
            }
            // Vs Ennemis
            for (int e = 0; e < MAX_ENEMIES; e++)
            {
                if (model->enemies[e].active && !model->enemies[e].exploding && check_collision(b, &model->enemies[e]))
                {
                    b->active = false;
                    model->enemies[e].exploding = true;
                    model->sounds.play_invader_killed = true;
                    model->enemies[e].explode_timer = 0.2f;
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
        else
        { // Balle ennemie
            if (model->player.active && check_collision(b, &model->player))
            {
                b->active = false;
                if (model->hit_timer <= 0)
                {
                    model->sounds.play_player_explosion = true;
                    model->lives--;
                    if (model->lives <= 0)
                    {
                        model->state = STATE_GAME_OVER;
                        model->sounds.play_game_over = true;
                        model->menu_selection = 0;
                        model->game_over_timer = 0;
                    }
                    else
                        model->hit_timer = 2.0f;
                }
            }
        }
    }
}
// 1. Fonction pour lister les sauvegardes
void model_scan_saves(GameModel *model)
{
    model->save_file_count = 0;
    DIR *d;
    struct dirent *dir;

    // On essaie d'ouvrir le dossier
    d = opendir("sauvegardes");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            // On ignore "." et ".." et on cherche les fichiers .dat
            if (strstr(dir->d_name, ".dat"))
            {
                if (model->save_file_count < MAX_SAVE_FILES)
                {
                    // On copie le nom du fichier dans notre liste
                    snprintf(model->save_files[model->save_file_count], 256, "%s", dir->d_name);
                    model->save_file_count++;
                }
            }
        }
        closedir(d);
    }
}

// 2. Sauvegarde avec un nom personnalisé
void model_save_named(const GameModel *model, const char *filename)
{
// On recrée le dossier au cas où
#ifdef _WIN32
    _mkdir("sauvegardes");
#else
    struct stat st = {0};
    if (stat("sauvegardes", &st) == -1)
        mkdir("sauvegardes", 0777);
#endif

    char path[128];
    // On construit le chemin : "sauvegardes/MonNom.dat"
    snprintf(path, sizeof(path), "sauvegardes/%s", filename);

    FILE *f = fopen(path, "wb");
    if (f)
    {
        fwrite(model, sizeof(GameModel), 1, f);
        fclose(f);
        printf("Sauvegarde reussie : %s\n", path);
    }
}

// 3. Chargement avec nom
bool model_load_named(GameModel *model, const char *filename)
{
    char path[128];
    snprintf(path, sizeof(path), "sauvegardes/%s", filename);

    FILE *f = fopen(path, "rb");
    if (!f)
        return false;

    GameModel temp;
    size_t res = fread(&temp, sizeof(GameModel), 1, f);
    fclose(f);

    if (res == 1)
    {
        *model = temp; // On récupère toutes les données (score, vies, positions...)

        // --- C'EST ICI QUE TU CHANGES ---
        // AVANT : model->state = STATE_PAUSED;
        // MAINTENANT :
        model->state = STATE_PLAYING;
        // --------------------------------

        // Petit reset visuel pour éviter que le vaisseau clignote rouge bizarrement
        model->hit_timer = 0;

        return true;
    }
    return false;
}
