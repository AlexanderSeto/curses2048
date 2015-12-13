#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

/*
 * Will define the size of the game board as SIZE rows and SIZE columns. As it
 * stands, the display funcions work best with 4.
 */
int SIZE = 4;

struct game_state {
    int **grid;
    long total_score;
    long score_last_move;
    int tiles_in_play;
    bool game_over;
};

struct tile {
    int x;
    int y;
};

struct game_state *initGameState();
void draw(struct game_state *);
void moveGrid(struct game_state *, int);
void addRandomTile(struct game_state *);
void getDirectionVector(int vector[], int dir);
void buildTraversals(int *traversals[2], int *vector);
void findFarthestPosition(struct game_state *, int[4], int, int, int[2]);
bool inRange(int);
bool tileMatchesAvailible(struct game_state *);
void initCurses();
bool parseArgs(int, char *[]);

int main(int argc, char *argv[]) {
    if (argc > 1) {
        if(!parseArgs(argc, argv))
            return -1;
    }
    struct game_state *game = initGameState();
    srand((unsigned)time(NULL));

    addRandomTile(game);
    addRandomTile(game);

    initCurses();

    draw(game);
    draw(game);

    int c;
    while ((c = getch()) != KEY_F(1) && game->game_over != true) {
        moveGrid(game, c - 258);
        draw(game);
    }
    mvprintw(0, 10, "Sorry, you lose! Score: %d", game->total_score);
    getch();
    endwin();
}

/*
 * Initialize game state into a locally allocated game state pointer and return.
 */
struct game_state *initGameState() {
    struct game_state *game =
        (struct game_state *)malloc(sizeof(struct game_state));
    game->grid = (int **)malloc(SIZE * sizeof(int *));
    for (int i = 0; i < SIZE; i++) {
        game->grid[i] = (int *)malloc(SIZE * sizeof(int));
        for (int j = 0; j < SIZE; j++) game->grid[i][j] = 0;
    }
    game->total_score = 0;
    game->score_last_move = 0;
    game->tiles_in_play = 0;
    game->game_over = false;
    return game;
}

/*
 * Find a random location in the grid, incremenent tiles_in_play.
 */
void addRandomTile(struct game_state *game) {
    bool inserted = false;
    while (!inserted) {
        int i = rand() % SIZE;
        int j = rand() % SIZE;
        if (game->grid[i][j] == 0) {
            game->grid[i][j] = 2;
            inserted = true;
        }
    }
    game->tiles_in_play++;
}

/*
 * Draw the SIZE by SIZE grid onto the screen via ncurses Each grid element is
 * its own ncurses WINDOW so that custom colors can be assigned to each value.
 */
void draw(struct game_state *game) {
    WINDOW *local_window[SIZE][SIZE];
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int y, x;
            getmaxyx(stdscr, y, x);
            local_window[i][j] = newwin(4, 6, (y - SIZE * 4) / 2 + i * 4,
                                        (x - SIZE * 6) / 2 + j * 6);
            // I could remove box and invert colors instead.
            box(local_window[i][j], 0, 0);
            char num[4];
            sprintf(num, "%d", game->grid[i][j]);
            wbkgd(local_window[i][j], COLOR_PAIR(game->grid[i][j] % 256));
            mvwprintw(local_window[i][j], 2, 2, num);
            wrefresh(local_window[i][j]);
        }
    }
    refresh();
}

/*
 * Shift the grid tiles in the direction specified. Determine the "vector" for
 * the shift, then build traversal arrays based on those vectors. Iterate
 * through the grid according to the travesal arrays and shift accordingly.
 * After moving, update the score and check for lose conditions.
 */
void moveGrid(struct game_state *game, int dir) {
    mvaddch(0, 0, dir + '0');
    bool moved = false;
    int vector[2] = {0, 0};
    getDirectionVector(vector, dir);
    int *traversals[2];
    buildTraversals(traversals, vector);
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int x = traversals[0][i];
            int y = traversals[1][j];
            int tile = game->grid[x][y];

            if (tile != 0) {
                int position[4];
                findFarthestPosition(game, position, x, y, vector);
                int f_x = position[0];
                int f_y = position[1];
                int next_x = position[2];
                int next_y = position[3];
                // ONE?
                if (inRange(next_x) && inRange(next_y) &&
                    game->grid[next_x][next_y] == game->grid[x][y]) {
                    game->grid[next_x][next_y] *= 2;
                    game->grid[x][y] = 0;
                    moved = true;
                    game->score_last_move = game->grid[next_x][next_y];
                    game->tiles_in_play--;
                } else if (inRange(f_x) && inRange(f_y)) {
                    game->grid[f_x][f_y] = game->grid[x][y];
                    if (f_x != x || f_y != y) game->grid[x][y] = 0;
                    moved = true;
                }
            }
        }
    }
    if (moved) {
        addRandomTile(game);
        game->total_score += game->score_last_move;
        mvprintw(0, 10, "Score %d = (+ %d)", game->total_score,
                 game->score_last_move);

        if ((SIZE * SIZE - game->tiles_in_play) <= 0 &&
            !tileMatchesAvailible(game)) {
            game->game_over = true;
        }
    }
}

/*
 * Determine the direction vector for a given direction. A direction vector is
 * just an x and a y value that correspond to the proveded direction.
 * 0: ( 0,  1) down         1: ( 0, -1) up
 * 2: (-1,  0) left         3: ( 1,  0) right
 */
void getDirectionVector(int vector[2], int dir) {
    switch (dir) {
        case 0:  // down
            vector[0] = 1;
            break;
        case 1:  // up
            vector[0] = -1;
            break;
        case 2:  // left
            vector[1] = -1;
            break;
        case 3:  // right
            vector[1] = 1;
            break;
    }
}

void buildTraversals(int *traversals[2], int *vector) {
    traversals[0] = (int *)malloc(SIZE * sizeof(int));
    traversals[1] = (int *)malloc(SIZE * sizeof(int));
    for (int i = 0; i < SIZE; i++) {
        if (vector[0] == 1)
            traversals[0][i] = SIZE - 1 - i;
        else
            traversals[0][i] = i;
        if (vector[1] == 1)
            traversals[1][i] = SIZE - 1 - i;
        else
            traversals[1][i] = i;
    }
}

void findFarthestPosition(struct game_state *game, int position[4], int x,
                            int y, int vector[2]) {
    int prev_x, prev_y;
    do {
        prev_x = x;
        prev_y = y;
        x = prev_x + vector[0];
        y = prev_y + vector[1];
    } while (inRange(x) && inRange(y) && game->grid[x][y] == 0);

    position[0] = prev_x;
    position[1] = prev_y;
    position[2] = x;
    position[3] = y;
}

bool inRange(int i) { return i >= 0 && i < SIZE; }

bool tileMatchesAvailible(struct game_state *game) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int tile = game->grid[i][j];
            if (tile) {
                for (int dir = 0; dir < 4; dir++) {
                    int vector[2];
                    getDirectionVector(vector, dir);
                    if (inRange(i + vector[0]) && inRange(i + vector[1])) {
                        int new_tile = game->grid[i + vector[0]][j + vector[1]];
                        if (new_tile == tile) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

void initCurses() {
    initscr();
    start_color();
    cbreak();
    noecho();

    keypad(stdscr, TRUE);

    init_pair(2, 179, 000);
    init_pair(4, 178, 000);
    init_pair(8, 172, 000);
    init_pair(16, 208, 000);
    init_pair(32, 160, 000);
    init_pair(64, 196, 000);
    init_pair(128, 220, 000);
    init_pair(256, 184, 000);
}

bool parseArgs(int argc, char *argv[]) {
    int c;
    while ((c = getopt(argc, argv, "s:")) != -1) {
        switch (c) {
            case 'h':
                /* TODO: Print help info */
                break;
            case 'c':
                /* TODO: Color Options as a parameter. */
                break;
            case 's':
                if(!atoi(optarg)) {
                    fprintf(stderr, "%s: Invalid argument parameter -- %s\n", argv[0], optarg);
                    return false;
                }

                SIZE = atoi(optarg);

                break;
            case '?':
                if (optopt == 's')
                    fprintf(stderr, "Option -%c requires an argument.\n",
                            optopt);
                 else if (isprint(optopt))
                     fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                 else
                     fprintf(stderr, "Unknown option character `\\x%x'.\n",
                             optopt);
                return false;
            default:
                abort();
        }
    }
    return true;
}
