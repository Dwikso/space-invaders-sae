/**
 * @file utils.h
 * @brief Fonctions utilitaires génériques (Temps, Aléatoire, Système).
 *
 * Ce module fournit des aides indépendantes de la logique du jeu,
 * utilisées pour la gestion du temps (boucle de jeu) et la génération
 * de nombres aléatoires.
 *
 * @date 2024
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// ============================================================================
//                          GESTION DU TEMPS
// ============================================================================

/**
 * @brief Récupère le temps actuel avec une haute précision.
 * Utilise l'horloge monotone du système pour éviter les sauts temporels.
 *
 * @return Le temps actuel en secondes (ex: 1234.5678).
 */
double utils_get_time(void);

/**
 * @brief Met le thread principal en pause.
 * Utilisé pour limiter les FPS (Frames Per Second).
 *
 * @param ms Durée de la pause en millisecondes.
 */
void utils_sleep_ms(int ms);

// ============================================================================
//                          MATHS & ALÉATOIRE
// ============================================================================

/**
 * @brief Génère un nombre entier pseudo-aléatoire dans un intervalle donné.
 *
 * @param min Valeur minimale (incluse).
 * @param max Valeur maximale (incluse).
 * @return Un entier n tel que min <= n <= max.
 */
int utils_random_int(int min, int max);

#endif // UTILS_H