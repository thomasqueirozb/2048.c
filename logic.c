#include <stdbool.h>
#include <stdint.h>
#include "logic.h"


uint8_t findTarget(uint8_t array[SIZE], uint8_t x, uint8_t stop) {
    uint8_t t;
    // if the position is already on the first, don't evaluate
    if (x == 0) {
        return x;
    }
    for (t = x - 1;; t--) {
        if (array[t] != 0) {
            if (array[t] != array[x]) {
                // merge is not possible, take next position
                return t + 1;
            }
            return t;
        } else {
            // we should not slide further, return this one
            if (t == stop) {
                return t;
            }
        }
    }
    // we did not find a
    return x;
}

bool slideArray(uint8_t array[SIZE]) {
    bool success = false;
    uint8_t x, t, stop = 0;

    for (x = 0; x < SIZE; x++) {
        if (array[x] != 0) {
            t = findTarget(array, x, stop);
            // if target is not original position, then move or merge
            if (t != x) {
                // if target is zero, this is a move
                if (array[t] == 0) {
                    array[t] = array[x];
                } else if (array[t] == array[x]) {
                    // merge (increase power of two)
                    array[t]++;
                    // increase score
                    score += (uint32_t)1 << array[t];
                    // set stop to avoid double merge
                    stop = t + 1;
                }
                array[x] = 0;
                success = true;
            }
        }
    }
    return success;
}

void rotateBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t i, j, n = SIZE;
    uint8_t tmp;
    for (i = 0; i < n / 2; i++) {
        for (j = i; j < n - i - 1; j++) {
            tmp = board[i][j];
            board[i][j] = board[j][n - i - 1];
            board[j][n - i - 1] = board[n - i - 1][n - j - 1];
            board[n - i - 1][n - j - 1] = board[n - j - 1][i];
            board[n - j - 1][i] = tmp;
        }
    }
}

bool moveUp(uint8_t board[SIZE][SIZE]) {
    bool success = false;
    uint8_t x;
    for (x = 0; x < SIZE; x++) {
        success |= slideArray(board[x]);
    }
    return success;
}

bool moveLeft(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

bool moveDown(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

bool moveRight(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    return success;
}
