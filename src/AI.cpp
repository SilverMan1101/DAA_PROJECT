#include "AI.h"
#include <algorithm>

static inline int scoreLine(char a, char b, char c) {
    if (a == 'O' && b == 'O' && c == 'O') return 10;
    if (a == 'X' && b == 'X' && c == 'X') return -10;
    return 0;
}

static int evaluate(const char b[9]) {
    // Rows
    for (int r = 0; r < 3; ++r) {
        int s = scoreLine(b[r * 3 + 0], b[r * 3 + 1], b[r * 3 + 2]);
        if (s != 0) return s;
    }
    // Cols
    for (int c = 0; c < 3; ++c) {
        int s = scoreLine(b[0 * 3 + c], b[1 * 3 + c], b[2 * 3 + c]);
        if (s != 0) return s;
    }
    // Diagonals
    {
        int s = scoreLine(b[0], b[4], b[8]);
        if (s != 0) return s;
    }
    {
        int s = scoreLine(b[2], b[4], b[6]);
        if (s != 0) return s;
    }
    return 0;
}

static bool movesLeft(const char b[9]) {
    for (int i = 0; i < 9; ++i) if (b[i] == ' ') return true;
    return false;
}

static int minimax(char b[9], int depth, bool isMax, int alpha, int beta) {
    int score = evaluate(b);
    if (score == 10) return score - depth;   // prefer quicker wins
    if (score == -10) return score + depth;  // prefer slower losses
    if (!movesLeft(b)) return 0;

    if (isMax) {
        int best = -1000;
        for (int i = 0; i < 9; ++i) {
            if (b[i] == ' ') {
                b[i] = 'O';
                int val = minimax(b, depth + 1, false, alpha, beta);
                b[i] = ' ';
                best = std::max(best, val);
                alpha = std::max(alpha, best);
                if (beta <= alpha) break; // beta cut-off
            }
        }
        return best;
    }
    else {
        int best = 1000;
        for (int i = 0; i < 9; ++i) {
            if (b[i] == ' ') {
                b[i] = 'X';
                int val = minimax(b, depth + 1, true, alpha, beta);
                b[i] = ' ';
                best = std::min(best, val);
                beta = std::min(beta, best);
                if (beta <= alpha) break; // alpha cut-off
            }
        }
        return best;
    }
}

int AI_FindBestMove(const char board[9]) {
    char b[9];
    for (int i = 0; i < 9; ++i) b[i] = board[i];

    int bestVal = -1000;
    int bestMove = -1;
    for (int i = 0; i < 9; ++i) {
        if (b[i] == ' ') {
            b[i] = 'O';
            int moveVal = minimax(b, 0, false, -10000, 10000);
            b[i] = ' ';
            if (moveVal > bestVal) {
                bestVal = moveVal;
                bestMove = i;
            }
        }
    }
    return bestMove;
}


