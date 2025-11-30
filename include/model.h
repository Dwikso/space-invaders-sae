#ifndef MODEL_H
#define MODEL_H

#include "common.h"
#include "controller.h" // Pour traiter les commandes

// --- Structures de données ---

typedef enum
{
    ENTITY_PLAYER,
    ENTITY_ENEMY_BASIC,
    ENTITY_ENEMY_FAST,
    ENTITY_BULLET_PLAYER,
    ENTITY_BULLET_ENEMY
} EntityType;

typedef struct
{
    float x, y;        // Position (float pour fluidité)
    int width, height; // Hitbox
    float dx, dy;      // Vitesse vectorielle
    bool active;       // Si false, l'entité n'est pas dessinée/traitée
    EntityType type;
} Entity;

typedef enum
{
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSED,
    STATE_GAME_OVER,
    STATE_VICTORY
} GameStateEnum;

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

    // Logique interne pour les vagues d'ennemis
    float enemy_speed_mult;
    int direction_enemies; // 1 (droite) ou -1 (gauche)
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

#endif // MODEL_H