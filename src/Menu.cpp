#include "Menu.h"
#include <string>

void Menu::init(int screenWidth, int screenHeight) {
    screenW = screenWidth;
    screenH = screenHeight;
    const float btnW = 320.0f;
    const float btnH = 64.0f;
    const float spacing = 24.0f;
    const float cx = screenW * 0.5f;
    const float cy = screenH * 0.5f;

    //btnTicTacToe = { cx - btnW * 0.5f, cy - btnH - spacing * 0.5f, btnW, btnH };
    //btnConnectFour = { cx - btnW * 0.5f, cy + spacing * 0.5f, btnW, btnH };
    //titleRect = { 0, cy - btnH - spacing * 2.5f, (float)screenW, 80.0f };
    btnTicTacToe = { cx - btnW * 0.5f, cy - btnH * 1.5f - spacing, btnW, btnH };
    btnConnectFour = { cx - btnW * 0.5f, cy - btnH * 0.5f, btnW, btnH };
    btnChess = { cx - btnW * 0.5f, cy + btnH * 0.5f + spacing, btnW, btnH };
    titleRect = { 0, cy - btnH * 2.0f - spacing * 2.5f, (float)screenW, 80.0f };

    // Difficulty buttons under game choices
    const float dW = 120.0f;
    const float dH = 44.0f;
    //float dy = cy + btnH + spacing * 1.8f;
    float dy = cy + btnH * 1.5f + spacing * 2.0f;
    float dx = cx - (dW * 3.0f + spacing * 2.0f) * 0.5f;
    btnEasy = { dx + 0 * (dW + spacing), dy, dW, dH };
    btnMedium = { dx + 1 * (dW + spacing), dy, dW, dH };
    btnHard = { dx + 2 * (dW + spacing), dy, dW, dH };
}

void Menu::update(GameState& stateOut, GameDifficulty& difficulty) {
    Vector2 m = GetMousePosition();
    bool click = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (CheckCollisionPointRec(m, btnTicTacToe)) {
        if (click) { btnPressT1 = 1.0f; stateOut = GameState::STATE_TIC_TAC_TOE; }
    }
    if (CheckCollisionPointRec(m, btnConnectFour)) {
        if (click) { btnPressT2 = 1.0f; stateOut = GameState::STATE_CONNECT_FOUR; }
    }
    if (CheckCollisionPointRec(m, btnChess)) {
        if (click) { btnPressT3 = 1.0f; stateOut = GameState::STATE_CHESS; }
    }

    if (CheckCollisionPointRec(m, btnEasy) && click) { btnPressDE = 1.0f; difficulty = GameDifficulty::EASY; }
    if (CheckCollisionPointRec(m, btnMedium) && click) { btnPressDM = 1.0f; difficulty = GameDifficulty::MEDIUM; }
    if (CheckCollisionPointRec(m, btnHard) && click) { btnPressDH = 1.0f; difficulty = GameDifficulty::HARD; }
    float dt = GetFrameTime();
    btnPressT1 = btnPressT1 > 0.0f ? btnPressT1 - dt * 4.0f : 0.0f;
    btnPressT2 = btnPressT2 > 0.0f ? btnPressT2 - dt * 4.0f : 0.0f;
    btnPressT3 = btnPressT3 > 0.0f ? btnPressT3 - dt * 4.0f : 0.0f;
    btnPressDE = btnPressDE > 0.0f ? btnPressDE - dt * 4.0f : 0.0f;
    btnPressDM = btnPressDM > 0.0f ? btnPressDM - dt * 4.0f : 0.0f;
    btnPressDH = btnPressDH > 0.0f ? btnPressDH - dt * 4.0f : 0.0f;
}

static void DrawButtonFancy(const Rectangle& r, const char* label, Font font, float pressT) {
    Vector2 m = GetMousePosition();
    bool hovered = CheckCollisionPointRec(m, r);
    Color base = { 50, 100, 200, 255 };
    Color hover = { 70, 130, 230, 255 };
    Color down = { 35, 70, 150, 255 };
    Color fill = hovered ? hover : base;
    if (pressT > 0.0f) fill = down;

    float radius = 12.0f;
    DrawRectangleRounded(r, 0.25f, 8, fill);

    // Subtle border
    DrawRectangleRoundedLinesEx(r, 0.25f, 8, 2.0f, Color{ 255, 255, 255, 40 });

    // Scale effect when pressed
    float scale = 1.0f - 0.06f * pressT;
    Vector2 textSize = MeasureTextEx(font, label, 28.0f, 2.0f);
    Vector2 pos = { r.x + (r.width - textSize.x * scale) * 0.5f, r.y + (r.height - textSize.y * scale) * 0.5f };
    DrawTextEx(font, label, pos, 32.0f * scale, 2.0f * scale, RAYWHITE);
}

void Menu::draw(Font font, GameDifficulty difficulty) const {
    const char* title = "Strategic Game Mastery";
    Vector2 tSize = MeasureTextEx(font, title, 48.0f, 3.0f);
    Vector2 tPos = { titleRect.width * 0.5f - tSize.x * 0.5f, titleRect.y-60 };
    DrawTextEx(font, title, tPos, 48.0f, 3.0f, RAYWHITE);

    DrawButtonFancy(btnTicTacToe, "Play Tic-Tac-Toe", font, btnPressT1);
    DrawButtonFancy(btnConnectFour, "Play Connect Four", font, btnPressT2);
    DrawButtonFancy(btnChess, "Play Chess", font, btnPressT3);

    const char* hint = "Click a game to start";
    Vector2 hSize = MeasureTextEx(font, hint, 22.0f, 2.0f);
    Vector2 hPos = { screenW * 0.5f - hSize.x * 0.5f-32, titleRect.y };
    DrawTextEx(font, hint, hPos, 32.0f, 2.0f, Color{ 200, 210, 225, 255 });

    // Difficulty UI
    auto drawDiffBtn = [&](const Rectangle& r, const char* label, bool selected, float pressT) {
        Vector2 m = GetMousePosition();
        bool hovered = CheckCollisionPointRec(m, r);
        Color base = selected ? Color{ 90, 170, 90, 255 } : Color{ 50, 100, 200, 255 };
        Color hover = selected ? Color{ 110, 190, 110, 255 } : Color{ 70, 130, 230, 255 };
        Color down = selected ? Color{ 60, 120, 60, 255 } : Color{ 35, 70, 150, 255 };
        Color fill = hovered ? hover : base;
        if (pressT > 0.0f) fill = down;
        DrawRectangleRounded(r, 0.25f, 8, fill);
        DrawRectangleRoundedLinesEx(r, 0.25f, 8, 2.0f, Color{ 255, 255, 255, 40 });
        Vector2 textSize = MeasureTextEx(font, label, 24.0f, 2.0f);
        Vector2 pos = { r.x + (r.width - textSize.x) * 0.5f-10, r.y + (r.height - textSize.y) * 0.5f };
        DrawTextEx(font, label, pos, 28.0f, 2.0f, RAYWHITE);
        };
    drawDiffBtn(btnEasy, "Easy", difficulty == GameDifficulty::EASY, btnPressDE);
    drawDiffBtn(btnMedium, "Medium", difficulty == GameDifficulty::MEDIUM, btnPressDM);
    drawDiffBtn(btnHard, "Hard", difficulty == GameDifficulty::HARD, btnPressDH);
}


