/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#define _XOPEN_SOURCE 500
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "logic.h"

uint8_t scheme = 0;
uint8_t *scheme_array[32];

void getColor(uint8_t value, char *color, size_t length) {
    uint8_t original[] = {8,   255, 1,   255, 2,   255, 3,   255, 4,   255, 5,
                          255, 6,   255, 7,   255, 9,   0,   10,  0,   11,  0,
                          12,  0,   13,  0,   14,  0,   255, 0,   255, 0};
    uint8_t blackwhite[] = {232, 255, 234, 255, 236, 255, 238, 255,
                            240, 255, 242, 255, 244, 255, 246, 0,
                            248, 0,   249, 0,   250, 0,   251, 0,
                            252, 0,   253, 0,   254, 0,   255, 0};
    uint8_t bluered[] = {235, 255, 63,  255, 57,  255, 93,  255, 129, 255, 165,
                         255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255,
                         196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
    uint8_t *schemes[] = {original, blackwhite, bluered};
    uint8_t *background = schemes[scheme] + 0;
    uint8_t *foreground = schemes[scheme] + 1;
    if (value > 0)
        while (value--) {
            if (background + 2 < schemes[scheme] + sizeof(original)) {
                background += 2;
                foreground += 2;
            }
        }
    snprintf(color, length, "\033[38;5;%d;48;5;%dm", *foreground, *background);
}

void drawBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t x, y;
    char color[40], reset[] = "\033[m";
    printf("\033[H");

    printf("2048.c %17d pts\n\n", score);

    for (y = 0; y < SIZE; y++) {
        for (x = 0; x < SIZE; x++) {
            getColor(board[x][y], color, 40);
            printf("%s", color);
            printf("       ");
            printf("%s", reset);
        }
        printf("\n");
        for (x = 0; x < SIZE; x++) {
            getColor(board[x][y], color, 40);
            printf("%s", color);
            if (board[x][y] != 0) {
                char s[8];
                snprintf(s, 8, "%u", (uint32_t)1 << board[x][y]);
                uint8_t t = 7 - strlen(s);
                printf("%*s%s%*s", t - t / 2, "", s, t / 2, "");
            } else {
                printf("   ·   ");
            }
            printf("%s", reset);
        }
        printf("\n");
        for (x = 0; x < SIZE; x++) {
            getColor(board[x][y], color, 40);
            printf("%s", color);
            printf("       ");
            printf("%s", reset);
        }
        printf("\n");
    }
    printf("\n");
    printf("        ←,↑,→,↓ or q        \n");
    printf("\033[A");  // one line up
}



bool findPairDown(uint8_t board[SIZE][SIZE]) {
    bool success = false;
    uint8_t x, y;
    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE - 1; y++) {
            if (board[x][y] == board[x][y + 1]) return true;
        }
    }
    return success;
}

uint8_t countEmpty(uint8_t board[SIZE][SIZE]) {
    uint8_t x, y;
    uint8_t count = 0;
    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE; y++) {
            if (board[x][y] == 0) {
                count++;
            }
        }
    }
    return count;
}

bool gameEnded(uint8_t board[SIZE][SIZE]) {
    bool ended = true;
    if (countEmpty(board) > 0) return false;
    if (findPairDown(board)) return false;
    rotateBoard(board);
    if (findPairDown(board)) ended = false;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return ended;
}

void addRandom(uint8_t board[SIZE][SIZE]) {
    static bool initialized = false;
    uint8_t x, y;
    uint8_t r, len = 0;
    uint8_t n, list[SIZE * SIZE][2];

    if (!initialized) {
        srand(time(NULL));
        initialized = true;
    }

    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE; y++) {
            if (board[x][y] == 0) {
                list[len][0] = x;
                list[len][1] = y;
                len++;
            }
        }
    }

    if (len > 0) {
        r = rand() % len;
        x = list[r][0];
        y = list[r][1];
        n = (rand() % 10) / 9 + 1;
        board[x][y] = n;
    }
}

void initBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t x, y;
    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE; y++) {
            board[x][y] = 0;
        }
    }

    addRandom(board);
    addRandom(board);
    drawBoard(board);
    score = 0;
}

void setBufferedInput(bool enable) {
    static bool enabled = true;
    static struct termios old;
    struct termios new;

    if (enable && !enabled) {
        // restore the former settings
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        // set the new state
        enabled = true;
    } else if (!enable && enabled) {
        // get the terminal settings for standard input
        tcgetattr(STDIN_FILENO, &new);
        // we want to keep the old setting to restore them at the end
        old = new;
        // disable canonical mode (buffered i/o) and local echo
        new.c_lflag &= (~ICANON & ~ECHO);
        // set the new settings immediately
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
        // set the new state
        enabled = false;
    }
}

int test() {
    uint8_t array[SIZE];
    // these are exponents with base 2 (1=2 2=4 3=8)
    uint8_t data[] = {0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 2, 0, 0, 0, 0, 1,
                      0, 1, 2, 0, 0, 0, 1, 0, 0, 1, 2, 0, 0, 0, 1, 0, 1, 0,
                      2, 0, 0, 0, 1, 1, 1, 0, 2, 1, 0, 0, 1, 0, 1, 1, 2, 1,
                      0, 0, 1, 1, 0, 1, 2, 1, 0, 0, 1, 1, 1, 1, 2, 2, 0, 0,
                      2, 2, 1, 1, 3, 2, 0, 0, 1, 1, 2, 2, 2, 3, 0, 0, 3, 0,
                      1, 1, 3, 2, 0, 0, 2, 0, 1, 1, 2, 2, 0, 0};
    uint8_t *in, *out;
    uint8_t t, tests;
    uint8_t i;
    bool success = true;

    tests = (sizeof(data) / sizeof(data[0])) / (2 * SIZE);
    for (t = 0; t < tests; t++) {
        in = data + t * 2 * SIZE;
        out = in + SIZE;
        for (i = 0; i < SIZE; i++) {
            array[i] = in[i];
        }
        slideArray(array);
        for (i = 0; i < SIZE; i++) {
            if (array[i] != out[i]) {
                success = false;
            }
        }
        if (success == false) {
            for (i = 0; i < SIZE; i++) {
                printf("%d ", in[i]);
            }
            printf("=> ");
            for (i = 0; i < SIZE; i++) {
                printf("%d ", array[i]);
            }
            printf("expected ");
            for (i = 0; i < SIZE; i++) {
                printf("%d ", in[i]);
            }
            printf("=> ");
            for (i = 0; i < SIZE; i++) {
                printf("%d ", out[i]);
            }
            printf("\n");
            break;
        }
    }
    if (success) {
        printf("All %u tests executed successfully\n", tests);
    }
    return !success;
}

void signal_callback_handler(int signum) {
    printf("         TERMINATED         \n");
    setBufferedInput(true);
    printf("\033[?25h\033[m");
    exit(signum);
}

void key() {
    printf("PRESS A KEY TO CONTINUE \n");
    int ch1 = getchar();
    printf("Character: %d\n", ch1);
}

int main(int argc, char *argv[]) {
    uint8_t board[SIZE][SIZE];
    char c;
    bool success;

    if (argc == 2) {
        if (strcmp(argv[1], "test") == 0) {
            return test();
        } else if (strcmp(argv[1], "blackwhite") == 0) {
            scheme = 1;
        } else if (strcmp(argv[1], "bluered") == 0) {
            scheme = 2;
        } else if (strcmp(argv[1], "key") == 0) {
            key();
            return 0;
        }
    }

    printf("\033[?25l\033[2J");

    // register signal handler for when ctrl-c is pressed
    signal(SIGINT, signal_callback_handler);

    initBoard(board);
    setBufferedInput(false);
    while (true) {
        c = getchar();
        if (c == -1) {  // TODO: maybe replace this -1 with a pre-defined
                        // constant(if it's in one of header files)
            puts("\nError! Cannot read keyboard input!");
            break;
        }
        switch (c) {
            case 97:   // 'a' key
            case 104:  // 'h' key
            case 68:   // left arrow
                success = moveLeft(board);
                break;
            case 100:  // 'd' key
            case 108:  // 'l' key
            case 67:   // right arrow
                success = moveRight(board);
                break;
            case 119:  // 'w' key
            case 107:  // 'k' key
            case 65:   // up arrow
                success = moveUp(board);
                break;
            case 115:  // 's' key
            case 106:  // 'j' key
            case 66:   // down arrow
                success = moveDown(board);
                break;
            default:
                success = false;
        }
        if (success) {
            drawBoard(board);
            usleep(150000);
            addRandom(board);
            drawBoard(board);
            if (gameEnded(board)) {
                printf("         GAME OVER          \n");
                break;
            }
            continue;
        }

        if (c == 12) {
            // printf("\033[?25h\033[m");
            printf("\e[1;1H\e[2J");
            drawBoard(board);
        }

        if (c == 'q') {
            printf("        QUIT? (y/n)         \n");
            c = getchar();
            if (c == 'y') {
                break;
            }
            drawBoard(board);
        }
        if (c == 'r') {
            printf("       RESTART? (y/n)       \n");
            c = getchar();
            if (c == 'y') {
                initBoard(board);
            }
            drawBoard(board);
        }
    }
    setBufferedInput(true);

    printf("\033[?25h\033[m");

    return EXIT_SUCCESS;
}
