#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define SIZE 4
// const long values[] = {
//    0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
// };

struct game_state {
    int grid[SIZE][SIZE];
    bool have_moved;
    long total_score;
    long score_last_move;
    int blocks_in_play;
};

struct tile {
    int x;
    int y;
};

struct game_state *init_game_state();
void draw(struct game_state *);
void slide_array(struct game_state *,int);
void insert_rand(struct game_state *);
void get_direction_vector(int vector[], int dir);
void build_traversals(int traversals[2][SIZE], int *vector);
void find_farthest_position(struct game_state *game, int postion[4], int x, int y, int vector[2]);
bool in_range(int i);

int main(int argc, char *argv[]) {

    struct game_state *game = init_game_state();
    srand((unsigned) time(NULL));

    insert_rand(game);
    insert_rand(game);

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

    draw(game);
    draw(game);

    int c;
    while((c = getch()) != KEY_F(1)) {
        slide_array(game, c - 258);
        //insert_rand(game);
        draw(game);
    }
    endwin();


}
struct game_state *init_game_state() {
    struct game_state *game = (struct game_state*) malloc(sizeof(struct game_state));
    for (int i = 0; i < SIZE; i++)
        for(int j = 0; j < SIZE; j++)
            game->grid[i][j] = 0;
    game->total_score = 0;
    game->score_last_move = 0;
    game->blocks_in_play = 0;
    return game;
}

void insert_rand(struct game_state *game) {
    bool inserted = false;
    while(!inserted) {
        int i = rand()%SIZE;
        int j = rand()%SIZE;
        if (game->grid[i][j] == 0) {
            game->grid[i][j] = 2;
            inserted = true;
        }

    }
    game->blocks_in_play++;
}

void draw(struct game_state *game) {
    WINDOW *local_window[SIZE][SIZE];
    for(int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            local_window[i][j] = newwin(4, 6, 4+i*4, 10 +j*6);
            box(local_window[i][j], 0, 0); // I could remove box and invert colors.
            char num[4];
            sprintf(num, "%d", game->grid[i][j]);
            wbkgd(local_window[i][j], COLOR_PAIR(game->grid[i][j] % 256));
            mvwprintw(local_window[i][j], 2,2, num);
            wrefresh(local_window[i][j]);
        }
    }
    refresh();
}
void slide_array (struct game_state *game, int dir) {
    mvaddch(0,0, dir + '0');
    bool moved = false;
    int vector[2] = {0, 0};
    get_direction_vector(vector, dir);
    int traversals[2][SIZE];
    build_traversals(traversals, vector);
    for(int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            int x = traversals[0][i];
            int y = traversals[1][j];
            refresh();
            int tile = game->grid[x][y];

            if(tile != 0) {
                int position[4];
                find_farthest_position(game, position, x, y, vector);
                int f_x = position[0];
                int f_y = position[1];
                int next_x = position[2];
                int next_y = position[3];
                // ONE?
                if(in_range(next_x) && in_range(next_y) && game->grid[next_x][next_y] == game->grid[x][y]) {
                    game->grid[next_x][next_y] *=2;
                    game->grid[x][y] = 0;
                    moved = true;
                    game->score_last_move = game->grid[next_x][next_y];
                }
                else if (in_range(f_x) && in_range(f_y)){
                    game->grid[f_x][f_y] = game->grid[x][y];
                    if(f_x != x || f_y != y)
                        game->grid[x][y] = 0;
                    moved = true;
                }

            }
        }
    }
    if(moved) {
        insert_rand(game);
        game->total_score += game->score_last_move;
        mvprintw(0, 10, "Score %d = (+ %d)", game->total_score, game->score_last_move);
    }

}

/*
 * 0 - 0 1 down
 * 1 - 0 -1 up
 * 2 - -1 0 left
 * 3 - 1 0 right
 */
void get_direction_vector(int vector[2], int dir) {
    switch (dir) {
        case 0: //down
            vector[0] = 1;
            break;
        case 1: //up
            vector[0] = -1;
            break;
        case 2:// left
            vector[1] = -1;
            break;
        case 3: //right
            vector[1] = 1;
            break;
        }

    //vector[0] = (2 - dir) % 2;
    //vector[1] = (dir - 1) % 2;

}

void build_traversals(int traversals[2][SIZE], int *vector) {
    //traversals[0] = (int *) malloc(sizeof(int)*SIZE);
    //traversals[1] = (int *) malloc(sizeof(int)*SIZE);

    for (int i = 0; i < SIZE; i++) {
        if(vector[0] == 1)
            traversals[0][i] = SIZE -1 - i;
        else
            traversals[0][i] = i;
        if(vector[1] == 1)
            traversals[1][i] = SIZE -1 -i;
        else
            traversals[1][i] = i;
    }
}

void find_farthest_position(struct game_state *game, int position[4], int x, int y, int vector[2]) {
    int prev_x, prev_y;
    do {
        prev_x = x; prev_y = y;
        x = prev_x + vector[0]; y = prev_y + vector[1];
    } while( in_range(x) && in_range(y) && game->grid[x][y] == 0);

    position[0] = prev_x;
    position[1] = prev_y;
    position[2] = x;
    position[3] = y;


}

bool in_range(int i) {
    return i >= 0 && i < SIZE;
}
