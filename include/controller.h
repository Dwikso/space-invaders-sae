/**
 * @file controller.h
 * @brief Définition des commandes abstraites (Input) du jeu.
 *
 * Ce fichier définit l'interface entre les entrées brutes (Clavier SDL, Ncurses, Joystick)
 * et la logique du jeu. Le modèle ne connaît pas les touches "Z", "Q", "Flèche",
 * il ne connaît que des commandes abstraites comme CMD_MOVE_LEFT.
 *
 * @date 2024
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

// ============================================================================
//                          COMMANDES ABSTRAITES
// ============================================================================

/**
 * @brief Liste de toutes les actions possibles que le joueur peut déclencher.
 * * Cette énumération permet de découpler la logique du jeu (Modèle)
 * de la gestion des périphériques (Vue/Inputs).
 */
typedef enum
{
    /** @name Commandes Système */
    ///@{
    CMD_NONE,  ///< Aucune action (état par défaut ou touche relâchée)
    CMD_PAUSE, ///< Mettre en pause ou reprendre le jeu (ex: P, Echap)
    CMD_EXIT,  ///< Demande de fermeture du jeu ou d'annulation rapide
    ///@}

    /** @name Commandes de Jeu (Gameplay) */
    ///@{
    CMD_MOVE_LEFT,  ///< Déplacement continu du joueur vers la gauche
    CMD_MOVE_RIGHT, ///< Déplacement continu du joueur vers la droite
    CMD_SHOOT,      ///< Action de tirer (ex: Espace, Entrée)
    ///@}

    /** @name Navigation dans les Menus */
    ///@{
    CMD_UP,        ///< Monter le curseur / Option précédente
    CMD_DOWN,      ///< Descendre le curseur / Option suivante
    CMD_LEFT,      ///< Diminuer une valeur (ex: Volume) / Gauche dans menu
    CMD_RIGHT,     ///< Augmenter une valeur (ex: Volume) / Droite dans menu
    CMD_RETURN,    ///< Valider une sélection (Entrée)
    CMD_BACKSPACE, ///< Effacer un caractère (pour la saisie du nom de sauvegarde)
    ///@}

} GameCommand;

#endif // CONTROLLER_H