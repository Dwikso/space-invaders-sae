#include "model.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Gestion des dossiers (Spécifique Linux/POSIX)
#include <sys/stat.h>
#include <dirent.h>

// ============================================================================
//                          1. FONCTIONS INTERNES (HELPERS)
// ============================================================================

// Vérifie la collision entre deux entités (AABB)
static bool check_collision(const Entity *a, const Entity *b)
{
    if (!a->active || !b->active)
        return false;

    return (a->x < b->x + b->width &&
            a->x + a->width > b->x &&
            a->y < b->y + b->height &&
            a->y + a->height > b->y);
}

// Trouve une place vide dans le tableau de balles
static void spawn_bullet(GameModel *model, float x, float y, float dy, EntityType type)
{
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (!model->bullets[i].active)
        {
            // On initialise la balle
            model->bullets[i].active = true;
            model->bullets[i].x = x;
            model->bullets[i].y = y;
            model->bullets[i].dx = 0;
            model->bullets[i].dy = dy;
            model->bullets[i].width = BULLET_WIDTH;
            model->bullets[i].height = BULLET_HEIGHT;
            model->bullets[i].type = type;
            model->bullets[i].anim_timer = 0;
            model->bullets[i].anim_frame = 0;
            return; // On a trouvé une place, on arrête
        }
    }
}

// Vérifie si un fichier existe déjà
static bool save_file_exists(const char *filename)
{
    char path[128];
    snprintf(path, sizeof(path), "sauvegardes/%s", filename);

    FILE *f = fopen(path, "rb");
    if (f)
    {
        fclose(f);
        return true;
    }
    return false;
}

// ============================================================================
//                          2. LOGIQUE ENNEMIS & UFO
// ============================================================================

static void init_enemies(GameModel *model)
{
    int idx = 0;

    // 5 rangées de 11 colonnes
    for (int row = 0; row < 5; row++)
    {
        for (int col = 0; col < 11; col++)
        {
            if (idx >= MAX_ENEMIES)
                break;

            Entity *e = &model->enemies[idx];

            // 1. NETTOYAGE COMPLET : Met tout à 0 (x, y, timer, active, exploding...)
            // Cela remplace toutes les lignes manuelles "e->exploding = false" etc.
            memset(e, 0, sizeof(Entity));

            // 2. CONFIGURATION
            e->active = true;
            e->width = ENEMY_WIDTH;
            e->height = ENEMY_HEIGHT;

            // Calcul position : MargeGauche + Col * (Taille + Espace)
            e->x = 5 + col * (ENEMY_WIDTH + 2);
            e->y = 7 + row * (ENEMY_HEIGHT + 2);

            // 3. TYPES SELON LA RANGÉE
            if (row == 0)
                e->type = ENTITY_ENEMY_TYPE_3; // Haut
            else if (row < 3)
                e->type = ENTITY_ENEMY_TYPE_2; // Milieu
            else
                e->type = ENTITY_ENEMY_TYPE_1; // Bas

            idx++;
        }
    }

    // Paramètres globaux de la vague
    model->enemy_speed_mult = 1.0f;
    model->direction_enemies = 1; // 1 = Droite, -1 = Gauche
    model->drop_direction = 1;    // Yo-Yo
    model->drop_step_count = 0;

    // Reset UFO pour être sûr qu'il n'est pas là au début du niveau
    model->ufo.active = false;
    model->ufo.hasSpawnedThisLevel = false;
    model->ufo.y = 4; // Position fixe en haut
}

static void spawn_ufo(GameModel *model)
{
    // On nettoie l'entité UFO avant de l'activer
    // Ça évite qu'il garde l'état "explosé" de la fois précédente
    model->ufo.exploding = false;
    model->ufo.explode_timer = 0.0f;
    model->ufo.active = true;
    model->ufo.hasSpawnedThisLevel = true;
    model->ufo.type = ENTITY_UFO;
    model->ufo.width = UFO_WIDTH;
    model->ufo.height = UFO_HEIGHT;
    model->ufo.y = 4;
    // Pile ou Face pour la direction
    if (rand() % 2 == 0)
    {
        model->ufo.x = -UFO_WIDTH; // Part de gauche
        model->ufo.dx = UFO_SPEED; // Va à droite
    }
    else
    {
        model->ufo.x = GAME_WIDTH;  // Part de droite
        model->ufo.dx = -UFO_SPEED; // Va à gauche
    }
}
// ============================================================================
//                          3. INITIALISATION & RESET
// ============================================================================

// Helper : Initialise les boucliers (Factorisé pour ne pas copier-coller)
static void init_shields(GameModel *model)
{
    float shield_w = 8.0f;
    float shield_h = 6.0f;
    // Espacement dynamique pour centrer
    float spacing = GAME_WIDTH / (MAX_SHIELDS + 1.0f);

    for (int i = 0; i < MAX_SHIELDS; i++)
    {
        model->shields[i].active = true;
        model->shields[i].health = SHIELD_MAX_HEALTH; // 10
        model->shields[i].width = shield_w;
        model->shields[i].height = shield_h;
        model->shields[i].x = (spacing * (i + 1)) - (shield_w / 2.0f);
        model->shields[i].y = GAME_HEIGHT - PLAYER_HEIGHT - 9;
    }
}

GameModel *model_init(void)
{
    // 1. Allocation + Mise à zéro automatique (calloc)
    // Plus besoin d'initialiser is_muted, dx, dy, timers... tout est à 0 !
    GameModel *model = calloc(1, sizeof(GameModel));
    if (!model)
        return NULL;

    // 2. Création du dossier sauvegarde (Linux)
    mkdir("sauvegardes", 0777);

    // 3. Valeurs par défaut (Seulement ce qui n'est pas 0)
    model->state = STATE_MENU;
    model->lives = 3;
    model->normal_max_lives = MAX_LIVES_NORMAL;
    model->level = 1;
    model->volume = 30; // 30% volume

    // 4. Initialisation Joueur
    model->player.active = true;
    model->player.type = ENTITY_PLAYER;
    model->player.width = PLAYER_WIDTH;
    model->player.height = PLAYER_HEIGHT;
    model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
    model->player.y = GAME_HEIGHT - PLAYER_HEIGHT - 1;

    // 5. Initialisation du Monde
    init_enemies(model);
    init_shields(model); // On utilise la fonction helper

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

    // 3. Nettoyage des balles (memset = instantané)
    // Remplace ta boucle for qui parcourait tout le tableau
    memset(model->bullets, 0, sizeof(model->bullets));

    // 4. Reset Entités
    model->ufo.active = false;
    model->ufo.hasSpawnedThisLevel = false;

    // Replacer le joueur au centre
    model->player.x = (GAME_WIDTH - PLAYER_WIDTH) / 2.0f;
    model->player.dx = 0;

    // 5. Recréation du niveau
    init_enemies(model);
    init_shields(model); // Plus de duplication de code ici !

    // 6. Lancement
    model->state = STATE_PLAYING;
}

void model_free(GameModel *model)
{
    if (model)
        free(model);
}

// Getter (Lecture seule)
const Entity *model_get_player(const GameModel *model)
{
    return &model->player;
}

// ============================================================================
//                          5. GESTION DES ENTRÉES (CONTROLLER -> MODEL)
// ============================================================================

void model_handle_input(GameModel *model, GameCommand cmd)
{
    // ---------------------------------------------------------
    // 1. MENU PRINCIPAL
    // ---------------------------------------------------------
    if (model->state == STATE_MENU)
    {
        if (cmd == CMD_EXIT)
            exit(0);

        // Navigation (0 à 4)
        if (cmd == CMD_UP)
            model->menu_selection = (model->menu_selection - 1 < 0) ? 4 : model->menu_selection - 1;
        if (cmd == CMD_DOWN)
            model->menu_selection = (model->menu_selection + 1 > 4) ? 0 : model->menu_selection + 1;

        // Gestion Volume (Option 3)
        if (model->menu_selection == 3)
        {
            if (cmd == CMD_LEFT || cmd == CMD_MOVE_LEFT)
            {
                model->volume = (model->volume - 10 < 0) ? 0 : model->volume - 10;
                model->is_muted = false;
            }
            if (cmd == CMD_RIGHT || cmd == CMD_MOVE_RIGHT)
            {
                model->volume = (model->volume + 10 > 100) ? 100 : model->volume + 10;
                model->is_muted = false;
            }
            if (cmd == CMD_SHOOT || cmd == CMD_RETURN)
                model->is_muted = !model->is_muted;
        }
        // Validation
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true;

            if (model->menu_selection == 0)
                reset_game(model); // JOUER (Utilise reset_game !)
            else if (model->menu_selection == 1)
                model->state = STATE_TUTORIAL;
            else if (model->menu_selection == 2)
            { // CHARGER
                model_scan_saves(model);
                if (model->save_file_count > 0)
                {
                    model->state = STATE_LOAD_MENU;
                    model->menu_selection = 0;
                }
            }
            else if (model->menu_selection == 4)
                exit(0);
        }
        return;
    }

    // ---------------------------------------------------------
    // 2. MENU CHARGEMENT
    // ---------------------------------------------------------
    if (model->state == STATE_LOAD_MENU)
    {
        if (cmd == CMD_EXIT || cmd == CMD_PAUSE)
        {
            model->state = STATE_MENU;
            model->menu_selection = 2; // Retour sur bouton Charger
            return;
        }

        if (cmd == CMD_UP)
            model->menu_selection = (model->menu_selection - 1 < 0) ? model->save_file_count - 1 : model->menu_selection - 1;
        if (cmd == CMD_DOWN)
            model->menu_selection = (model->menu_selection + 1 >= model->save_file_count) ? 0 : model->menu_selection + 1;

        if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            if (model->save_file_count > 0)
            {
                model->sounds.play_select_sound = true;
                model_load_named(model, model->save_files[model->menu_selection]);
            }
        }
        return;
    }

    // ---------------------------------------------------------
    // 3. TUTORIEL
    // ---------------------------------------------------------
    if (model->state == STATE_TUTORIAL)
    {
        if (cmd == CMD_EXIT || cmd == CMD_PAUSE || cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true;
            model->state = STATE_MENU;
            model->menu_selection = 1; // Retour sur bouton Tuto
        }
        return;
    }

    // ---------------------------------------------------------
    // 4. JEU (PLAYING)
    // ---------------------------------------------------------
    if (model->state == STATE_PLAYING)
    {
        if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PAUSED;
            model->menu_selection = 0;
            return;
        }

        if (cmd == CMD_MOVE_LEFT || cmd == CMD_LEFT)
            model->player.dx = -PLAYER_SPEED;
        else if (cmd == CMD_MOVE_RIGHT || cmd == CMD_RIGHT)
            model->player.dx = PLAYER_SPEED;
        else if (cmd == CMD_NONE)
            model->player.dx = 0;

        if (cmd == CMD_SHOOT && model->player.shoot_timer <= 0.0f)
        {
            // Tir centré par rapport au joueur
            spawn_bullet(model,
                         model->player.x + 1.5f,
                         model->player.y - 1,
                         -BULLET_SPEED,
                         ENTITY_BULLET_PLAYER);
            model->player.shoot_timer = 0.5f;
            model->sounds.play_shoot = true;
        }
        return;
    }

    // ---------------------------------------------------------
    // 5. PAUSE
    // ---------------------------------------------------------
    if (model->state == STATE_PAUSED)
    {
        if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PLAYING;
            return;
        }

        if (cmd == CMD_UP)
            model->menu_selection = (model->menu_selection - 1 < 0) ? 3 : model->menu_selection - 1;
        if (cmd == CMD_DOWN)
            model->menu_selection = (model->menu_selection + 1 > 3) ? 0 : model->menu_selection + 1;

        // Volume (Index 1)
        if (model->menu_selection == 1)
        {
            if (cmd == CMD_LEFT)
            {
                model->volume = (model->volume < 10) ? 0 : model->volume - 10;
                model->is_muted = false;
            }
            if (cmd == CMD_RIGHT)
            {
                model->volume = (model->volume > 90) ? 100 : model->volume + 10;
                model->is_muted = false;
            }
            if (cmd == CMD_SHOOT || cmd == CMD_RETURN)
                model->is_muted = !model->is_muted;
        }
        else if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true;
            if (model->menu_selection == 0)
                model->state = STATE_PLAYING; // Reprendre
            else if (model->menu_selection == 2)
            { // Sauvegarder
                model_scan_saves(model);
                model->state = STATE_SAVE_SELECT;
                model->menu_selection = 0;
            }
            else if (model->menu_selection == 3)
            { // Quitter sans sauver
                model->previous_state = STATE_PAUSED;
                model->state = STATE_CONFIRM_QUIT;
                model->menu_selection = 1; // "Non" par défaut
            }
        }
        return;
    }

    // ---------------------------------------------------------
    // 6. GAME OVER
    // ---------------------------------------------------------
    if (model->state == STATE_GAME_OVER)
    {
        if (model->game_over_timer > 0.5f)
            return; // Petit délai de sécurité

        if (cmd == CMD_UP)
            model->menu_selection = (model->menu_selection - 1 < 0) ? 2 : model->menu_selection - 1;
        if (cmd == CMD_DOWN)
            model->menu_selection = (model->menu_selection + 1 > 2) ? 0 : model->menu_selection + 1;

        if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true;
            if (model->menu_selection == 0)
            { // Sauvegarder
                model_scan_saves(model);
                model->state = STATE_SAVE_SELECT;
                model->menu_selection = 0;
            }
            else if (model->menu_selection == 1)
                reset_game(model); // Rejouer
            else if (model->menu_selection == 2)
            { // Quitter
                model->previous_state = STATE_GAME_OVER;
                model->state = STATE_CONFIRM_QUIT;
                model->menu_selection = 1;
            }
        }
        return;
    }

    // ---------------------------------------------------------
    // 7. SYSTÈME DE SAUVEGARDE (Select / Input / Confirm)
    // ---------------------------------------------------------

    // Étape A : Choix du Slot
    if (model->state == STATE_SAVE_SELECT)
    {
        if (cmd == CMD_PAUSE || cmd == CMD_EXIT)
        {
            model->state = STATE_PAUSED;
            return;
        } // Retour

        int max = model->save_file_count; // +1 pour "Nouveau" (index 0)
        if (cmd == CMD_UP)
            model->menu_selection = (model->menu_selection - 1 < 0) ? max : model->menu_selection - 1;
        if (cmd == CMD_DOWN)
            model->menu_selection = (model->menu_selection + 1 > max) ? 0 : model->menu_selection + 1;

        if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true;
            if (model->menu_selection == 0)
            { // Nouveau
                model->state = STATE_SAVE_INPUT;
                model->input_buffer[0] = '\0';
            }
            else
            { // Écraser existant
                // On copie le nom sans ".dat" pour l'éditer si besoin
                char *f = model->save_files[model->menu_selection - 1];
                int len = strlen(f) - 4;
                if (len > 0)
                {
                    strncpy(model->input_buffer, f, len);
                    model->input_buffer[len] = '\0';
                }

                model->state = STATE_OVERWRITE_CONFIRM;
                model->menu_selection = 0;
            }
        }
        return;
    }

    // Étape B : Saisie du nom
    if (model->state == STATE_SAVE_INPUT)
    {
        if (cmd == CMD_PAUSE)
        {
            model->state = STATE_PAUSED;
            return;
        }

        if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            if (strlen(model->input_buffer) > 0)
            {
                model->sounds.play_select_sound = true;
                char filename[64];
                snprintf(filename, 64, "%s.dat", model->input_buffer);

                if (save_file_exists(filename))
                {
                    model->state = STATE_OVERWRITE_CONFIRM;
                    model->menu_selection = 1; // "Non" par défaut
                }
                else
                {
                    model_save_named(model, filename);
                    model->state = STATE_SAVE_SUCCESS;
                    model->save_success_timer = 2.0f;
                }
            }
        }
        // Note: La saisie des caractères (A-Z) est gérée directement dans model_handle_input
        // via SDL_TEXTINPUT ou les touches, mais ici tu as géré ça dans view_sdl.c qui remplit input_buffer.
        // On gère juste Backspace ici si ce n'est pas fait ailleurs.
        if (cmd == CMD_BACKSPACE)
        {
            int l = strlen(model->input_buffer);
            if (l > 0)
                model->input_buffer[l - 1] = '\0';
        }
        return;
    }

    // Étape C : Confirmation Écrasement
    if (model->state == STATE_OVERWRITE_CONFIRM)
    {
        if (cmd == CMD_LEFT || cmd == CMD_RIGHT || cmd == CMD_UP || cmd == CMD_DOWN)
            model->menu_selection = !model->menu_selection;

        if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true;
            if (model->menu_selection == 0)
            { // OUI
                char filename[64];
                snprintf(filename, 64, "%s.dat", model->input_buffer); // On n'oublie pas le .dat !
                model_save_named(model, filename);

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

    // ---------------------------------------------------------
    // 8. CONFIRMATION QUITTER
    // ---------------------------------------------------------
    if (model->state == STATE_CONFIRM_QUIT)
    {
        if (cmd == CMD_UP)
            model->menu_selection = (model->menu_selection - 1 < 0) ? 2 : model->menu_selection - 1;
        if (cmd == CMD_DOWN)
            model->menu_selection = (model->menu_selection + 1 > 2) ? 0 : model->menu_selection + 1;

        if (cmd == CMD_RETURN || cmd == CMD_SHOOT)
        {
            model->sounds.play_select_sound = true;
            if (model->menu_selection == 0)
                exit(0); // Quitter
            else if (model->menu_selection == 1)
            { // Retour
                // On retourne d'où on vient
                model->state = (model->previous_state == STATE_GAME_OVER) ? STATE_GAME_OVER : STATE_PAUSED;
                model->menu_selection = 3;
            }
            else if (model->menu_selection == 2)
            { // Sauver et Quitter
                model_scan_saves(model);
                model->state = STATE_SAVE_SELECT;
                model->menu_selection = 0;
            }
        }
        return;
    }
}

// ============================================================================
//                          6. MISE À JOUR DU MONDE (GAME LOOP)
// ============================================================================

void model_update(GameModel *model, double dt)
{
    // A. ÉTATS SPÉCIAUX
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

    // B. TIMERS
    if (model->player.shoot_timer > 0)
        model->player.shoot_timer -= dt;
    if (model->hit_timer > 0)
        model->hit_timer -= dt;

    float beat = 0.5f - (model->level * 0.05f);
    if (beat < 0.05f)
        beat = 0.05f;
    model->animation_timer += dt;
    if (model->animation_timer >= beat)
    {
        model->animation_frame = !model->animation_frame;
        model->animation_timer = 0;
        model->sounds.play_beat = true;
        model->sounds.beat_index = (model->sounds.beat_index + 1) % 4;
    }

    // C. JOUEUR
    if (model->player.active)
    {
        model->player.x += model->player.dx * dt;
        if (model->player.x < 0)
            model->player.x = 0;
        if (model->player.x > GAME_WIDTH - PLAYER_WIDTH)
            model->player.x = GAME_WIDTH - PLAYER_WIDTH;
    }

    // D. UFO (OVNI)
    if (model->ufo.active)
    {
        model->sounds.ufo_loopING = !model->ufo.exploding;

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
                (model->ufo.dx < 0 && model->ufo.x < -UFO_WIDTH))
            {
                model->ufo.active = false;
            }
        }
    }
    else
    {
        model->sounds.ufo_loopING = false;
        // Vérifie s'il reste des ennemis vivants
        bool enemies_alive = false;
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (model->enemies[i].active)
            {
                enemies_alive = true;
                break;
            }

        // Spawn aléatoire
        if (!model->ufo.hasSpawnedThisLevel && enemies_alive && (rand() % 500 == 0))
            spawn_ufo(model);
    }

    // E. ENNEMIS
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

    if (active_enemies == 0 && !model->ufo.active)
    {
        model->level++;
        model->sounds.play_level_up = true;
        init_enemies(model);
        return;
    }

    if (touch_edge)
    {
        model->direction_enemies *= -1;
        float dy = (model->drop_direction == 1) ? ENEMY_DROP_HEIGHT : -ENEMY_DROP_HEIGHT;
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
                model->enemies[i].y += dy;
                model->enemies[i].x += model->direction_enemies * 2.0f;
            }
    }
    else
    {
        float spd = ENEMY_SPEED_BASE * model->enemy_speed_mult * model->direction_enemies;
        for (int i = 0; i < MAX_ENEMIES; i++)
            if (model->enemies[i].active && !model->enemies[i].exploding)
                model->enemies[i].x += spd * dt;
    }

    // Tirs Ennemis
    if ((rand() % 100) < (model->level * 2))
    {
        for (int k = 0; k < 10; k++)
        {
            int idx = rand() % MAX_ENEMIES;
            if (model->enemies[idx].active && !model->enemies[idx].exploding)
            {
                spawn_bullet(model, model->enemies[idx].x + ENEMY_WIDTH / 2.0f, model->enemies[idx].y + ENEMY_HEIGHT, BULLET_SPEED * 0.6f, ENTITY_BULLET_ENEMY);
                break;
            }
        }
    }

    // F. BALLES & COLLISIONS
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        Entity *b = &model->bullets[i];
        if (!b->active)
            continue;

        b->y += b->dy * dt;
        b->anim_timer += dt;
        if (b->anim_timer > 0.1f)
        {
            b->anim_timer = 0;
            b->anim_frame = (b->anim_frame + 1) % 4;
        }
        if (b->y < -10 || b->y > GAME_HEIGHT)
        {
            b->active = false;
            continue;
        }

        // --- Boucliers ---
        bool hit_shield = false;
        for (int s = 0; s < MAX_SHIELDS; s++)
        {
            if (model->shields[s].active)
            {
                if (b->x < model->shields[s].x + model->shields[s].width &&
                    b->x + b->width > model->shields[s].x &&
                    b->y < model->shields[s].y + model->shields[s].height &&
                    b->y + b->height > model->shields[s].y)
                {
                    b->active = false;
                    hit_shield = true;
                    model->shields[s].health--;
                    if (model->shields[s].health <= 0)
                        model->shields[s].active = false;
                    break;
                }
            }
        }
        if (hit_shield)
            continue;

        // --- Collisions Spécifiques ---
        if (b->type == ENTITY_BULLET_PLAYER)
        {

            // 1. VS UFO (COLLISION MANUELLE - C'EST LA CORRECTION !)
            if (model->ufo.active && !model->ufo.exploding)
            {
                // Vérification collision Rectangle vs Rectangle
                if (b->x < model->ufo.x + model->ufo.width &&
                    b->x + b->width > model->ufo.x &&
                    b->y < model->ufo.y + model->ufo.height &&
                    b->y + b->height > model->ufo.y)
                {
                    // --- IMPACT ---
                    b->active = false;               // La balle disparaît
                    model->ufo.exploding = true;     // Déclenche l'image ufoExplosion.bmp
                    model->ufo.explode_timer = 0.5f; // L'explosion reste 0.5 secondes

                    // --- RÉCOMPENSES ---
                    model->score += 100; // +100 Points
                    model->lives++;      // +1 Cœur Bonus !

                    // --- AUDIO ---
                    // play_invader_killed jouera le son d'explosion
                    model->sounds.play_invader_killed = true;

                    // Note: Le son de boucle (ufoSound.wav) s'arrêtera tout seul
                    // car ufo_loopING regarde si (!model->ufo.exploding) plus haut dans le code.

                    continue; // On passe à la balle suivante
                }
            }

            // 2. VS ENNEMIS
            for (int e = 0; e < MAX_ENEMIES; e++)
            {
                if (model->enemies[e].active && !model->enemies[e].exploding && check_collision(b, &model->enemies[e]))
                {
                    b->active = false;
                    model->enemies[e].exploding = true;
                    model->enemies[e].explode_timer = 0.2f;
                    int pts = (model->enemies[e].type == ENTITY_ENEMY_TYPE_1) ? 10 : (model->enemies[e].type == ENTITY_ENEMY_TYPE_2 ? 20 : 30);
                    model->score += pts;
                    model->sounds.play_invader_killed = true;
                    break;
                }
            }
        }
        else
        {
            // 3. VS JOUEUR
            if (model->player.active && model->hit_timer <= 0 && check_collision(b, &model->player))
            {
                b->active = false;
                model->lives--;
                model->hit_timer = 2.0f;
                model->sounds.play_player_explosion = true;
                if (model->lives <= 0)
                {
                    model->state = STATE_GAME_OVER;
                    model->sounds.play_game_over = true;
                }
            }
        }
    }
}
// ============================================================================
//                          7. GESTION DES FICHIERS (SAUVEGARDE / CHARGEMENT)
// ============================================================================

// 1. Lister les sauvegardes disponibles
void model_scan_saves(GameModel *model)
{
    model->save_file_count = 0;

    // Ouverture du dossier (Standard POSIX/Linux)
    DIR *d = opendir("sauvegardes");
    if (!d)
        return; // Le dossier n'existe peut-être pas encore

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        // On ignore les dossiers cachés (. et ..)
        if (dir->d_name[0] == '.')
            continue;

        // On cherche uniquement les fichiers finissant par .dat
        if (strstr(dir->d_name, ".dat"))
        {
            if (model->save_file_count < MAX_SAVE_FILES)
            {
                // On copie le nom (ex: "partie1.dat")
                snprintf(model->save_files[model->save_file_count], 256, "%s", dir->d_name);
                model->save_file_count++;
            }
        }
    }
    closedir(d);
}

// 2. Sauvegarder l'état actuel
void model_save_named(const GameModel *model, const char *filename)
{
    // On s'assure que le dossier existe (Linux : 0777 = permissions complètes)
    mkdir("sauvegardes", 0777);

    char path[128];
    snprintf(path, sizeof(path), "sauvegardes/%s", filename);

    FILE *f = fopen(path, "wb"); // Mode binaire
    if (f)
    {
        // On écrit toute la structure d'un coup (Dump mémoire)
        fwrite(model, sizeof(GameModel), 1, f);
        fclose(f);
        printf("[SYSTEM] Sauvegarde reussie : %s\n", path);
    }
    else
    {
        fprintf(stderr, "[ERREUR] Impossible d'ecrire dans %s\n", path);
    }
}

// 3. Charger une sauvegarde
bool model_load_named(GameModel *model, const char *filename)
{
    char path[128];
    snprintf(path, sizeof(path), "sauvegardes/%s", filename);

    FILE *f = fopen(path, "rb"); // Mode binaire
    if (!f)
        return false;

    GameModel temp;
    // On lit le fichier dans une variable temporaire pour vérifier l'intégrité
    size_t res = fread(&temp, sizeof(GameModel), 1, f);
    fclose(f);

    if (res == 1)
    {
        // Copie brutale de la mémoire : On remplace l'état actuel par celui du fichier
        *model = temp;

        // --- FORÇAGE DE L'ÉTAT ---
        // On veut reprendre le jeu tout de suite, pas être en pause
        model->state = STATE_PLAYING;

        // Reset des timers visuels pour éviter des glitchs
        model->hit_timer = 0;

        // On s'assure que le son est coupé ou reset (les pointeurs audio sont gérés par la Vue, pas le Modèle)
        // Mais on reset les flags de sons "One Shot" pour ne pas entendre un vieux bruit
        memset(&model->sounds, 0, sizeof(SoundState));

        printf("[SYSTEM] Chargement reussi : %s\n", path);
        return true;
    }

    printf("[ERREUR] Fichier de sauvegarde corrompu.\n");
    return false;
}