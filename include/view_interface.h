#ifndef VIEW_INTERFACE_H
#define VIEW_INTERFACE_H

#include "model.h"
#include "controller.h"

// Structure contenant des pointeurs de fonctions (Polymorphisme en C)
typedef struct
{
    // Initialise la librairie graphique (fenêtre, assets...)
    // Retourne true si succès, false sinon
    bool (*init)(void);

    // Ferme la fenêtre et libère la mémoire graphique
    void (*close)(void);

    // Dessine tout l'état du jeu
    void (*render)(const GameModel *model);

    // Récupère l'entrée utilisateur et la traduit en commande de jeu
    GameCommand (*get_input)(GameModel *model);
} ViewInterface;

#endif // VIEW_INTERFACE_H