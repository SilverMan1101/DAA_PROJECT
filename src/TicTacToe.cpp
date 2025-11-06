#include "TicTacToe.h"
#include <algorithm>

// --- Embedded simple AI (minimax with alpha-beta) for Tic-Tac-Toe ---
static inline int tttScoreLine(char a, char b, char c) {
    if (a == 'O' && b == 'O' && c == 'O') return 10;
    if (a == 'X' && b == 'X' && c == 'X') return -10;
    return 0;
}
static int tttEvaluate(const char b[9]) {
    for (int r = 0; r < 3; ++r) {
        int s = tttScoreLine(b[r * 3 + 0], b[r * 3 + 1], b[r * 3 + 2]);
        if (s != 0) return s;
    }
    for (int c = 0; c < 3; ++c) {
        int s = tttScoreLine(b[0 * 3 + c], b[1 * 3 + c], b[2 * 3 + c]);
        if (s != 0) return s;
    }
    {
        int s = tttScoreLine(b[0], b[4], b[8]);
        if (s != 0) return s;
    }
    {
        int s = tttScoreLine(b[2], b[4], b[6]);
        if (s != 0) return s;
    }
    return 0;
}
static bool tttMovesLeft(const char b[9]) {
    for (int i = 0; i < 9; ++i) if (b[i] == ' ') return true; return false;
}
static int tttMinimax(char b[9], int depth, bool isMax, int alpha, int beta) {
    int score = tttEvaluate(b);
    if (score == 10) return score - depth;
    if (score == -10) return score + depth;
    if (!tttMovesLeft(b)) return 0;
    if (isMax) {
        int best = -1000;
        for (int i = 0; i < 9; ++i) if (b[i] == ' ') { b[i] = 'O'; int v = tttMinimax(b, depth + 1, false, alpha, beta); b[i] = ' '; best = std::max(best, v); alpha = std::max(alpha, best); if (beta <= alpha) break; }
        return best;
    }
    else {
        int best = 1000;
        for (int i = 0; i < 9; ++i) if (b[i] == ' ') { b[i] = 'X'; int v = tttMinimax(b, depth + 1, true, alpha, beta); b[i] = ' '; best = std::min(best, v); beta = std::min(beta, best); if (beta <= alpha) break; }
        return best;
    }
}
static int tttBestMove(const char board[9]) {
    char b[9]; for (int i = 0; i < 9; ++i) b[i] = board[i];
    int bestVal = -1000, bestMove = -1;
    for (int i = 0; i < 9; ++i) if (b[i] == ' ') {
        b[i] = 'O'; int val = tttMinimax(b, 0, false, -10000, 10000); b[i] = ' ';
        if (val > bestVal) { bestVal = val; bestMove = i; }
    }
    return bestMove;
}

void TicTacToeGame::init(int screenWidth, int screenHeight) {
    reset();
    const float margin = 50.0f;
    float size = (float)std::min(screenWidth, screenHeight) - 2.0f * margin;
    boardRect = { (screenWidth - size) * 0.5f, (screenHeight - size) * 0.5f, size, size };
    cellSize = size / 3.0f;
}

void TicTacToeGame::reset() {
    for (auto& row : board) row.fill(' ');
    gameOver = false;
    current = 'X';
    winner = ' ';
    isAnimating = false;
    animationProgress = 0.0f;
    lastRow = lastCol = -1;
    lastPiece = ' ';
    hasWinLine = false;
}

void TicTacToeGame::update(GameState& stateOut) {
    if (IsKeyPressed(KEY_M)) { stateOut = GameState::STATE_MENU; return; }
    if (IsKeyPressed(KEY_R)) { reset(); }
    if (gameOver) return;

    if (isAnimating) {
        animationProgress += GetFrameTime() * 2.0f;
        if (animationProgress >= 1.0f) {
            isAnimating = false;
            animationProgress = 1.0f;
            // After animation completes, switch turns and check game state
            char w = checkWinner();
            if (w != ' ') { gameOver = true; winner = w; }
            else if (isDraw()) { gameOver = true; winner = ' '; }
            else current = (lastPiece == 'X') ? 'O' : 'X';
        }
        return;
    }

    if (current == 'X') handleHuman();
    else aiTurn();
}

void TicTacToeGame::handleHuman() {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 m = GetMousePosition();
        if (CheckCollisionPointRec(m, boardRect)) {
            int col = (int)((m.x - boardRect.x) / cellSize);
            int row = (int)((m.y - boardRect.y) / cellSize);
            if (row >= 0 && row < 3 && col >= 0 && col < 3) {
                makeMove(row, col, 'X');
            }
        }
    }
}

bool TicTacToeGame::makeMove(int r, int c, char ch) {
    if (board[r][c] == ' ') { board[r][c] = ch; lastRow = r; lastCol = c; lastPiece = ch; isAnimating = true; animationProgress = 0.0f; hasWinLine = false; return true; } return false;
}

bool TicTacToeGame::hasMoves() const {
    for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) if (board[r][c] == ' ') return true; return false;
}

char TicTacToeGame::checkWinner() {
    // Rows
    for (int r = 0; r < 3; ++r) if (board[r][0] != ' ' && board[r][0] == board[r][1] && board[r][1] == board[r][2]) {
        hasWinLine = true; winStart = cellCenter(r, 0); winEnd = cellCenter(r, 2); return board[r][0];
    }
    // Cols
    for (int c = 0; c < 3; ++c) if (board[0][c] != ' ' && board[0][c] == board[1][c] && board[1][c] == board[2][c]) {
        hasWinLine = true; winStart = cellCenter(0, c); winEnd = cellCenter(2, c); return board[0][c];
    }
    // Diagonals
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) { hasWinLine = true; winStart = cellCenter(0, 0); winEnd = cellCenter(2, 2); return board[0][0]; }
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) { hasWinLine = true; winStart = cellCenter(0, 2); winEnd = cellCenter(2, 0); return board[0][2]; }
    hasWinLine = false; return ' ';
}

bool TicTacToeGame::isDraw()  { return checkWinner() == ' ' && !hasMoves(); }

void TicTacToeGame::aiTurn() {
    // Easy: random valid move
    if (difficulty == GameDifficulty::EASY) {
        int choices[9]; int n = 0;
        for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) if (board[r][c] == ' ') choices[n++] = r * 3 + c;
        if (n > 0) {
            int pick = choices[GetRandomValue(0, n - 1)];
            makeMove(pick / 3, pick % 3, 'O');
        }
        return;
    }

    // Medium: play immediate win/block else random center/corners/any
    if (difficulty == GameDifficulty::MEDIUM) {
        // Try to win
        for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) if (board[r][c] == ' ') {
            board[r][c] = 'O'; char w = checkWinner(); board[r][c] = ' ';
            if (w == 'O') { makeMove(r, c, 'O'); return; }
        }
        // Block X
        for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) if (board[r][c] == ' ') {
            board[r][c] = 'X'; char w = checkWinner(); board[r][c] = ' ';
            if (w == 'X') { makeMove(r, c, 'O'); return; }
        }
        // Prefer center, then corners, then any
        if (board[1][1] == ' ') { makeMove(1, 1, 'O'); return; }
        int corners[4][2] = { {0,0},{0,2},{2,0},{2,2} };
        for (auto& cc : corners) { if (board[cc[0]][cc[1]] == ' ') { makeMove(cc[0], cc[1], 'O'); return; } }
        int choices[9]; int n = 0;
        for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) if (board[r][c] == ' ') choices[n++] = r * 3 + c;
        if (n > 0) { int pick = choices[GetRandomValue(0, n - 1)]; makeMove(pick / 3, pick % 3, 'O'); }
        return;
    }

    // Hard: perfect play via minimax
    char flat[9];
    for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) flat[r * 3 + c] = board[r][c];
    int idx = tttBestMove(flat);
    if (idx >= 0) { int r = idx / 3; int c = idx % 3; makeMove(r, c, 'O'); }
}

Vector2 TicTacToeGame::cellCenter(int row, int col) const {
    return { boardRect.x + col * cellSize + cellSize * 0.5f, boardRect.y + row * cellSize + cellSize * 0.5f };
}

void TicTacToeGame::draw() const {
    const Color gridCol = { 30, 60, 130, 255 };
    const float x = boardRect.x; const float y = boardRect.y; const float s = boardRect.width; const float cs = cellSize;
    // Grid border and lines
    DrawRectangleRoundedLinesEx(boardRect, 0.06f, 6, 4.0f, gridCol);
    for (int i = 1; i < 3; ++i) {
        DrawLineEx({ x + i * cs, y }, { x + i * cs, y + s }, 6.0f, gridCol);
        DrawLineEx({ x, y + i * cs }, { x + s, y + i * cs }, 6.0f, gridCol);
    }
    // Pieces
    for (int r = 0; r < 3; ++r) for (int c = 0; c < 3; ++c) {
        char v = board[r][c]; if (v == ' ') continue; Vector2 center = cellCenter(r, c); float rad = cs * 0.3f;
        bool isLast = (r == lastRow && c == lastCol && isAnimating);
        if (v == 'X') {
            if (isLast) {
                float p = animationProgress;
                float half = 0.5f;
                Color xCol = Color{ 230, 80, 90, 255 };
                if (p <= half) {
                    float t = p / half;
                    Vector2 a = { center.x - rad, center.y - rad };
                    Vector2 b = { a.x + (rad * 2.0f) * t, a.y + (rad * 2.0f) * t };
                    DrawLineEx(a, b, 10.0f, xCol);
                }
                else {
                    Vector2 a1 = { center.x - rad, center.y - rad };
                    Vector2 b1 = { center.x + rad, center.y + rad };
                    DrawLineEx(a1, b1, 10.0f, xCol);
                    float t = (p - half) / (1.0f - half);
                    Vector2 a2 = { center.x + rad, center.y - rad };
                    Vector2 b2 = { a2.x - (rad * 2.0f) * t, a2.y + (rad * 2.0f) * t };
                    DrawLineEx(a2, b2, 10.0f, xCol);
                }
            }
            else {
                DrawLineEx({ center.x - rad, center.y - rad }, { center.x + rad, center.y + rad }, 10.0f, Color{ 230, 80, 90, 255 });
                DrawLineEx({ center.x + rad, center.y - rad }, { center.x - rad, center.y + rad }, 10.0f, Color{ 230, 80, 90, 255 });
            }
        }
        else { // 'O'
            if (isLast) {
                float p = animationProgress;
                float start = -90.0f;
                float end = start + 360.0f * p;
                DrawRing(center, rad - 8.0f, rad, start, end, 64, Color{ 80, 180, 255, 255 });
            }
            else {
                DrawRing(center, rad - 8.0f, rad, 0.0f, 360.0f, 64, Color{ 80, 180, 255, 255 });
            }
        }
    }
    // Win line highlight
    if (gameOver && hasWinLine) {
        DrawLineEx(winStart, winEnd, 14.0f, Color{ 255, 255, 255, 120 });
    }
    // Status text
    std::string status;
    if (gameOver) {
        if (winner == 'X') status = "Player Wins!";
        else if (winner == 'O') status = "AI Wins!";
        else status = "Draw!";
    }
    else if (isAnimating) {
        status = (lastPiece == 'X') ? "Placing X..." : "Placing O...";
    }
    else {
        status = (current == 'X') ? "Your turn (X)" : "AI thinking (O)";
    }
    Vector2 ts = MeasureTextEx(uiFont, status.c_str(), 24.0f, 2.0f);
    Vector2 tp = { GetScreenWidth() * 0.5f - ts.x * 0.5f, boardRect.y + boardRect.height + 16 };
    DrawTextEx(uiFont, status.c_str(), tp, 24.0f, 2.0f, Color{ 220, 230, 240, 255 });

    // Title and hint
    const char* title = "Tic-Tac-Toe";
    Vector2 tSize = MeasureTextEx(uiFont, title, 32.0f, 2.0f);
    DrawTextEx(uiFont, title, { GetScreenWidth() * 0.5f - tSize.x * 0.5f , boardRect.y - 50 }, 32.0f, 2.0f, RAYWHITE);
    const char* hint = "Click a column. R to restart. M for Menu.";
    Vector2 hSize = MeasureTextEx(uiFont, hint, 20.0f, 2.0f);
    DrawTextEx(uiFont, hint, { GetScreenWidth() * 0.5f - hSize.x * 0.5f - 30, boardRect.y - 22 }, 25.0f, 2.0f, Color{ 200,210,225,255 });
}


