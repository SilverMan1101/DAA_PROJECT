#include "ConnectFour.h"
#include <algorithm>

void ConnectFourGame::init(int screenWidth, int screenHeight) {
    screenW = screenWidth; screenH = screenHeight;
    reset();
    const float marginX = 40.0f;
    const float marginY = 80.0f;
    float width = screenWidth - marginX * 2.0f;
    float height = screenHeight - marginY * 2.0f;
    float csX = width / COLS;
    float csY = height / ROWS;
    cellSize = std::min(csX, csY);
    float bw = cellSize * COLS;
    float bh = cellSize * ROWS;
    boardRect = { (screenWidth - bw) * 0.5f, (screenHeight - bh) * 0.5f, bw, bh };
}

void ConnectFourGame::reset() {
    for (auto& row : board) row.fill(' ');
    gameOver = false;
    winner = ' ';
    current = 'R';
    hoverCol = -1;
    isAnimating = false;
    animTargetRow = animTargetCol = -1; animPiece = ' ';
    winningCount = 0;
}

int ConnectFourGame::lowestEmptyRow(int col) const {
    if (col < 0 || col >= COLS) return -1;
    for (int r = ROWS - 1; r >= 0; --r) if (board[r][col] == ' ') return r;
    return -1;
}

bool ConnectFourGame::dropPiece(int col, char who) { // immediate place (used by AI search only)
    int r = lowestEmptyRow(col);
    if (r == -1) return false;
    board[r][col] = who;
    return true;
}

bool ConnectFourGame::hasMoves() const {
    for (int c = 0; c < COLS; ++c) if (board[0][c] == ' ') return true; return false;
}

char ConnectFourGame::checkWinner() {
    // Horizontal
    winningCount = 0;
    for (int r = 0; r < ROWS; ++r) for (int c = 0; c <= COLS - 4; ++c) {
        char ch = board[r][c]; if (ch == ' ') continue;
        if (board[r][c + 1] == ch && board[r][c + 2] == ch && board[r][c + 3] == ch) {
            winningCount = 4;
            for (int i = 0; i < 4; ++i) winningPieces[i] = { boardRect.x + (c + i + 0.5f) * cellSize, boardRect.y + (r + 0.5f) * cellSize };
            return ch;
        }
    }
    // Vertical
    for (int c = 0; c < COLS; ++c) for (int r = 0; r <= ROWS - 4; ++r) {
        char ch = board[r][c]; if (ch == ' ') continue;
        if (board[r + 1][c] == ch && board[r + 2][c] == ch && board[r + 3][c] == ch) {
            winningCount = 4;
            for (int i = 0; i < 4; ++i) winningPieces[i] = { boardRect.x + (c + 0.5f) * cellSize, boardRect.y + (r + i + 0.5f) * cellSize };
            return ch;
        }
    }
    // Diagonal down-right
    for (int r = 0; r <= ROWS - 4; ++r) for (int c = 0; c <= COLS - 4; ++c) {
        char ch = board[r][c]; if (ch == ' ') continue;
        if (board[r + 1][c + 1] == ch && board[r + 2][c + 2] == ch && board[r + 3][c + 3] == ch) {
            winningCount = 4;
            for (int i = 0; i < 4; ++i) winningPieces[i] = { boardRect.x + (c + i + 0.5f) * cellSize, boardRect.y + (r + i + 0.5f) * cellSize };
            return ch;
        }
    }
    // Diagonal up-right
    for (int r = 3; r < ROWS; ++r) for (int c = 0; c <= COLS - 4; ++c) {
        char ch = board[r][c]; if (ch == ' ') continue;
        if (board[r - 1][c + 1] == ch && board[r - 2][c + 2] == ch && board[r - 3][c + 3] == ch) {
            winningCount = 4;
            for (int i = 0; i < 4; ++i) winningPieces[i] = { boardRect.x + (c + i + 0.5f) * cellSize, boardRect.y + (r - i + 0.5f) * cellSize };
            return ch;
        }
    }
    return ' ';
}

bool ConnectFourGame::isDraw()  { return checkWinner() == ' ' && !hasMoves(); }

int ConnectFourGame::columnFromMouse(Vector2 m) const {
    if (!CheckCollisionPointRec(m, boardRect)) return -1;
    int col = (int)((m.x - boardRect.x) / cellSize);
    if (col < 0 || col >= COLS) return -1;
    return col;
}

void ConnectFourGame::update(GameState& stateOut) {
    if (IsKeyPressed(KEY_M)) { stateOut = GameState::STATE_MENU; return; }
    if (IsKeyPressed(KEY_R)) { reset(); }
    if (gameOver) return;

    // Hover tracking
    hoverCol = columnFromMouse(GetMousePosition());

    // Animate falling piece
    if (isAnimating) {
        float targetY = boardRect.y + (animTargetRow + 0.5f) * cellSize;
        animationPos.y += animationSpeed * GetFrameTime();
        if (animationPos.y >= targetY) {
            animationPos.y = targetY;
            // finalize placement
            board[animTargetRow][animTargetCol] = animPiece;
            isAnimating = false;
            // check winner/draw
            char w = checkWinner();
            if (w != ' ') { winner = w; gameOver = true; return; }
            if (isDraw()) { winner = ' '; gameOver = true; return; }
            // switch turn
            current = (animPiece == 'R') ? 'Y' : 'R';
        }
        return; // pause game logic during animation
    }

    if (current == 'R') {
        int col = hoverCol;
        if (col != -1 && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int r = lowestEmptyRow(col);
            if (r != -1) {
                animTargetRow = r; animTargetCol = col; animPiece = 'R';
                animationPos = { boardRect.x + (col + 0.5f) * cellSize, boardRect.y - cellSize * 0.5f };
                isAnimating = true;
            }
        }
    }
    else {
        int col = aiChooseColumn();
        if (col != -1) {
            int r = lowestEmptyRow(col);
            if (r != -1) {
                animTargetRow = r; animTargetCol = col; animPiece = 'Y';
                animationPos = { boardRect.x + (col + 0.5f) * cellSize, boardRect.y - cellSize * 0.5f };
                isAnimating = true;
            }
        }
    }
}

static int scoreCount(int countSelf, int countOpp, int countEmpty) {
    if (countSelf == 4) return 100000;
    if (countSelf == 3 && countEmpty == 1) return 100;
    if (countSelf == 2 && countEmpty == 2) return 10;
    if (countOpp == 3 && countEmpty == 1) return -120; // prioritize blocking
    if (countOpp == 2 && countEmpty == 2) return -8;
    return 0;
}

int ConnectFourGame::evaluateWindow(int r0, int c0, int dr, int dc) const {
    int r = r0, c = c0;
    int cntR = 0, cntY = 0, cntE = 0;
    for (int i = 0; i < 4; ++i) {
        char ch = board[r][c];
        if (ch == 'R') ++cntR; else if (ch == 'Y') ++cntY; else ++cntE;
        r += dr; c += dc;
    }
    return scoreCount(cntR, cntY, cntE) - scoreCount(cntY, cntR, cntE) / 10; // small asymmetry
}

int ConnectFourGame::evaluateBoard() const {
    int score = 0;
    // Center column preference
    int center = COLS / 2;
    int centerCountR = 0, centerCountY = 0;
    for (int r = 0; r < ROWS; ++r) { if (board[r][center] == 'R') ++centerCountR; if (board[r][center] == 'Y') ++centerCountY; }
    score += (centerCountR - centerCountY) * 6;

    // Horizontal
    for (int r = 0; r < ROWS; ++r) for (int c = 0; c <= COLS - 4; ++c) score += evaluateWindow(r, c, 0, 1);
    // Vertical
    for (int c = 0; c < COLS; ++c) for (int r = 0; r <= ROWS - 4; ++r) score += evaluateWindow(r, c, 1, 0);
    // Diagonals
    for (int r = 0; r <= ROWS - 4; ++r) for (int c = 0; c <= COLS - 4; ++c) score += evaluateWindow(r, c, 1, 1);
    for (int r = 3; r < ROWS; ++r) for (int c = 0; c <= COLS - 4; ++c) score += evaluateWindow(r, c, -1, 1);
    return score;
}

int ConnectFourGame::minimax(int depth, int alpha, int beta, bool maximizing) {
    char w = checkWinner();
    if (w == 'Y') return 1000000 - depth;
    if (w == 'R') return -1000000 + depth;
    if (isDraw() || depth >= getSearchDepth()) return evaluateBoard();

    if (maximizing) {
        int best = -10000000;
        // Order by proximity to center
        int order[7] = { 3,2,4,1,5,0,6 };
        for (int k = 0; k < COLS; ++k) {
            int c = order[k];
            int r = lowestEmptyRow(c);
            if (r == -1) continue;
            board[r][c] = 'Y';
            int val = minimax(depth + 1, alpha, beta, false);
            board[r][c] = ' ';
            best = std::max(best, val);
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;
        }
        return best;
    }
    else {
        int best = 10000000;
        int order[7] = { 3,2,4,1,5,0,6 };
        for (int k = 0; k < COLS; ++k) {
            int c = order[k];
            int r = lowestEmptyRow(c);
            if (r == -1) continue;
            board[r][c] = 'R';
            int val = minimax(depth + 1, alpha, beta, true);
            board[r][c] = ' ';
            best = std::min(best, val);
            beta = std::min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }
}

int ConnectFourGame::aiChooseColumn() {
    // Easy: random valid column
    if (difficulty == GameDifficulty::EASY) {
        int cols[COLS]; int n = 0;
        for (int c = 0; c < COLS; ++c) if (lowestEmptyRow(c) != -1) cols[n++] = c;
        if (n == 0) return -1;
        return cols[GetRandomValue(0, n - 1)];
    }

    // Medium/Hard: minimax with depth based on difficulty
    int bestVal = -10000000;
    int bestCol = -1;
    int order[7] = { 3,2,4,1,5,0,6 };
    for (int k = 0; k < COLS; ++k) {
        int c = order[k];
        int r = lowestEmptyRow(c);
        if (r == -1) continue;
        board[r][c] = 'Y';
        int val = minimax(0, -10000000, 10000000, false);
        board[r][c] = ' ';
        if (val > bestVal) { bestVal = val; bestCol = c; }
    }
    return bestCol;
}

void ConnectFourGame::draw() const {
    // Board frame
    Color boardCol = { 30, 60, 130, 255 };
    DrawRectangleRounded({ boardRect.x - 12, boardRect.y - 12, boardRect.width + 24, boardRect.height + 24 }, 0.08f, 8, boardCol);
    DrawRectangleRoundedLinesEx({ boardRect.x - 12, boardRect.y - 12, boardRect.width + 24, boardRect.height + 24 }, 0.08f, 8, 3.0f, Color{ 255,255,255,40 });

    // Hover column overlay
    if (hoverCol != -1) {
        Rectangle colRect = { boardRect.x + hoverCol * cellSize, boardRect.y, cellSize, boardRect.height };
        DrawRectangleRec(colRect, Color{ 255, 255, 255, 30 });
    }

    // Draw holes background and pieces
    Color holeCol = { 15, 22, 35, 255 };
    for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c) {
        float cx = boardRect.x + c * cellSize + cellSize * 0.5f;
        float cy = boardRect.y + r * cellSize + cellSize * 0.5f;
        float rad = cellSize * 0.38f;
        char ch = board[r][c];
        if (ch == ' ') {
            DrawCircle((int)cx, (int)cy, rad, holeCol);
        }
        else {
            // Piece with simple shading
            Color base = (ch == 'R') ? Color{ 200, 60, 60, 255 } : Color{ 235, 200, 60, 255 };
            Color shadow = Color{ 0,0,0,60 };
            DrawCircle((int)cx, (int)cy, rad, base);
            DrawCircle((int)(cx - rad * 0.2f), (int)(cy - rad * 0.2f), rad * 0.6f, Color{ 255,255,255,40 });
            DrawCircle((int)(cx + rad * 0.15f), (int)(cy + rad * 0.15f), rad * 0.9f, Color{ 0,0,0,25 });
        }
    }

    // Animated falling piece
    if (isAnimating) {
        float rad = cellSize * 0.38f;
        Color base = (animPiece == 'R') ? Color{ 200, 60, 60, 255 } : Color{ 235, 200, 60, 255 };
        DrawCircle((int)animationPos.x, (int)animationPos.y, rad, base);
        DrawCircle((int)(animationPos.x - rad * 0.2f), (int)(animationPos.y - rad * 0.2f), rad * 0.6f, Color{ 255,255,255,40 });
        DrawCircle((int)(animationPos.x + rad * 0.15f), (int)(animationPos.y + rad * 0.15f), rad * 0.9f, Color{ 0,0,0,25 });
    }

    // Preview piece at top
    if (!gameOver && !isAnimating && hoverCol != -1) {
        float cx = boardRect.x + hoverCol * cellSize + cellSize * 0.5f;
        float cy = boardRect.y - cellSize * 0.5f;
        float rad = cellSize * 0.34f;
        Color base = (current == 'R') ? Color{ 200, 60, 60, 160 } : Color{ 235, 200, 60, 160 };
        DrawCircle((int)cx, (int)cy, rad, base);
    }

    // Title and hint
    const char* title = "Connect Four";
    Vector2 tSize = MeasureTextEx(uiFont, title, 32.0f, 2.0f);
    DrawTextEx(uiFont, title, { GetScreenWidth() * 0.5f - tSize.x * 0.5f, boardRect.y - 75 }, 32.0f, 2.0f, RAYWHITE);
    const char* hint = "Click a column. R to restart. M for Menu.";
    Vector2 hSize = MeasureTextEx(uiFont, hint, 20.0f, 2.0f);
    DrawTextEx(uiFont, hint, { GetScreenWidth() * 0.5f - hSize.x * 0.5f-30, boardRect.y - 40 }, 25.0f, 2.0f, Color{ 200,210,225,255 });

    // Win highlight
    if (gameOver && winningCount == 4) {
        float pulse = 0.6f + 0.4f * sinf((float)GetTime() * 4.0f);
        Color glow = { 255,255,255, (unsigned char)(120 + 60 * pulse) };
        for (int i = 0; i < 4; ++i) {
            DrawCircleV(winningPieces[i], cellSize * 0.22f, glow);
        }
        const char* msg = (winner == 'R') ? "You win!" : (winner == 'Y') ? "AI wins!" : "Draw!";
        Vector2 mSize = MeasureTextEx(uiFont, msg, 28.0f, 2.0f);
        DrawTextEx(uiFont, msg, { GetScreenWidth() * 0.5f - mSize.x * 0.5f, boardRect.y + boardRect.height + 35 }, 28.0f, 2.0f, RAYWHITE);
    }
}


