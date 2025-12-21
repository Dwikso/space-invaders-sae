#include "view_ncurses.h"
#include <ncurses.h>
#include <string.h>

// --- Constantes graphiques ASCII ---
static const char *SPRITE_PLAYER = "_^_";
static const char *SPRITE_ENEMY = "/M\\";
static const char CHAR_BULLET = '|';

// --- Fonctions internes ---

static bool ncurses_init(void)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    nodelay(stdscr, TRUE);

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
    endwin();
}

static void ncurses_render(const GameModel *model)
{
    erase();

    // 1. Dessiner le cadre
    attron(COLOR_PAIR(4));
    mvhline(0, 0, '#', GAME_WIDTH + 2);
    mvhline(GAME_HEIGHT + 1, 0, '#', GAME_WIDTH + 2);
    mvvline(0, 0, '#', GAME_HEIGHT + 2);
    mvvline(0, GAME_WIDTH + 1, '#', GAME_HEIGHT + 2);

    // Infos UI
    mvprintw(0, 2, " SCORE: %d | VIES: %d | NIVEAU: %d ", model->score, model->lives, model->level);

    // --- AFFICHAGE DU MENU ---
    if (model->state == STATE_MENU)
    {
        attron(A_BOLD);
        mvprintw(GAME_HEIGHT / 2 - 4, GAME_WIDTH / 2 - 10, "=== SPACE INVADERS ===");
        attroff(A_BOLD);

        const char *options[] = {"   JOUER   ", "  CHARGER  ", "  QUITTER  "};

        for (int i = 0; i < 3; i++)
        {
            int y = GAME_HEIGHT / 2 - 1 + (i * 2);
            int x = GAME_WIDTH / 2 - 6;

            if (i == model->menu_selection)
            {
                attron(A_REVERSE); // Surlignage
                mvprintw(y, x, "> %s <", options[i]);
                attroff(A_REVERSE);
            }
            else
            {
                mvprintw(y, x, "  %s  ", options[i]);
            }
        }

        mvprintw(GAME_HEIGHT - 2, 2, "Fleches: Naviguer | Espace: Valider");
        attroff(COLOR_PAIR(4));
        refresh();
        return;
    }

    if (model->state == STATE_GAME_OVER)
    {
        mvprintw(GAME_HEIGHT / 2, GAME_WIDTH / 2 - 5, "GAME OVER");
        mvprintw(GAME_HEIGHT / 2 + 1, GAME_WIDTH / 2 - 10, "Score Final: %d", model->score);
        mvprintw(GAME_HEIGHT / 2 + 3, GAME_WIDTH / 2 - 12, "Appuyez sur TIR pour Menu");
        attroff(COLOR_PAIR(4));
        refresh();
        return;
    }
    attroff(COLOR_PAIR(4));

    // 2. Dessiner le Joueur
    if (model->player.active)
    {
        attron(COLOR_PAIR(1));
        mvprintw((int)model->player.y + 1, (int)model->player.x + 1, "%s", SPRITE_PLAYER);
        attroff(COLOR_PAIR(1));
    }

    // 3. Dessiner les Ennemis
    attron(COLOR_PAIR(2));
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (model->enemies[i].active)
        {
            mvprintw((int)model->enemies[i].y + 1, (int)model->enemies[i].x + 1, "%s", SPRITE_ENEMY);
        }
    }
    attroff(COLOR_PAIR(2));

    // 4. Dessiner les Balles
    attron(COLOR_PAIR(3));
    for (int i = 0; i < MAX_BULLETS; i++)
    {
        if (model->bullets[i].active)
        {
            mvaddch((int)model->bullets[i].y + 1, (int)model->bullets[i].x + 1, CHAR_BULLET);
        }
    }
    attroff(COLOR_PAIR(3));

    refresh();
}

static GameCommand ncurses_get_input(void)
{
    int ch = getch();

    switch (ch)
    {
    case KEY_LEFT:
        return CMD_MOVE_LEFT;
    case KEY_RIGHT:
        return CMD_MOVE_RIGHT;
    case KEY_UP:
        return CMD_UP; // <--- Menu Haut
    case KEY_DOWN:
        return CMD_DOWN; // <--- Menu Bas
    case ' ':
        return CMD_SHOOT; // <--- Valider / Tirer
    case '\n':
        return CMD_SHOOT; // <--- Entrée valide aussi
    case 'p':
        return CMD_PAUSE;
    case 'q':
        return CMD_EXIT;
    default:
        return CMD_NONE;
    }
}

const ViewInterface view_ncurses = {
    .init = ncurses_init,
    .close = ncurses_close,
    .render = ncurses_render,
    .get_input = ncurses_get_input};