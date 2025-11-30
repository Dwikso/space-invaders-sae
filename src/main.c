#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "model.h"
#include "view_ncurses.h"
#include "view_sdl.h" // Sera utilisé plus tard
#include "utils.h"

int main(int argc, char *argv[])
{
    // 1. Choix de l'interface (CLI args)
    // Par défaut : Ncurses
    const ViewInterface *view = &view_ncurses;

    if (argc > 1 && strcmp(argv[1], "sdl") == 0)
    {
        printf("Démarrage en mode SDL...\n");
        view = &view_sdl; // On pointera vers SDL
    }
    else
    {
        printf("Démarrage en mode Ncurses...\n");
    }

    // 2. Initialisation du Modèle
    GameModel *model = model_init();
    if (!model)
    {
        fprintf(stderr, "Erreur: Impossible d'allouer le modèle.\n");
        return 1;
    }

    // 3. Initialisation de la Vue
    if (!view->init())
    {
        fprintf(stderr, "Erreur: Impossible d'initialiser la vue.\n");
        model_free(model);
        return 1;
    }

    // 4. Boucle de Jeu (Game Loop)
    bool running = true;
    double last_time = utils_get_time();
    double accumulator = 0.0;
    const double dt = 1.0 / TARGET_FPS; // Pas de temps fixe (0.016s)

    while (running && model->state != STATE_GAME_OVER)
    {
        // A. Gestion du Temps
        double current_time = utils_get_time();
        double frame_time = current_time - last_time;
        last_time = current_time;

        // Protection contre la spirale de la mort (si l'ordi lag trop)
        if (frame_time > 0.25)
            frame_time = 0.25;

        accumulator += frame_time;

        // B. Gestion des Entrées
        GameCommand cmd = view->get_input();
        if (cmd == CMD_EXIT)
        {
            running = false;
        }
        else
        {
            model_handle_input(model, cmd);
        }

        // C. Mise à jour physique (Timestep fixe)
        while (accumulator >= dt)
        {
            model_update(model, dt);
            accumulator -= dt;
        }

        // D. Rendu (Aussi vite que possible, ou vsync)
        view->render(model);

        // Pause pour économiser le CPU (limite à ~60 FPS grossièrement)
        double frame_end = utils_get_time();
        double work_time = frame_end - current_time;
        if (work_time < dt)
        {
            utils_sleep_ms((int)((dt - work_time) * 1000));
        }
    }

    // Si Game Over, on attend un peu avant de fermer
    if (model->state == STATE_GAME_OVER)
    {
        view->render(model);
        utils_sleep_ms(3000);
    }

    // 5. Nettoyage
    view->close();
    model_free(model);

    printf("Merci d'avoir joué !\n");
    return 0;
}