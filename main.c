#include <ncurses.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// const long values[] = {
//    0, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048
// };

struct game_state {
    int grid[4][4];
    bool have_moved;
    long total_score;
    long score_last_move;
    int blocks_in_play;
};

struct game_state *init_game_state();
void draw(struct game_state *);
void slide_array(struct game_state *,int);
void insert_rand(struct game_state *);

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

    init_pair(1, 179, 000);


    draw(game);
    draw(game);

    int c;
    while((c = getch()) != KEY_F(1)) {
        slide_array(game, c - 258);
        insert_rand(game);
        draw(game);
    }
    endwin();


}
struct game_state *init_game_state() {
    struct game_state *game = (struct game_state*) malloc(sizeof(struct game_state));
    for (int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++)
            game->grid[i][j] = 0;
    game->total_score = 0;
    game->score_last_move = 0;
    game->blocks_in_play = 0;
    return game;
}

void insert_rand(struct game_state *game) {
    bool inserted = false;
    while(!inserted) {
        int i = rand()%4;
        int j = rand()%4;
        if (game->grid[i][j] == 0) {
            game->grid[i][j] = 2;
            inserted = true;
        }

    }
    game->blocks_in_play++;
}

void draw(struct game_state *game) {
    static WINDOW *local_window[4][4];
    for(int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            local_window[i][j] = newwin(4, 6, 4+i*4, 10 +j*6);
            box(local_window[i][j], 0, 0); // I could remove box and invert colors.
            char num[4];
            sprintf(num, "%d", game->grid[i][j]);
            wbkgd(local_window[i][j], COLOR_PAIR(1) );
            mvwprintw(local_window[i][j], 2,2, num);
            wrefresh(local_window[i][j]);
            wbkgd(local_window[i][j], COLOR_PAIR(1));
            //mvprintw(4 + i*4,10 +j*6, num);
        }
    }
    refresh();
}
void slide_array (struct game_state *game, int dir) {
    mvaddch(0,0, dir + '0');
    switch(dir) {
        case 0: //down
        //game->grid[][] = 40;
            for (int j = 0; j < 4; j++) {
                for(int i = 0; i < 4; i++) {
                    if (i < 3 && game->grid[i+1][j] == 0) {
                        game->grid[i+1][j] = game->grid[i][j];
                        game->grid[i][j] = 0;
                    }
                    else if(i < 3 && game->grid[i+1][j] == game->grid[i][j]) {
                        game->grid[i+1][j] *=2;
                        game->grid[i][j] = 0;
                    }

                }
            }
            break;
        case 1: //up
            for (int j = 0; j < 4; j++) {
                for(int i = 3; i >=0; i--) {
                    if (i > 0 && game->grid[i-1][j] == 0) {
                        game->grid[i-1][j] = game->grid[i][j];
                        game->grid[i][j] = 0;
                    }
                    else if(i > 0 && game->grid[i-1][j] == game->grid[i][j]) {
                        game->grid[i-1][j] *=2;
                        game->grid[i][j] = 0;
                    }

                }
            }
            break;
        case 2: //left
            for (int i = 0; i < 4; i++) {
                for(int j = 3; j >= 0; j--) {
                    if (j > 0 && game->grid[i][j-1] == 0) {
                        game->grid[i][j-1] = game->grid[i][j];
                        game->grid[i][j] = 0;
                    }
                    else if(i > 0 && game->grid[i][j-1] == game->grid[i][j]) {
                        game->grid[i][j-1] *=2;
                        game->grid[i][j] = 0;
                    }

                }
            }
            break;
        case 3: //right
            for (int i = 0; i < 4; i++) {
                for(int j = 0; j < 4; j++) {
                    if (j < 3 && game->grid[i][j+1] == 0) {
                        game->grid[i][j+1] = game->grid[i][j];
                        game->grid[i][j] = 0;
                    }
                    else if(i < 3 && game->grid[i][j+1] == game->grid[i][j]) {
                        game->grid[i][j+1] *=2;
                        game->grid[i][j] = 0;
                    }

                }
            }

            break;
        default:
            break;
    }
}
