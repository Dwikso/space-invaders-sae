/**
 * @file view_interface.h
 * @brief Définition de l'interface abstraite pour les Vues.
 *
 * Ce fichier définit un "contrat" (via une structure de pointeurs de fonctions)
 * que toutes les interfaces graphiques (SDL, Ncurses, etc.) doivent respecter.
 * Cela permet au moteur de jeu d'utiliser n'importe quelle vue de manière interchangeable
 * (Pattern Strategy / Polymorphisme).
 *
 */

#ifndef VIEW_INTERFACE_H
#define VIEW_INTERFACE_H

#include "model.h"
#include "controller.h"

// ============================================================================
//                          INTERFACE (VTABLE)
// ============================================================================

/**
 * @brief Structure regroupant les méthodes virtuelles de la Vue.
 * Une implémentation concrète (ex: view_sdl) doit remplir ces pointeurs.
 */
typedef struct
{
    /**
     * @brief Initialise la librairie graphique (Fenêtre, Textures, Sons, etc.).
     * @return true en cas de succès, false si une erreur critique survient.
     */
    bool (*init)(void);

    /**
     * @brief Ferme la fenêtre et libère toutes les ressources allouées (Textures, Mémoire).
     * Doit être appelé avant de quitter le programme.
     */
    void (*close)(void);

    /**
     * @brief Dessine l'état actuel du jeu à l'écran.
     * Cette fonction est appelée à chaque frame (60 fois par seconde).
     * @param model Pointeur constant vers le modèle (Lecture seule pour l'affichage).
     */
    void (*render)(const GameModel *model);

    /**
     * @brief Récupère les entrées utilisateur (Clavier, Souris, Manette).
     * @param model Pointeur vers le modèle (utile si l'input dépend de l'état, ex: saisie de texte).
     * @return Une commande abstraite (GameCommand) à traiter par le contrôleur.
     */
    GameCommand (*get_input)(GameModel *model);

} ViewInterface;

#endif // VIEW_INTERFACE_H