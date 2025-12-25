#ifndef MODEL_H
#define MODEL_H

#define MAX_SAVE_FILES 10
#define MAX_FILENAME_LEN 32
#include "common.h"
#include "controller.h" // Pour traiter les commandes
#define MAX_SHIELDS 4
#define SHIELD_MAX_HEALTH 10 // 1 état Full + 9 états Damage

// --- Constantes UFO ---
#define UFO_POINTS 60
#define UFO_SPEED 10.0f // Très rapide !
#define UFO_WIDTH 4     // Taille arbitraire, adapte selon ton image bmp
#define UFO_HEIGHT 2
#define MAX_LIVES_NORMAL 3 // Le nombre de vies "standard" avant le bonus

// --- Structures de données ---

typedef enum
{
    ENTITY_PLAYER,
    ENTITY_BULLET_PLAYER,
    ENTITY_BULLET_ENEMY,
    ENTITY_ENEMY_TYPE_1, // En bas (10 pts) - "Octopus"
    ENTITY_ENEMY_TYPE_2, // Milieu (20 pts) - "Crab"
    ENTITY_ENEMY_TYPE_3  // En haut (40 pts) - "Squid"
} EntityType;

typedef struct
{
    float x, y;        // Position (float pour fluidité)
    int width, height; // Hitbox
    float dx, dy;      // Vitesse vectorielle
    bool active;       // Si false, l'entité n'est pas dessinée/traitée
    EntityType type;

    // --- AJOUT IMPORTANT POUR LE TIR ---
    float shoot_timer; // Temps restant avant le prochain tir (Cooldown)
    // -----------------------------------
    // --- NOUVEAU POUR ANIMATION DES BALLES ---
    float anim_timer; // Temps écoulé depuis la dernière frame d'animation
    int anim_frame;   // Numéro de l'image (0, 1, 2 ou 3)
    // ----------------------------------------
    bool exploding;      // Est-ce qu'il est touché ?
    float explode_timer; // Combien de temps afficher l'explosion
} Entity;

typedef enum
{
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_TUTORIAL,
    STATE_GAME_OVER,
    STATE_VICTORY,
    STATE_CONFIRM_QUIT,
    STATE_SAVE_INPUT,
    STATE_LOAD_MENU,
    STATE_OVERWRITE_CONFIRM,
    STATE_SAVE_SELECT,
} GameStateEnum;

// Structure spécifique pour l'OVNI
typedef struct
{
    float x, y;
    float width, height;
    float dx;                 // Direction et vitesse horizontale
    bool active;              // Est-il actuellement à l'écran ?
    bool hasSpawnedThisLevel; // Pour s'assurer qu'il ne passe qu'une fois par niveau
    bool exploding;
    float explode_timer;
} Ufo;

typedef struct
{
    float x, y;
    float width, height;
    int health; // 10 = Full, 9 = Damage_1 ... 1 = Damage_9, 0 = Mort
    bool active;
} Shield;
typedef struct
{
    // Drapeaux (Flags) : Si true, le son doit être joué, puis remis à false par la vue
    bool play_shoot;
    bool play_invader_killed;
    bool play_player_explosion;

    // Pour le rythme de fond (Beat)
    bool play_beat; // Doit-on jouer une note de fond ?
    int beat_index; // 0, 1, 2 ou 3 (pour fastinvader1..4)

    // État continu
    bool ufo_loopING; // L'UFO est-il en train de faire du bruit ?
} SoundState;
typedef struct
{
    GameStateEnum state;

    Entity player;

    // Tableaux statiques pour simplifier la gestion mémoire au début
    Entity enemies[MAX_ENEMIES];
    Entity bullets[MAX_BULLETS];

    int score;
    int lives;
    int level;
    int menu_selection;

    // Logique interne pour les vagues d'ennemis
    float enemy_speed_mult;
    int direction_enemies; // 1 (droite) ou -1 (gauche)
    int animation_frame;   // 0 ou 1
    float animation_timer;

    // NOUVEAUX CHAMPS POUR LE MOUVEMENT
    int drop_direction;  // 1 = descend, -1 = monte
    int drop_step_count; // Compteur : de 0 à 4

    float game_over_timer; // Compteur de temps pour le Game Over
    float hit_timer;

    Shield shields[MAX_SHIELDS];
    // NOUVEAU : L'OVNI
    Ufo ufo;

    // NOUVEAU : On garde en mémoire le max normal pour savoir quand donner le bonus
    int normal_max_lives;
    GameStateEnum previous_state;

    // NOUVEAU : Gestion Sauvegarde
    char input_buffer[MAX_FILENAME_LEN]; // Ce que le joueur tape

    // NOUVEAU : Liste des fichiers pour le menu charger
    char save_files[MAX_SAVE_FILES][256]; // Liste des noms de fichiers
    int save_file_count;                  // Combien on en a trouvé
    bool pending_quit;                    // Est-ce qu'on doit quitter après la sauvegarde ?
    SoundState sounds;

    // --- AJOUT GESTION VOLUME ---
    int volume;    // De 0 à 100
    bool is_muted; // true = silence total
    // ---------------------------
} GameModel;

// --- Fonctions Publiques (API du Modèle) ---

// Initialisation et nettoyage
GameModel *model_init(void);
void model_free(GameModel *model);

// Mise à jour (appeler à chaque frame)
// dt = delta time (temps écoulé depuis la dernière frame en secondes)
void model_update(GameModel *model, double dt);

// Interaction
void model_handle_input(GameModel *model, GameCommand cmd);

// Getters pour l'affichage (lecture seule recommandée)
const Entity *model_get_player(const GameModel *model);
// AJOUTE CES 3 LIGNES :
void model_scan_saves(GameModel *model);
void model_save_named(const GameModel *model, const char *filename);
bool model_load_named(GameModel *model, const char *filename);

#endif // MODEL_H