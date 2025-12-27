/**
 * @file view_ncurses.h
 * @brief Implémentation de la Vue pour le terminal (Mode Texte/ASCII).
 *
 * Ce module expose l'instance de l'interface utilisant la librairie Ncurses.
 * Il permet de jouer au jeu dans un terminal, sans fenêtre graphique.
 *
 */

#ifndef VIEW_NCURSES_H
#define VIEW_NCURSES_H

#include "view_interface.h"

/**
 * @brief Instance concrète de l'interface View pour Ncurses.
 *
 * Cette variable globale est définie dans view_ncurses.c.
 * Elle contient les pointeurs vers les fonctions spécifiques à Ncurses
 * (ncurses_init, ncurses_render, etc.).
 */
extern const ViewInterface view_ncurses;

#endif // VIEW_NCURSES_H