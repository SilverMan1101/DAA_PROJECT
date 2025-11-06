#pragma once
#include "raylib.h"
#include "globals.h"

enum class GameState { STATE_MENU = 0, STATE_TIC_TAC_TOE, STATE_CONNECT_FOUR, STATE_CHESS };

class Menu {
public:
    void init(int screenWidth, int screenHeight);
    void update(GameState& stateOut, GameDifficulty& difficulty);
    void draw(Font font, GameDifficulty difficulty) const;
    void setFont(Font f) { uiFont = f; }

private:
    Rectangle btnTicTacToe{ 0,0,0,0 };
    Rectangle btnConnectFour{ 0,0,0,0 };
    Rectangle btnChess{ 0,0,0,0 };
    Rectangle titleRect{ 0,0,0,0 };
    Rectangle btnEasy{ 0,0,0,0 };
    Rectangle btnMedium{ 0,0,0,0 };
    Rectangle btnHard{ 0,0,0,0 };
    int screenW = 0;
    int screenH = 0;
    Font uiFont{};
    mutable float btnPressT1 = 0.0f;
    mutable float btnPressT2 = 0.0f;
    mutable float btnPressT3 = 0.0f;
    mutable float btnPressDE = 0.0f;
    mutable float btnPressDM = 0.0f;
    mutable float btnPressDH = 0.0f;
};


