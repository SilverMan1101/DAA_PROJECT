#pragma once
#include "raylib.h"
#include <array>
#include <vector>
#include "Menu.h" // for GameState enum
#include "globals.h"
#ifdef WHITE
#undef WHITE
#endif
#ifdef BLACK
#undef BLACK
#endif

enum class PieceType { EMPTY, KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN };
enum class PieceColor { NONE, WHITE, BLACK };


struct Piece {
    PieceType type = PieceType::EMPTY;
    PieceColor color = PieceColor::NONE;

    bool isEmpty() const { return type == PieceType::EMPTY; }
};

struct Move {
    int fromRow, fromCol;
    int toRow, toCol;
    PieceType promotion = PieceType::QUEEN; // For pawn promotion
    bool isCastling = false;
    bool isEnPassant = false;

    Move(int fr, int fc, int tr, int tc) : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc) {}
};

class ChessGame {
public:
    void init(int screenWidth, int screenHeight);
    void update(GameState& stateOut);
    void draw() const;
    void setFont(Font f) { uiFont = f; }
    void setDifficulty(GameDifficulty d) { difficulty = d; }

private:
    static constexpr int BOARD_SIZE = 8;
    std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> board{};
    Rectangle boardRect{ 0,0,0,0 };
    float cellSize = 0.0f;

    PieceColor currentPlayer = PieceColor::WHITE; // Human = WHITE, AI = BLACK
    bool gameOver = false;
    bool AisCheckmate = false;
    bool AisInCheck = false;
    bool AisStalemate = false;
    
    PieceColor winner = PieceColor::NONE;

    // Selection and moves
    int selectedRow = -1;
    int selectedCol = -1;
    std::vector<Move> legalMoves;
    std::vector<Move> highlightedMoves; // Moves for selected piece

    // Castling rights
    bool whiteKingMoved = false;
    bool whiteRookLeftMoved = false;
    bool whiteRookRightMoved = false;
    bool blackKingMoved = false;
    bool blackRookLeftMoved = false;
    bool blackRookRightMoved = false;

    // En passant
    int enPassantRow = -1;
    int enPassantCol = -1;

    Font uiFont{};
    int screenW = 0;
    int screenH = 0;
    GameDifficulty difficulty = GameDifficulty::HARD;

    // Material values for evaluation
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 300;
    static constexpr int BISHOP_VALUE = 300;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    static constexpr int KING_VALUE = 0; // Not used in evaluation

    void reset();
    void initBoard();
    Vector2 squareCenter(int row, int col) const;
    int squareFromMouse(Vector2 m, int& row, int& col) const;

    // Move generation
    void generatePseudoLegalMoves(PieceColor color, std::vector<Move>& moves) const;
    void generateMovesForPiece(int row, int col, std::vector<Move>& moves) const;
    void generatePawnMoves(int row, int col, std::vector<Move>& moves) const;
    void generateRookMoves(int row, int col, std::vector<Move>& moves) const;
    void generateBishopMoves(int row, int col, std::vector<Move>& moves) const;
    void generateQueenMoves(int row, int col, std::vector<Move>& moves) const;
    void generateKnightMoves(int row, int col, std::vector<Move>& moves) const;
    void generateKingMoves(int row, int col, std::vector<Move>& moves) const;

    // Move validation
    bool isLegalMove(const Move& move, PieceColor color) const;
    bool isInCheck(PieceColor color) const;
    bool hasLegalMoves(PieceColor color) const;
    void validateMoves(std::vector<Move>& moves, PieceColor color) const;

    // Move execution
    void makeMove(const Move& move);
    void unmakeMove(const Move& move, Piece capturedPiece);
    Piece applyMove(const Move& move);

    // Game state
    void checkGameState();
    bool isCheckmate(PieceColor color) const;
    bool isStalemate(PieceColor color) const;

    // AI
    void aiTurn();
    Move aiChooseMove();
    int minimax(int depth, int alpha, int beta, bool maximizing, PieceColor color);
    int evaluateBoard() const;
    int getPieceValue(PieceType type) const;

    // Utility
    bool isValidSquare(int row, int col) const;
    bool isSquareAttacked(int row, int col, PieceColor attackerColor) const;
    int findKing(PieceColor color, int& row, int& col) const;

    // Drawing helpers
    //const char* getPieceUnicode(PieceType type, PieceColor color) const;
    int getPieceCodepoint(PieceType type, PieceColor color) const;
    Color getSquareColor(int row, int col) const;
    
};

