/**
 * @file model.h
 * @brief Définition du modèle de données (Data Model) du jeu Space Invaders.
 *
 * Ce fichier contient toutes les structures de données représentant l'état du jeu,
 * les constantes de gameplay, ainsi que les prototypes des fonctions de manipulation
 * du modèle (MVC).
 *
 */

#ifndef MODEL_H
#define MODEL_H

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

#include "common.h"
#include "controller.h" // Pour l'enum GameCommand

// ============================================================================
//                              CONSTANTES & MACROS
// ============================================================================

/** @name Système de Sauvegarde */
///@{
#define MAX_SAVE_FILES 10   ///< Nombre maximum de fichiers de sauvegarde détectés
#define MAX_FILENAME_LEN 32 ///< Longueur maximale d'un nom de fichier
///@}

/** @name Configuration Gameplay */
///@{
#define MAX_SHIELDS 4        ///< Nombre de boucliers (bunkers)
#define SHIELD_MAX_HEALTH 10 ///< Points de vie d'un bouclier (1 état intact + 9 états de dégâts)
#define MAX_LIVES_NORMAL 3   ///< Nombre de vies de départ
///@}

/** @name Configuration UFO (OVNI) */
///@{
#define UFO_POINTS 60   ///< Points gagnés en détruisant l'OVNI
#define UFO_SPEED 10.0f ///< Vitesse de déplacement horizontal de l'OVNI
#define UFO_WIDTH 4     ///< Largeur logique de l'OVNI
#define UFO_HEIGHT 2    ///< Hauteur logique de l'OVNI
///@}

// ============================================================================
//                              ENUMÉRATIONS
// ============================================================================

/**
 * @brief Types d'entités présentes dans le jeu.
 * Utiliser pour distinguer le comportement lors du rendu et des collisions.
 */
typedef enum
{
    ENTITY_PLAYER,        ///< Le vaisseau du joueur
    ENTITY_BULLET_PLAYER, ///< Projectile tiré par le joueur
    ENTITY_BULLET_ENEMY,  ///< Projectile tiré par un ennemi
    ENTITY_ENEMY_TYPE_1,  ///< Ennemi rangée du bas (10 pts) - "Octopus"
    ENTITY_ENEMY_TYPE_2,  ///< Ennemi rangée du milieu (20 pts) - "Crab"
    ENTITY_ENEMY_TYPE_3,  ///< Ennemi rangée du haut (40 pts) - "Squid"
    ENTITY_UFO
} EntityType;

/**
 * @brief États possibles de la machine à états du jeu (FSM).
 * Gère les transitions entre les différents écrans et modes.
 */
typedef enum
{
    STATE_MENU,              ///< Menu principal
    STATE_PLAYING,           ///< Jeu en cours
    STATE_PAUSED,            ///< Jeu en pause (avec menu contextuel)
    STATE_TUTORIAL,          ///< Écran d'explication des touches
    STATE_GAME_OVER,         ///< Écran de fin de partie (Défaite)
    STATE_VICTORY,           ///< Écran de victoire (Fin du jeu)
    STATE_CONFIRM_QUIT,      ///< Pop-up de confirmation pour quitter
    STATE_SAVE_INPUT,        ///< Écran de saisie du nom de sauvegarde
    STATE_LOAD_MENU,         ///< Menu de chargement d'une partie
    STATE_OVERWRITE_CONFIRM, ///< Confirmation d'écrasement de sauvegarde
    STATE_SAVE_SELECT,       ///< Menu de sélection d'emplacement de sauvegarde
    STATE_SAVE_SUCCESS       ///< Écran temporaire de succès après sauvegarde
} GameStateEnum;

// ============================================================================
//                          STRUCTURES DE DONNÉES
// ============================================================================

/**
 * @brief Représente une entité générique du jeu (Vaisseau, Ennemi, Projectile).
 */
typedef struct
{
    float x;         ///< Position horizontale
    float y;         ///< Position verticale
    int width;       ///< Largeur de la hitbox
    int height;      ///< Hauteur de la hitbox
    float dx;        ///< Vitesse horizontale
    float dy;        ///< Vitesse verticale
    bool active;     ///< Si false, l'entité est ignorée (morte/détruite)
    EntityType type; ///< Le type de l'entité (Joueur, Ennemi, etc.)

    float shoot_timer; ///< Temps restant avant le prochain tir (Cooldown)

    // --- Animation ---
    float anim_timer; ///< Temps écoulé depuis la dernière frame d'animation
    int anim_frame;   ///< Index de l'image actuelle (sprite animation)

    // --- État Explosion ---
    bool exploding;      ///< Indique si l'entité est en train d'exploser
    float explode_timer; ///< Durée restante de l'animation d'explosion
} Entity;

/**
 * @brief Structure spécifique pour l'OVNI bonus (Mystery Ship).
 */
typedef struct
{
    float x, y;               ///< Position
    float width, height;      ///< Dimensions
    float dx;                 ///< Direction et vitesse de déplacement
    bool active;              ///< Est-il actuellement présent à l'écran ?
    bool hasSpawnedThisLevel; ///< Empêche l'OVNI d'apparaître plusieurs fois par niveau
    bool exploding;           ///< En cours d'explosion
    float explode_timer;      ///< Durée de l'explosion
    EntityType type;          ///< Type de l'entité (ENTITY_UFO)
} Ufo;

/**
 * @brief Représente un bunker/bouclier destructible.
 */
typedef struct
{
    float x, y;          ///< Position
    float width, height; ///< Dimensions
    int health;          ///< État de santé : 10 (Intact) à 0 (Détruit)
    bool active;         ///< Si false, le bouclier est entièrement détruit
} Shield;

/**
 * @brief Système de drapeaux (Flags) pour le moteur audio.
 * * Le Modèle lève ces drapeaux (true), la Vue les lit, joue le son,
 * puis les remet à zéro (false).
 */
typedef struct
{
    // --- SFX Ponctuels ---
    bool play_shoot;            ///< Son de tir
    bool play_invader_killed;   ///< Son destruction ennemi
    bool play_player_explosion; ///< Son mort du joueur
    bool play_select_sound;     ///< Bruitage "Bip" de validation dans les menus
    bool play_game_over;        ///< Jingle de Game Over
    bool play_level_up;         ///< Jingle de changement de niveau

    // --- Ambiances / Boucles ---
    bool play_beat;   ///< Trigger pour le rythme alien
    int beat_index;   ///< Index du son de rythme (0 à 3)
    bool ufo_loopING; ///< État continu : L'UFO est-il actif ?
} SoundState;

/**
 * @brief Structure principale du modèle (God Object).
 * * Contient l'intégralité de l'état du jeu à un instant T.
 */
typedef struct
{
    GameStateEnum state;          ///< État courant de la machine à états
    GameStateEnum previous_state; ///< État précédent (utile pour le retour de Pause/Confirm)

    // --- Entités ---
    Entity player;               ///< Le vaisseau du joueur
    Entity enemies[MAX_ENEMIES]; ///< Tableau des ennemis (Invaders)
    Entity bullets[MAX_BULLETS]; ///< Tableau des projectiles (Joueur + Ennemis)
    Ufo ufo;                     ///< L'OVNI bonus
    Shield shields[MAX_SHIELDS]; ///< Les boucliers de protection

    // --- Données de la partie ---
    int score;            ///< Score actuel
    int lives;            ///< Vies restantes
    int level;            ///< Niveau actuel
    int normal_max_lives; ///< Seuil pour gagner une vie bonus

    // --- Logique des Ennemis (Vague) ---
    float enemy_speed_mult; ///< Multiplicateur de vitesse (augmente avec les niveaux)
    int direction_enemies;  ///< 1 (droite) ou -1 (gauche)
    int drop_direction;     ///< 1 (descend), -1 (remonte - rare)
    int drop_step_count;    ///< Compteur pour l'animation de descente

    // --- Animation Globale ---
    int animation_frame;   ///< Frame globale pour synchroniser les ennemis (0 ou 1)
    float animation_timer; ///< Timer pour le rythme d'animation

    // --- Timers de Jeu ---
    float game_over_timer;    ///< Délai avant interaction sur l'écran Game Over
    float hit_timer;          ///< Temps d'invulnérabilité/clignotement après un coup
    float save_success_timer; ///< Temps d'affichage du message "Sauvegarde Réussie"

    // --- Interface Utilisateur ---
    int menu_selection;                  ///< Index de l'élément sélectionné dans le menu courant
    char input_buffer[MAX_FILENAME_LEN]; ///< Buffer pour la saisie de texte (Nom de sauvegarde)

    // --- Système de Fichiers ---
    char save_files[MAX_SAVE_FILES][256]; ///< Cache des noms de fichiers de sauvegarde trouvés
    int save_file_count;                  ///< Nombre de fichiers trouvés
    bool pending_quit;                    ///< Drapeau : Doit-on quitter après l'opération en cours ?

    // --- Audio ---
    SoundState sounds; ///< État des déclencheurs sonores
    int volume;        ///< Volume global (0 à 100)
    bool is_muted;     ///< Si true, le son est coupé
} GameModel;

// ============================================================================
//                          FONCTIONS PUBLIQUES
// ============================================================================

/**
 * @brief Initialise le modèle de jeu.
 * Alloue la mémoire et configure les valeurs par défaut.
 * @return Un pointeur vers la structure GameModel nouvellement créée.
 */
GameModel *model_init(void);

/**
 * @brief Libère la mémoire allouée pour le modèle.
 * @param model Pointeur vers le modèle à détruire.
 */
void model_free(GameModel *model);

/**
 * @brief Met à jour la logique du jeu pour une frame.
 * Gère les déplacements, collisions, timers et changements d'états.
 * * @param model Pointeur vers le modèle.
 * @param dt Delta Time : temps écoulé depuis la dernière frame (en secondes).
 */
void model_update(GameModel *model, double dt);

/**
 * @brief Traite une commande utilisateur (Input).
 * Modifie l'état du modèle en fonction de l'action demandée (Tir, Mouvement, Menu).
 * * @param model Pointeur vers le modèle.
 * @param cmd La commande reçue du contrôleur (via clavier/manette).
 */
void model_handle_input(GameModel *model, GameCommand cmd);

/**
 * @brief Récupère un pointeur en lecture seule vers le joueur.
 * Utile pour la vue si elle a besoin d'infos spécifiques sans modifier.
 * * @param model Pointeur vers le modèle.
 * @return Pointeur constant vers l'entité joueur.
 */
const Entity *model_get_player(const GameModel *model);

// --- Gestion des Sauvegardes ---

/**
 * @brief Scanne le répertoire des sauvegardes pour remplir la liste save_files.
 * Met à jour model->save_files et model->save_file_count.
 * * @param model Pointeur vers le modèle.
 */
void model_scan_saves(GameModel *model);

/**
 * @brief Sauvegarde l'état actuel du jeu dans un fichier spécifique.
 * * @param model Pointeur vers le modèle à sauvegarder.
 * @param filename Nom du fichier (ex: "partie1.dat").
 */
void model_save_named(const GameModel *model, const char *filename);

/**
 * @brief Charge un état de jeu depuis un fichier.
 * * @param model Pointeur vers le modèle à mettre à jour.
 * @param filename Nom du fichier à charger.
 * @return true si le chargement a réussi, false sinon.
 */
bool model_load_named(GameModel *model, const char *filename);

#endif // MODEL_H