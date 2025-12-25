#include "view_ncurses.h"
#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <math.h> // Nécessaire pour round() si besoin, ou cast int simple

// --- 1. SPRITES ---
static const char *SPRITE_PLAYER = "_^_";
static const char *SPRITE_A1 = "/^\\";
static const char *SPRITE_A2 = "/M\\";
static const char *SPRITE_A3 = "/o\\";
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
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
        init_pair(3, COLOR_YELLOW, COLOR_BLACK);
        init_pair(4, COLOR_WHITE, COLOR_BLUE);
        init_pair(5, COLOR_CYAN, COLOR_BLACK);
        init_pair(6, COLOR_WHITE, COLOR_RED);
        init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    }
    return true;
}

static void ncurses_close(void)
{
    endwin();
}

// Fonction utilitaire pour centrer du texte (reste inchangée car elle est déjà responsive via getmaxyx)
static void draw_centered_screen(int y_offset, const char *text, int pair_color)
{
    int screen_h, screen_w;
    getmaxyx(stdscr, screen_h, screen_w);
    int len = strlen(text);
    int x = (screen_w / 2) - (len / 2);
    int y = (screen_h / 2) + y_offset;

    // Sécurité pour ne pas écrire hors écran
    if (x < 0)
        x = 0;
    if (y < 0 || y >= screen_h)
        return;

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

    // On accepte des terminaux plus petits, mais on garde une limite minimale absolue pour que ce soit jouable
    if (rows < 10 || cols < 20)
    {
        mvprintw(0, 0, "TROP PETIT");
        refresh();
        return;
    }

    // --- RESPONSIVE : CALCUL DES RATIOS ---
    // On utilise (cols - 2) et (rows - 2) pour laisser la place au cadre
    // On convertit en float pour la précision, puis en int pour l'affichage
    float scale_x = (float)(cols - 2) / (float)GAME_WIDTH;
    float scale_y = (float)(rows - 2) / (float)GAME_HEIGHT;

    // DESSIN DU CADRE (S'adapte à la fenêtre entière)
    attron(COLOR_PAIR(4));
    mvhline(0, 0, '#', cols);        // Haut
    mvhline(rows - 1, 0, '#', cols); // Bas
    mvvline(0, 0, '#', rows);        // Gauche
    mvvline(0, cols - 1, '#', rows); // Droite
    attroff(COLOR_PAIR(4));

    // =================================================
    // MENUS (Centrés, pas besoin de scaling complexe)
    // =================================================
    // Note : J'ai gardé votre logique de menu, elle fonctionne bien car draw_centered_screen est dynamique.

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
            {
                snprintf(buf, sizeof(buf), "  %s  ", options[i]);
            }
            draw_centered_screen(-2 + (i * 2), buf, color);
            if (i == model->menu_selection)
                attroff(A_REVERSE);
        }
    }
    // ... (Je passe les autres menus comme TUTORIAL/SAVE/LOAD car ils utilisent draw_centered_screen qui est OK) ...
    else if (model->state == STATE_TUTORIAL)
    {
        draw_centered_screen(-5, "TUTORIEL (Voir Readme)", 5);
        draw_centered_screen(0, "Appuyez sur Entree...", 4);
    }
    // Ajoutez ici vos blocs ELSE IF pour SAVE/LOAD comme dans votre code original
    // Ils fonctionneront car ils utilisent draw_centered_screen.

    // =================================================
    // JEU (RESPONSIVE SCALING)
    // =================================================

    bool draw_game = (model->state == STATE_PLAYING || model->state == STATE_PAUSED ||
                      model->state == STATE_GAME_OVER || model->state == STATE_CONFIRM_QUIT ||
                      model->state == STATE_OVERWRITE_CONFIRM);

    if (draw_game)
    {
        // HUD en haut
        mvprintw(1, 2, "SC:%d | LVL:%d", model->score, model->level);

        // --- JOUEUR ---
        if (model->player.active)
        {
            // Conversion coordonnées Jeu -> Terminal
            // Le "+1" sert à décaler après le cadre de gauche/haut
            int screen_x = (int)(model->player.x * scale_x) + 1;
            int screen_y = (int)(model->player.y * scale_y) + 1;

            // Sécurité bornes
            if (screen_x >= cols - 4)
                screen_x = cols - 4;
            if (screen_y >= rows - 2)
                screen_y = rows - 2;

            attron(COLOR_PAIR(1));
            mvprintw(screen_y, screen_x, "%s", SPRITE_PLAYER);
            attroff(COLOR_PAIR(1));
        }

        // --- ENNEMIS ---
        for (int i = 0; i < MAX_ENEMIES; i++)
        {
            if (model->enemies[i].active)
            {
                int color = 2;
                const char *sprite = SPRITE_A2;

                if (i < 11)
                {
                    color = 7;
                    sprite = SPRITE_A1;
                }
                else if (i < 33)
                {
                    color = 5;
                    sprite = SPRITE_A2;
                }
                else
                {
                    color = 1;
                    sprite = SPRITE_A3;
                }

                // --- SCALING RESPONSIVE ---
                int ex = (int)(model->enemies[i].x * scale_x) + 1;
                int ey = (int)(model->enemies[i].y * scale_y) + 1;

                // On s'assure de ne pas dessiner sur le cadre
                if (ex > 0 && ex < cols - 3 && ey > 0 && ey < rows - 1)
                {
                    attron(COLOR_PAIR(color));
                    mvprintw(ey, ex, "%s", sprite);
                    attroff(COLOR_PAIR(color));
                }
            }
        }

        // --- BALLES ---
        attron(COLOR_PAIR(3));
        for (int i = 0; i < MAX_BULLETS; i++)
        {
            if (model->bullets[i].active)
            {
                int bx = (int)(model->bullets[i].x * scale_x) + 1;
                int by = (int)(model->bullets[i].y * scale_y) + 1;

                if (bx > 0 && bx < cols - 1 && by > 0 && by < rows - 1)
                {
                    mvaddch(by, bx, CHAR_BULLET);
                }
            }
        }
        attroff(COLOR_PAIR(3));

        // --- POPUPS (Centrés via calcul relatif) ---
        if (model->state != STATE_PLAYING)
        {
            // On dessine une boite au milieu, quelle que soit la taille
            int box_w = (cols > 40) ? 40 : cols - 4; // Largeur max 40 ou adaptée
            int box_h = 10;
            int start_y = (rows - box_h) / 2;
            int start_x = (cols - box_w) / 2;

            // Effacer la zone
            for (int k = 0; k < box_h; k++)
                mvhline(start_y + k, start_x, ' ', box_w);

            // Cadre Popup
            attron(COLOR_PAIR(4));
            mvhline(start_y, start_x, '-', box_w);
            mvhline(start_y + box_h, start_x, '-', box_w);
            mvvline(start_y, start_x, '|', box_h);
            mvvline(start_y, start_x + box_w, '|', box_h);
            attroff(COLOR_PAIR(4));

            // Titre simple pour debug
            if (model->state == STATE_PAUSED)
                draw_centered_screen(-1, "PAUSE", 5);
            else if (model->state == STATE_GAME_OVER)
                draw_centered_screen(-1, "GAME OVER", 2);
            else
                draw_centered_screen(-1, "CONFIRMATION", 5);
        }
    }
    refresh();
}

static GameCommand ncurses_get_input(GameModel *model)
{
    int ch = getch();
    if (ch == KEY_RESIZE)
        return CMD_NONE; // Important pour ne pas crash au redimensionnement

    // ... (Garder votre logique d'input existante ici, elle est parfaite) ...
    // Je remets juste le switch standard pour l'exemple
    switch (ch)
    {
    case KEY_LEFT:
        return CMD_LEFT; // Ou CMD_MOVE_LEFT selon l'état
    case KEY_RIGHT:
        return CMD_RIGHT;
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
        return CMD_PAUSE; // Echap
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