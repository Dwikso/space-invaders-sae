/**
 * @file common.h
 * @brief Constantes globales et configuration générale du projet.
 *
 * Ce fichier contient les paramètres fondamentaux qui sont partagés
 * entre le Modèle, la Vue et le Contrôleur. Il définit notamment
 * les dimensions abstraites du jeu (indépendantes de l'affichage)
 * et les contraintes de performance (FPS, limites mémoire).
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>

// ============================================================================
//                        DIMENSIONS DU MONDE (LOGIQUE)
// ============================================================================

/** * @name Système de Coordonnées
 * Ces dimensions définissent la grille logique du jeu.
 * Le rendu (SDL ou Ncurses) se charge ensuite d'étirer (scale) ces coordonnées
 * pour qu'elles s'adaptent à la taille réelle de la fenêtre ou du terminal.
 */
///@{

/** * @brief Largeur logique du terrain de jeu.
 * Les entités évoluent entre x=0 et x=100.
 */
#define GAME_WIDTH 100

/** * @brief Hauteur logique du terrain de jeu.
 * Les entités évoluent entre y=0 et y=50.
 */
#define GAME_HEIGHT 50

///@}

// ============================================================================
//                        LIMITES MÉMOIRE & OPTIMISATION
// ============================================================================

/** * @name Allocation Statique
 * Ces constantes définissent la taille maximale des tableaux fixes.
 * Cela permet d'éviter l'utilisation lente de `malloc` pendant la boucle de jeu.
 */
///@{

/** * @brief Nombre maximum d'ennemis affichables simultanément.
 * (5 rangées de 11 ennemis = 55, arrondi à 60 par sécurité).
 */
#define MAX_ENEMIES 60

/** * @brief Nombre maximum de projectiles simultanés.
 * Inclut les tirs du joueur et ceux des ennemis.
 */
#define MAX_BULLETS 100

///@}

// ============================================================================
//                        CONFIGURATION DU TIMING (BOUCLE DE JEU)
// ============================================================================

/** * @name Gestion du Temps
 * Paramètres pour réguler la vitesse d'exécution du jeu.
 */
///@{

/** * @brief Fréquence d'images cible (Frames Per Second).
 * Définit la fluidité du jeu. 60 FPS est le standard pour les jeux d'arcade.
 */
#define TARGET_FPS 60

/** * @brief Durée minimale d'une frame en millisecondes.
 * Calculé : 1000ms / 60fps ≈ 16ms.
 * Utilisé pour endormir le processeur (SDL_Delay) si le calcul est trop rapide.
 */
#define FRAME_DELAY (1000 / TARGET_FPS)

///@}

#endif // COMMON_H