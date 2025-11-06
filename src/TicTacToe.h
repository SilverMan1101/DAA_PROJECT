#pragma once
#include "raylib.h"
#include <array>
#include <string>
#include "Menu.h" // for GameState enum
#include "globals.h"

class TicTacToeGame {
public:
    void init(int screenWidth, int screenHeight);
    void update(GameState& stateOut);
    void draw() const;
    void setFont(Font f) { uiFont = f; }
    void setDifficulty(GameDifficulty d) { difficulty = d; }

private:
    std::array<std::array<char, 3>, 3> board{}; // ' ', 'X', 'O'
    Rectangle boardRect{ 0,0,0,0 };
    float cellSize = 0.0f;
    bool gameOver = false;
    char current = 'X';
    char winner = ' ';
    Font uiFont{};
    // Animation state
    bool isAnimating = false;
    float animationProgress = 0.0f; // 0..1
    int lastRow = -1;
    int lastCol = -1;
    char lastPiece = ' ';
    // Win line
    bool hasWinLine = false;
    Vector2 winStart{ 0,0 };
    Vector2 winEnd{ 0,0 };

    GameDifficulty difficulty = GameDifficulty::HARD;

    void reset();
    void handleHuman();
    void aiTurn();
    bool makeMove(int r, int c, char ch);
    bool hasMoves() const;
    char checkWinner(); // returns 'X', 'O', or ' '
    bool isDraw() ;
    Vector2 cellCenter(int row, int col) const;
};


