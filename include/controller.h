#ifndef CONTROLLER_H
#define CONTROLLER_H

// Commandes abstraites indépendantes de la touche réelle (Z, Q, Flèches...)
typedef enum
{
    CMD_NONE,       // Aucune action
    CMD_MOVE_LEFT,  // Aller à gauche
    CMD_MOVE_RIGHT, // Aller à droite
    CMD_SHOOT,      // Tirer
    CMD_PAUSE,      // Mettre en pause
    CMD_EXIT,       // Quitter le jeu
    CMD_UP,
    CMD_DOWN,
    CMD_RETURN
} GameCommand;

#endif // CONTROLLER_H