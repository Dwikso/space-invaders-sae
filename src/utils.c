#define _POSIX_C_SOURCE 199309L
#include "utils.h"
#include <time.h>
#include <stdlib.h>

// Retourne le temps en secondes avec précision
double utils_get_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void utils_sleep_ms(int ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int utils_random_int(int min, int max)
{
    return min + rand() % (max - min + 1);
}