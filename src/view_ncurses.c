#include "view_ncurses.h"
#include <ncurses.h>
#include <string.h>
#include <ctype.h>

// --- 1. NOUVEAUX SPRITES ASCII ---
static const char *SPRITE_PLAYER = "_^_";
// On définit 3 formes différentes pour bien les distinguer
static const char *SPRITE_A1 = "/^\\"; // 30 pts (Haut)
static const char *SPRITE_A2 = "/M\\"; // 20 pts (Milieu)
static const char *SPRITE_A3 = "/o\\"; // 10 pts (Bas)
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
        // --- DEFINITION DES COULEURS ---
        init_pair(1, COLOR_GREEN, COLOR_BLACK);   // Joueur / Alien 3 (10pts)
        init_pair(2, COLOR_RED, COLOR_BLACK);     // UFO / Danger
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Balles / UI
        init_pair(4, COLOR_WHITE, COLOR_BLUE);    // Cadres
        init_pair(5, COLOR_CYAN, COLOR_BLACK);    // Alien 2 (20pts) / Titres
        init_pair(6, COLOR_WHITE, COLOR_RED);     // Erreur
        init_pair(7, COLOR_MAGENTA, COLOR_BLACK); // Alien 1 (30pts)
    }
    return true;
}

static void ncurses_close(void)
{
    endwin();
}

// Fonction utilitaire Responsive
static void draw_centered_screen(int y_offset, const char *text, int pair_color)
{
    int screen_h, screen_w;
    getmaxyx(stdscr, screen_h, screen_w);

    int len = strlen(text);
    int x = (screen_w / 2) - (len / 2);
    int y = (screen_h / 2) + y_offset;

    attron(COLOR_PAIR(pair_color));
    mvprintw(y, x, "%s", text);
    attroff(COLOR_PAIR(pair_color));
}

static void ncurses_render(const GameModel *model)
{
    erase();

    // 1. RECUPERATION DE LA TAILLE DU TERMINAL
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    if (rows < GAME_HEIGHT + 2 || cols < GAME_WIDTH + 2)
    {
        attron(COLOR_PAIR(6));
        mvprintw(rows / 2, (cols - 20) / 2, "TERMINAL TROP PETIT");
        attroff(COLOR_PAIR(6));
        refresh();
        return;
    }

    // Calcul des offsets pour centrer le jeu
    int off_y = (rows - (GAME_HEIGHT + 2)) / 2;
    int off_x = (cols - (GAME_WIDTH + 2)) / 2;

    // DESSIN DU CADRE
    attron(COLOR_PAIR(4));
    mvhline(off_y, off_x, '#', GAME_WIDTH + 2);
    mvhline(off_y + GAME_HEIGHT + 1, off_x, '#', GAME_WIDTH + 2);
    mvvline(off_y, off_x, '#', GAME_HEIGHT + 2);
    mvvline(off_y, off_x + GAME_WIDTH + 1, '#', GAME_HEIGHT + 2);
    attroff(COLOR_PAIR(4));

    // =================================================
    // MENUS (Responsive)
    // =================================================

    if (model->state == STATE_MENU)
    {
        draw_centered_screen(-6, "=== SPACE INVADERS ===", 5);
        const char *options[] = {"JOUER", "TUTORIEL", "CHARGER", "QUITTER"};
        for (int i = 0; i < 4; i++)
        {
            char buf[64];
            int color = (i == model->menu_selection) ? 1 : 3;
            if (i == model->menu_selection)
            {
                attron(A_REVERSE);
                snprintf(buf, sizeof(buf), "> %s <", options[i]);
            }
            else
                snprintf(buf, sizeof(buf), "  %s  ", options[i]);
            draw_centered_screen(-2 + (i * 2), buf, color);
            if (i == model->menu_selection)
                attroff(A_REVERSE);
        }
    }
    else if (model->state == STATE_TUTORIAL)
    {
        // --- CORRECTION ET COULEURS ICI ---
        draw_centered_screen(-8, "COMMENT JOUER ?", 5);
        draw_centered_screen(-5, "Fleches : Se Deplacer", 3);
        draw_centered_screen(-3, "Espace  : Tirer", 3);

        // On affiche les vrais sprites avec leurs couleurs
        draw_centered_screen(0, "/^\\ (Alien 1) = 30 pts", 7); // Magenta
        draw_centered_screen(2, "/M\\ (Alien 2) = 20 pts", 5); // Cyan
        draw_centered_screen(4, "/o\\ (Alien 3) = 10 pts", 1); // Vert

        draw_centered_screen(6, "UFO ???       = MYSTERE", 2); // Rouge

        draw_centered_screen(10, "Appuyez sur une touche...", 4);
    }
    else if (model->state == STATE_SAVE_SELECT)
    {
        draw_centered_screen(-8, "CHOISIR EMPLACEMENT", 5);

        char buf[64];
        if (model->menu_selection == 0)
        {
            attron(A_REVERSE);
            snprintf(buf, sizeof(buf), "> [ CREER NOUVELLE ] <");
            draw_centered_screen(-5, buf, 1);
            attroff(A_REVERSE);
        }
        else
            draw_centered_screen(-5, "[ CREER NOUVELLE ]", 3);

        for (int i = 0; i < model->save_file_count; i++)
        {
            if (i + 1 == model->menu_selection)
            {
                attron(A_REVERSE);
                snprintf(buf, sizeof(buf), "> %s <", model->save_files[i]);
                draw_centered_screen(-2 + i, buf, 1);
                attroff(A_REVERSE);
            }
            else
            {
                snprintf(buf, sizeof(buf), "  %s  ", model->save_files[i]);
                draw_centered_screen(-2 + i, buf, 3);
            }
        }
        draw_centered_screen(10, "(Echap pour Annuler)", 4);
    }
    else if (model->state == STATE_LOAD_MENU)
    {
        draw_centered_screen(-8, "CHARGER UNE PARTIE", 5);
        if (model->save_file_count == 0)
            draw_centered_screen(-4, "Aucun fichier !", 2);

        for (int i = 0; i < model->save_file_count; i++)
        {
            char buf[64];
            if (i == model->menu_selection)
            {
                attron(A_REVERSE);
                snprintf(buf, sizeof(buf), "> %s <", model->save_files[i]);
                draw_centered_screen(-4 + i, buf, 1);
                attroff(A_REVERSE);
            }
            else
            {
                snprintf(buf, sizeof(buf), "  %s  ", model->save_files[i]);
                draw_centered_screen(-4 + i, buf, 3);
            }
        }
    }
    else if (model->state == STATE_SAVE_INPUT)
    {
        draw_centered_screen(-2, "NOM DE LA SAUVEGARDE :", 5);
        char buf[128];
        snprintf(buf, sizeof(buf), "%s_", model->input_buffer);
        draw_centered_screen(0, buf, 1);
        draw_centered_screen(4, "(Entree: Valider | Echap: Annuler)", 3);
    }

    // =================================================
    // JEU (Avec couleurs des ennemis)
    // =================================================

    bool draw_game = (model->state == STATE_PLAYING || model->state == STATE_PAUSED ||
                      model->state == STATE_GAME_OVER || model->state == STATE_CONFIRM_QUIT ||
                      model->state == STATE_OVERWRITE_CONFIRM);

    if (draw_game)
    {
        mvprintw(off_y - 1, off_x, " SCORE: %d | VIES: %d | NIV: %d ", model->score, model->lives, model->level);

        // JOUEUR
        if (model->player.active)
        {
            attron(COLOR_PAIR(1));
            mvprintw(off_y + (int)model->player.y + 1, off_x + (int)model->player.x + 1, "%s", SPRITE_PLAYER);
            attroff(COLOR_PAIR(1));
        }

        // ENNEMIS (Gestion des couleurs par rangée)
        // On suppose que les ennemis sont stockés dans l'ordre (Haut vers Bas)
        // 0-10 = Rangée 1 (30pts) | 11-32 = Rangée 2&3 (20pts) | 33+ = Rangée 4&5 (10pts)
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (model->enemies[i].active)
            {

                int color = 2; // Rouge par défaut
                const char *sprite = SPRITE_A2;

                if (i < 11)
                {
                    color = 7;
                    sprite = SPRITE_A1; // Magenta (30pts)
                }
                else if (i < 33)
                {
                    color = 5;
                    sprite = SPRITE_A2; // Cyan (20pts)
                }
                else
                {
                    color = 1;
                    sprite = SPRITE_A3; // Vert (10pts)
                }

                attron(COLOR_PAIR(color));
                mvprintw(off_y + (int)model->enemies[i].y + 1, off_x + (int)model->enemies[i].x + 1, "%s", sprite);
                attroff(COLOR_PAIR(color));
            }
        }

        // BALLES
        attron(COLOR_PAIR(3));
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (model->bullets[i].active)
            {
                mvaddch(off_y + (int)model->bullets[i].y + 1, off_x + (int)model->bullets[i].x + 1, CHAR_BULLET);
            }
        }
        attroff(COLOR_PAIR(3));

        // POPUPS
        if (model->state != STATE_PLAYING)
        {
            const char *title = "";
            const char **opts = NULL;
            int opt_count = 0;
            static const char *opts_pause[] = {"SAUVEGARDER", "RECOMMENCER", "QUITTER"};
            static const char *opts_go[] = {"SAUVEGARDER", "REJOUER", "QUITTER"};
            static const char *opts_quit[] = {"OUI", "NON", "SAUVEGARDER ET QUITTER"};
            static const char *opts_over[] = {"OUI (ECRASER)", "NON (CHANGER NOM)"};

            if (model->state == STATE_PAUSED)
            {
                title = "PAUSE";
                opts = opts_pause;
                opt_count = 3;
            }
            else if (model->state == STATE_GAME_OVER)
            {
                title = "GAME OVER";
                opts = opts_go;
                opt_count = 3;
            }
            else if (model->state == STATE_CONFIRM_QUIT)
            {
                title = "QUITTER ?";
                opts = opts_quit;
                opt_count = 3;
            }
            else if (model->state == STATE_OVERWRITE_CONFIRM)
            {
                title = "ECRASER ?";
                opts = opts_over;
                opt_count = 2;
            }

            // Effacer zone popup (fond noir)
            int popup_h = 10, popup_w = 40;
            int py = (rows - popup_h) / 2;
            int px = (cols - popup_w) / 2;
            for (int k = 0; k < popup_h; k++)
                mvhline(py + k, px, ' ', popup_w);

            attron(COLOR_PAIR(4));
            mvhline(py, px, '-', popup_w);
            mvhline(py + popup_h, px, '-', popup_w);
            mvvline(py, px, '|', popup_h);
            mvvline(py, px + popup_w, '|', popup_h);
            attroff(COLOR_PAIR(4));

            draw_centered_screen(-2, title, 5);

            for (int i = 0; i < opt_count; i++)
            {
                char buf[64];
                if (i == model->menu_selection)
                {
                    attron(A_REVERSE);
                    snprintf(buf, sizeof(buf), "> %s <", opts[i]);
                    draw_centered_screen(0 + (i * 2), buf, 1);
                    attroff(A_REVERSE);
                }
                else
                {
                    snprintf(buf, sizeof(buf), "  %s  ", opts[i]);
                    draw_centered_screen(0 + (i * 2), buf, 3);
                }
            }
        }
    }
    refresh();
}

static GameCommand ncurses_get_input(GameModel *model)
{
    int ch = getch();
    if (ch == KEY_RESIZE)
        return CMD_NONE;

    if (model->state == STATE_SAVE_INPUT)
    {
        if (ch == '\n' || ch == KEY_ENTER)
            return CMD_RETURN;
        if (ch == 27)
            return CMD_PAUSE;
        if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b')
        {
            int len = strlen(model->input_buffer);
            if (len > 0)
                model->input_buffer[len - 1] = '\0';
        }
        else if (strlen(model->input_buffer) < MAX_FILENAME_LEN - 1)
        {
            if (isalnum(ch) || ch == '-' || ch == '_')
            {
                int len = strlen(model->input_buffer);
                model->input_buffer[len] = (char)ch;
                model->input_buffer[len + 1] = '\0';
            }
        }
        return CMD_NONE;
    }

    switch (ch)
    {
    case KEY_LEFT:
        return (model->state == STATE_PLAYING) ? CMD_MOVE_LEFT : CMD_LEFT;
    case KEY_RIGHT:
        return (model->state == STATE_PLAYING) ? CMD_MOVE_RIGHT : CMD_RIGHT;
    case KEY_UP:
        return CMD_UP;
    case KEY_DOWN:
        return CMD_DOWN;
    case ' ':
        return CMD_SHOOT;
    case '\n':
        return CMD_RETURN;
    case 'p':
        return CMD_PAUSE;
    case 27:
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