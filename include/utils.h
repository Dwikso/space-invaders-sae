#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// Retourne le temps actuel en secondes (haute précision)
double utils_get_time(void);

// Pause le programme pour x millisecondes
void utils_sleep_ms(int ms);

// Génère un int aléatoire entre min et max inclus
int utils_random_int(int min, int max);

#endif // UTILS_H