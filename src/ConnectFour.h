#pragma once
#include "raylib.h"
#include <array>
#include "Menu.h" // GameState enum
#include "globals.h"

class ConnectFourGame {
public:
    void init(int screenWidth, int screenHeight);
    void update(GameState& stateOut);
    void draw() const;
    void setFont(Font f) { uiFont = f; }
    void setDifficulty(GameDifficulty d) { difficulty = d; }

private:
    static constexpr int ROWS = 6;
    static constexpr int COLS = 7;
    std::array<std::array<char, COLS>, ROWS> board{}; // ' ', 'R' (human), 'Y' (AI)
    Rectangle boardRect{ 0,0,0,0 };
    float cellSize = 0.0f;
    bool gameOver = false;
    char current = 'R';
    char winner = ' ';
    Font uiFont{};

    int screenW = 0;
    int screenH = 0;

    // Hover/preview
    int hoverCol = -1;

    // Drop animation
    mutable bool isAnimating = false;
    mutable Vector2 animationPos{ 0,0 };
    mutable int animTargetRow = -1;
    mutable int animTargetCol = -1;
    mutable float animationSpeed = 900.0f; // pixels per second
    mutable char animPiece = ' ';

    // Win highlight
    mutable int winningCount = 0;
    mutable Vector2 winningPieces[4]{};

    GameDifficulty difficulty = GameDifficulty::HARD;
    int getSearchDepth() const { return (difficulty == GameDifficulty::EASY) ? 2 : (difficulty == GameDifficulty::MEDIUM) ? 4 : 5; }

    void reset();
    int lowestEmptyRow(int col) const; // -1 if column full
    bool dropPiece(int col, char who);
    bool hasMoves() const;
    char checkWinner(); // 'R', 'Y', or ' '
    bool isDraw() ;
    int columnFromMouse(Vector2 m) const;

    // AI helpers
    int aiChooseColumn();
    int minimax(int depth, int alpha, int beta, bool maximizing);
    int evaluateBoard() const;
    int evaluateWindow(int r0, int c0, int dr, int dc) const; // window of 4
};


