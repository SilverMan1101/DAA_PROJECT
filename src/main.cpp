// main.cpp - Strategic Game Mastery entry and state machine
#include "raylib.h"
#include "Menu.h"
#include "TicTacToe.h"
#include "ConnectFour.h"
#include "Chess.h"
#include "globals.h"

// Define the global difficulty variable (default Hard)
GameDifficulty currentDifficulty = HARD;

static void RunGameLoop() {
    const int screenWidth = 800;
    const int screenHeight = 720;
    InitWindow(screenWidth, screenHeight, "Strategic Game Mastery");
    SetTargetFPS(60);
    Font uiFont = LoadFontEx("src/NotoSansSymbols2-Regular.ttf", 64, 0, 65535);
    Color bg = { 20, 27, 40, 255 };
	//Image i = LoadImage("src/im.png");
    GameState state = GameState::STATE_MENU;
    Menu menu; menu.init(screenWidth, screenHeight); menu.setFont(uiFont);
    TicTacToeGame ttt; ttt.init(screenWidth, screenHeight); ttt.setFont(uiFont);
    ConnectFourGame c4; c4.init(screenWidth, screenHeight); c4.setFont(uiFont);
    ChessGame chess; chess.init(screenWidth, screenHeight); chess.setFont(uiFont);

    while (!WindowShouldClose()) {
        // Update
        switch (state) {
        case GameState::STATE_MENU: menu.update(state, currentDifficulty); break;
        case GameState::STATE_TIC_TAC_TOE: ttt.setDifficulty(currentDifficulty); ttt.update(state); break;
        case GameState::STATE_CONNECT_FOUR: c4.setDifficulty(currentDifficulty); c4.update(state); break;
        case GameState::STATE_CHESS: chess.setDifficulty(currentDifficulty); chess.update(state); break;
        }

        // Draw
        BeginDrawing();
        ClearBackground(bg);
        switch (state) {
        case GameState::STATE_MENU: menu.draw(uiFont, currentDifficulty); break;
        case GameState::STATE_TIC_TAC_TOE: ttt.draw(); break;
        case GameState::STATE_CONNECT_FOUR: c4.draw(); break;
        case GameState::STATE_CHESS: chess.draw(); break;
        }
        EndDrawing();
    }

    UnloadFont(uiFont);
    CloseWindow();
}

// Standard console entry (useful for console builds)
int main() {
    RunGameLoop();
    return 0;
}