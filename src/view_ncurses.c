#include "view_ncurses.h"
#include <ncurses.h>
#include <string.h> // Pour strlen

// --- Constantes graphiques ASCII ---
static const char *SPRITE_PLAYER = "_^_";
static const char *SPRITE_ENEMY = "/M\\";
static const char CHAR_BULLET = '|';

// --- Fonctions internes ---

// Initialisation de Ncurses
static bool ncurses_init(void)
{
    initscr();             // Démarre le mode ncurses
    cbreak();              // Pas besoin d'appuyer sur Entrée
    noecho();              // Ne pas afficher les touches pressées
    keypad(stdscr, TRUE);  // Active les flèches directionnelles
    curs_set(0);           // Cache le curseur clignotant
    nodelay(stdscr, TRUE); // getch() ne bloque pas le programme (essentiel !)

    // Gestion des couleurs
    if (has_colors())
    {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Joueur
        init_pair(2, COLOR_RED, COLOR_BLACK);    // Ennemis
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Balles
        init_pair(4, COLOR_WHITE, COLOR_BLUE);   // UI / Bordures
    }
    return true;
}

static void ncurses_close(void)
{
    endwin(); // Restaure le terminal normal
}

// Fonction de rendu principale
static void ncurses_render(const GameModel *model)
{
    erase(); // Efface l'écran à chaque frame

    // 1. Dessiner le cadre de jeu (délimitation visuelle)
    // On dessine un cadre autour de la zone 0,0 -> GAME_WIDTH, GAME_HEIGHT
    attron(COLOR_PAIR(4));
    mvhline(0, 0, '#', GAME_WIDTH + 2);
    mvhline(GAME_HEIGHT + 1, 0, '#', GAME_WIDTH + 2);
    mvvline(0, 0, '#', GAME_HEIGHT + 2);
    mvvline(0, GAME_WIDTH + 1, '#', GAME_HEIGHT + 2);

    // Infos UI
    mvprintw(0, 2, " SCORE: %d | VIES: %d | NIVEAU: %d ", model->score, model->lives, model->level);

    if (model->state == STATE_MENU)
    {
        mvprintw(GAME_HEIGHT / 2, GAME_WIDTH / 2 - 10, "SPACE INVADERS");
        mvprintw(GAME_HEIGHT / 2 + 1, GAME_WIDTH / 2 - 14, "Appuyez sur ESPACE pour tirer/jouer");
        mvprintw(GAME_HEIGHT / 2 + 2, GAME_WIDTH / 2 - 14, "Appuyez sur Q pour quitter");
        attroff(COLOR_PAIR(4));
        refresh();
        return;
    }

    if (model->state == STATE_GAME_OVER)
    {
        mvprintw(GAME_HEIGHT / 2, GAME_WIDTH / 2 - 5, "GAME OVER");
        mvprintw(GAME_HEIGHT / 2 + 1, GAME_WIDTH / 2 - 10, "Score Final: %d", model->score);
        attroff(COLOR_PAIR(4));
        refresh();
        return;
    }
    attroff(COLOR_PAIR(4));

    // 2. Dessiner le Joueur
    if (model->player.active)
    {
        attron(COLOR_PAIR(1)); // Vert
        // On convertit les float du modèle en int pour l'affichage
        mvprintw((int)model->player.y + 1, (int)model->player.x + 1, "%s", SPRITE_PLAYER);
        attroff(COLOR_PAIR(1));
    }

    // 3. Dessiner les Ennemis
    attron(COLOR_PAIR(2)); // Rouge
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (model->enemies[i].active)
        {
            mvprintw((int)model->enemies[i].y + 1, (int)model->enemies[i].x + 1, "%s", SPRITE_ENEMY);
        }
    }
    attroff(COLOR_PAIR(2));

    // 4. Dessiner les Balles
    attron(COLOR_PAIR(3)); // Jaune
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (model->bullets[i].active)
        {
            mvaddch((int)model->bullets[i].y + 1, (int)model->bullets[i].x + 1, CHAR_BULLET);
        }
    }
    attroff(COLOR_PAIR(3));

    refresh(); // Applique les changements à l'écran
}

// Gestion des entrées Clavier -> Commandes de Jeu
static GameCommand ncurses_get_input(void)
{
    int ch = getch(); // Lecture non bloquante

    switch (ch)
    {
    case KEY_LEFT:
        return CMD_MOVE_LEFT;
    case KEY_RIGHT:
        return CMD_MOVE_RIGHT;
    case ' ':
        return CMD_SHOOT;
    case 'p':
        return CMD_PAUSE;
    case 'q':
        return CMD_EXIT;
    default:
        return CMD_NONE;
    }
}

// --- Définition de l'interface publique ---
// C'est cette structure que le main.c va récupérer
const ViewInterface view_ncurses = {
    .init = ncurses_init,
    .close = ncurses_close,
    .render = ncurses_render,
    .get_input = ncurses_get_input};