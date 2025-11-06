#include "Chess.h"
#include <algorithm>
#include <cmath>
#include <string>

void ChessGame::init(int screenWidth, int screenHeight) {
    screenW = screenWidth;
    screenH = screenHeight;
    reset();
    const float margin = 40.0f;
    float size = (float)std::min(screenWidth - margin * 2, screenHeight - margin * 2 - 100);
    cellSize = size / BOARD_SIZE;
    float boardSize = cellSize * BOARD_SIZE;
    boardRect = { (screenWidth - boardSize) * 0.5f, (screenHeight - boardSize) * 0.5f - 20, boardSize, boardSize };
}

void ChessGame::reset() {
    initBoard();
    currentPlayer = PieceColor::WHITE;
    gameOver = false;
    AisCheckmate = false;
    AisStalemate = false;
    AisInCheck = false;
    winner = PieceColor::NONE;
    selectedRow = selectedCol = -1;
    highlightedMoves.clear();
    legalMoves.clear();

    whiteKingMoved = false;
    whiteRookLeftMoved = false;
    whiteRookRightMoved = false;
    blackKingMoved = false;
    blackRookLeftMoved = false;
    blackRookRightMoved = false;
    enPassantRow = enPassantCol = -1;
}

void ChessGame::initBoard() {
    // Clear board
    for (auto& row : board) {
        for (auto& piece : row) {
            piece = Piece{ PieceType::EMPTY, PieceColor::NONE };
        }
    }

    // White pieces (row 7)
    board[7][0] = { PieceType::ROOK, PieceColor::WHITE };
    board[7][1] = { PieceType::KNIGHT, PieceColor::WHITE };
    board[7][2] = { PieceType::BISHOP, PieceColor::WHITE };
    board[7][3] = { PieceType::QUEEN, PieceColor::WHITE };
    board[7][4] = { PieceType::KING, PieceColor::WHITE };
    board[7][5] = { PieceType::BISHOP, PieceColor::WHITE };
    board[7][6] = { PieceType::KNIGHT, PieceColor::WHITE };
    board[7][7] = { PieceType::ROOK, PieceColor::WHITE };

    // White pawns (row 6)
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[6][i] = { PieceType::PAWN, PieceColor::WHITE };
    }

    // Black pieces (row 0)
    board[0][0] = { PieceType::ROOK, PieceColor::BLACK };
    board[0][1] = { PieceType::KNIGHT, PieceColor::BLACK };
    board[0][2] = { PieceType::BISHOP, PieceColor::BLACK };
    board[0][3] = { PieceType::QUEEN, PieceColor::BLACK };
    board[0][4] = { PieceType::KING, PieceColor::BLACK };
    board[0][5] = { PieceType::BISHOP, PieceColor::BLACK };
    board[0][6] = { PieceType::KNIGHT, PieceColor::BLACK };
    board[0][7] = { PieceType::ROOK, PieceColor::BLACK };

    // Black pawns (row 1)
    for (int i = 0; i < BOARD_SIZE; ++i) {
        board[1][i] = { PieceType::PAWN, PieceColor::BLACK };
    }
}

Vector2 ChessGame::squareCenter(int row, int col) const {
    return { boardRect.x + col * cellSize + cellSize * 0.5f, boardRect.y + row * cellSize + cellSize * 0.5f };
}

int ChessGame::squareFromMouse(Vector2 m, int& row, int& col) const {
    if (!CheckCollisionPointRec(m, boardRect)) return 0;
    col = (int)((m.x - boardRect.x) / cellSize);
    row = (int)((m.y - boardRect.y) / cellSize);
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return 0;
    return 1;
}

bool ChessGame::isValidSquare(int row, int col) const {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

void ChessGame::generatePawnMoves(int row, int col, std::vector<Move>& moves) const {
    PieceColor color = board[row][col].color;
    bool isWhite = (color == PieceColor::WHITE);
    int direction = isWhite ? -1 : 1;
    int startRow = isWhite ? 6 : 1;

    // Forward one square
    int newRow = row + direction;
    if (isValidSquare(newRow, col) && board[newRow][col].isEmpty()) {
        moves.push_back(Move(row, col, newRow, col));
        // Forward two squares from starting position
        if (row == startRow && isValidSquare(newRow + direction, col) && board[newRow + direction][col].isEmpty()) {
            moves.push_back(Move(row, col, newRow + direction, col));
        }
    }

    // Capture diagonally
    for (int offset : {-1, 1}) {
        int newCol = col + offset;
        if (isValidSquare(newRow, newCol)) {
            if (!board[newRow][newCol].isEmpty() && board[newRow][newCol].color != color) {
                moves.push_back(Move(row, col, newRow, newCol));
            }
            // En passant
            if (enPassantRow == newRow && enPassantCol == newCol) {
                Move epMove(row, col, newRow, newCol);
                epMove.isEnPassant = true;
                moves.push_back(epMove);
            }
        }
    }
}

void ChessGame::generateRookMoves(int row, int col, std::vector<Move>& moves) const {
    PieceColor color = board[row][col].color;
    int directions[4][2] = { {-1,0}, {1,0}, {0,-1}, {0,1} };

    for (auto& dir : directions) {
        for (int i = 1; i < BOARD_SIZE; ++i) {
            int newRow = row + dir[0] * i;
            int newCol = col + dir[1] * i;
            if (!isValidSquare(newRow, newCol)) break;
            if (board[newRow][newCol].isEmpty()) {
                moves.push_back(Move(row, col, newRow, newCol));
            }
            else {
                if (board[newRow][newCol].color != color) {
                    moves.push_back(Move(row, col, newRow, newCol));
                }
                break;
            }
        }
    }
}

void ChessGame::generateBishopMoves(int row, int col, std::vector<Move>& moves) const {
    PieceColor color = board[row][col].color;
    int directions[4][2] = { {-1,-1}, {-1,1}, {1,-1}, {1,1} };

    for (auto& dir : directions) {
        for (int i = 1; i < BOARD_SIZE; ++i) {
            int newRow = row + dir[0] * i;
            int newCol = col + dir[1] * i;
            if (!isValidSquare(newRow, newCol)) break;
            if (board[newRow][newCol].isEmpty()) {
                moves.push_back(Move(row, col, newRow, newCol));
            }
            else {
                if (board[newRow][newCol].color != color) {
                    moves.push_back(Move(row, col, newRow, newCol));
                }
                break;
            }
        }
    }
}

void ChessGame::generateQueenMoves(int row, int col, std::vector<Move>& moves) const {
    generateRookMoves(row, col, moves);
    generateBishopMoves(row, col, moves);
}

void ChessGame::generateKnightMoves(int row, int col, std::vector<Move>& moves) const {
    PieceColor color = board[row][col].color;
    int moves_knight[8][2] = { {-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1} };

    for (auto& m : moves_knight) {
        int newRow = row + m[0];
        int newCol = col + m[1];
        if (isValidSquare(newRow, newCol)) {
            if (board[newRow][newCol].isEmpty() || board[newRow][newCol].color != color) {
                moves.push_back(Move(row, col, newRow, newCol));
            }
        }
    }
}

void ChessGame::generateKingMoves(int row, int col, std::vector<Move>& moves) const {
    PieceColor color = board[row][col].color;
    int directions[8][2] = { {-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1} };

    for (auto& dir : directions) {
        int newRow = row + dir[0];
        int newCol = col + dir[1];
        if (isValidSquare(newRow, newCol)) {
            if (board[newRow][newCol].isEmpty() || board[newRow][newCol].color != color) {
                moves.push_back(Move(row, col, newRow, newCol));
            }
        }
    }

    // Castling
    if (color == PieceColor::WHITE && !whiteKingMoved && row == 7 && col == 4) {
        // Kingside castling
        if (!whiteRookRightMoved && board[7][5].isEmpty() && board[7][6].isEmpty() && board[7][7].type == PieceType::ROOK && board[7][7].color == PieceColor::WHITE) {
            Move castlingMove(7, 4, 7, 6);
            castlingMove.isCastling = true;
            moves.push_back(castlingMove);
        }
        // Queenside castling
        if (!whiteRookLeftMoved && board[7][1].isEmpty() && board[7][2].isEmpty() && board[7][3].isEmpty() && board[7][0].type == PieceType::ROOK && board[7][0].color == PieceColor::WHITE) {
            Move castlingMove(7, 4, 7, 2);
            castlingMove.isCastling = true;
            moves.push_back(castlingMove);
        }
    }
    else if (color == PieceColor::BLACK && !blackKingMoved && row == 0 && col == 4) {
        // Kingside castling
        if (!blackRookRightMoved && board[0][5].isEmpty() && board[0][6].isEmpty() && board[0][7].type == PieceType::ROOK && board[0][7].color == PieceColor::BLACK) {
            Move castlingMove(0, 4, 0, 6);
            castlingMove.isCastling = true;
            moves.push_back(castlingMove);
        }
        // Queenside castling
        if (!blackRookLeftMoved && board[0][1].isEmpty() && board[0][2].isEmpty() && board[0][3].isEmpty() && board[0][0].type == PieceType::ROOK && board[0][0].color == PieceColor::BLACK) {
            Move castlingMove(0, 4, 0, 2);
            castlingMove.isCastling = true;
            moves.push_back(castlingMove);
        }
    }
}

void ChessGame::generateMovesForPiece(int row, int col, std::vector<Move>& moves) const {
    PieceType type = board[row][col].type;
    switch (type) {
    case PieceType::PAWN: generatePawnMoves(row, col, moves); break;
    case PieceType::ROOK: generateRookMoves(row, col, moves); break;
    case PieceType::BISHOP: generateBishopMoves(row, col, moves); break;
    case PieceType::QUEEN: generateQueenMoves(row, col, moves); break;
    case PieceType::KNIGHT: generateKnightMoves(row, col, moves); break;
    case PieceType::KING: generateKingMoves(row, col, moves); break;
    default: break;
    }
}

void ChessGame::generatePseudoLegalMoves(PieceColor color, std::vector<Move>& moves) const {
    moves.clear();
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (!board[r][c].isEmpty() && board[r][c].color == color) {
                generateMovesForPiece(r, c, moves);
            }
        }
    }
}

int ChessGame::findKing(PieceColor color, int& row, int& col) const {
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (board[r][c].type == PieceType::KING && board[r][c].color == color) {
                row = r; col = c;
                return 1;
            }
        }
    }
    return 0;
}

bool ChessGame::isSquareAttacked(int row, int col, PieceColor attackerColor) const {
    std::vector<Move> attackerMoves;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (!board[r][c].isEmpty() && board[r][c].color == attackerColor) {
                std::vector<Move> pieceMoves;
                generateMovesForPiece(r, c, pieceMoves);
                for (const auto& move : pieceMoves) {
                    if (move.toRow == row && move.toCol == col) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool ChessGame::isInCheck(PieceColor color) const {
    int kingRow, kingCol;
    if (!findKing(color, kingRow, kingCol)) return false;
    PieceColor attacker = (color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
    return isSquareAttacked(kingRow, kingCol, attacker);
}

bool ChessGame::isLegalMove(const Move& move, PieceColor color) const {
    // Save board state
    std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> savedBoard = board;
    bool savedWhiteKingMoved = whiteKingMoved;
    bool savedWhiteRookLeftMoved = whiteRookLeftMoved;
    bool savedWhiteRookRightMoved = whiteRookRightMoved;
    bool savedBlackKingMoved = blackKingMoved;
    bool savedBlackRookLeftMoved = blackRookLeftMoved;
    bool savedBlackRookRightMoved = blackRookRightMoved;
    int savedEnPassantRow = enPassantRow;
    int savedEnPassantCol = enPassantCol;

    // Temporarily make the move (const_cast needed because applyMove modifies state)
    ChessGame* nonConstThis = const_cast<ChessGame*>(this);
    Piece captured = nonConstThis->applyMove(move);

    // Check if this move leaves own king in check
    bool legal = !isInCheck(color);

    // Restore board state
    nonConstThis->board = savedBoard;
    nonConstThis->whiteKingMoved = savedWhiteKingMoved;
    nonConstThis->whiteRookLeftMoved = savedWhiteRookLeftMoved;
    nonConstThis->whiteRookRightMoved = savedWhiteRookRightMoved;
    nonConstThis->blackKingMoved = savedBlackKingMoved;
    nonConstThis->blackRookLeftMoved = savedBlackRookLeftMoved;
    nonConstThis->blackRookRightMoved = savedBlackRookRightMoved;
    nonConstThis->enPassantRow = savedEnPassantRow;
    nonConstThis->enPassantCol = savedEnPassantCol;

    return legal;
}

void ChessGame::validateMoves(std::vector<Move>& moves, PieceColor color) const {
    moves.erase(std::remove_if(moves.begin(), moves.end(),
        [this, color](const Move& m) { return !isLegalMove(m, color); }), moves.end());
}

bool ChessGame::hasLegalMoves(PieceColor color) const {
    std::vector<Move> moves;
    generatePseudoLegalMoves(color, moves);
    validateMoves(moves, color);
    return !moves.empty();
}

bool ChessGame::isCheckmate(PieceColor color) const {
    return isInCheck(color) && !hasLegalMoves(color);
}

bool ChessGame::isStalemate(PieceColor color) const {
    return !isInCheck(color) && !hasLegalMoves(color);
}

Piece ChessGame::applyMove(const Move& move) {
    Piece captured = board[move.toRow][move.toCol];
    bool isCastlingMove = move.isCastling || (board[move.fromRow][move.fromCol].type == PieceType::KING && abs(move.fromCol - move.toCol) == 2);

    // Handle castling
    if (isCastlingMove) {
        // Move king
        board[move.toRow][move.toCol] = board[move.fromRow][move.fromCol];
        board[move.fromRow][move.fromCol] = Piece{ PieceType::EMPTY, PieceColor::NONE };
        // Move rook
        if (move.toCol == 6) { // Kingside
            board[move.fromRow][5] = board[move.fromRow][7];
            board[move.fromRow][7] = Piece{ PieceType::EMPTY, PieceColor::NONE };
        }
        else if (move.toCol == 2) { // Queenside
            board[move.fromRow][3] = board[move.fromRow][0];
            board[move.fromRow][0] = Piece{ PieceType::EMPTY, PieceColor::NONE };
        }
    }
    // Handle en passant
    else if (move.isEnPassant || (board[move.fromRow][move.fromCol].type == PieceType::PAWN && move.toCol != move.fromCol && board[move.toRow][move.toCol].isEmpty())) {
        board[move.toRow][move.toCol] = board[move.fromRow][move.fromCol];
        board[move.fromRow][move.fromCol] = Piece{ PieceType::EMPTY, PieceColor::NONE };
        // Capture the pawn that was passed
        int capturedPawnRow = (board[move.toRow][move.toCol].color == PieceColor::WHITE) ? move.toRow + 1 : move.toRow - 1;
        if (isValidSquare(capturedPawnRow, move.toCol)) {
            board[capturedPawnRow][move.toCol] = Piece{ PieceType::EMPTY, PieceColor::NONE };
        }
    }
    // Handle pawn promotion
    else if (board[move.fromRow][move.fromCol].type == PieceType::PAWN && (move.toRow == 0 || move.toRow == 7)) {
        board[move.toRow][move.toCol] = Piece{ move.promotion, board[move.fromRow][move.fromCol].color };
        board[move.fromRow][move.fromCol] = Piece{ PieceType::EMPTY, PieceColor::NONE };
    }
    // Normal move
    else {
        board[move.toRow][move.toCol] = board[move.fromRow][move.fromCol];
        board[move.fromRow][move.fromCol] = Piece{ PieceType::EMPTY, PieceColor::NONE };
    }

    // Update castling rights
    if (board[move.toRow][move.toCol].type == PieceType::KING) {
        if (board[move.toRow][move.toCol].color == PieceColor::WHITE) whiteKingMoved = true;
        else blackKingMoved = true;
    }
    if (board[move.toRow][move.toCol].type == PieceType::ROOK) {
        if (move.fromRow == 7 && move.fromCol == 0) whiteRookLeftMoved = true;
        if (move.fromRow == 7 && move.fromCol == 7) whiteRookRightMoved = true;
        if (move.fromRow == 0 && move.fromCol == 0) blackRookLeftMoved = true;
        if (move.fromRow == 0 && move.fromCol == 7) blackRookRightMoved = true;
    }

    // Update en passant target
    enPassantRow = enPassantCol = -1;
    if (board[move.toRow][move.toCol].type == PieceType::PAWN && abs(move.toRow - move.fromRow) == 2) {
        enPassantRow = (move.fromRow + move.toRow) / 2;
        enPassantCol = move.toCol;
    }

    return captured;
}

void ChessGame::unmakeMove(const Move& move, Piece capturedPiece) {
    // Restore the board state - simplified version
    // For castling, we'd need to restore rook positions too, but this is called during minimax
    // where we'll use a different approach
}

void ChessGame::makeMove(const Move& move) {
    applyMove(move);
    currentPlayer = (currentPlayer == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
    selectedRow = selectedCol = -1;
    highlightedMoves.clear();
    checkGameState();
}

void ChessGame::checkGameState() {
    AisInCheck = isInCheck(currentPlayer);
    if (isCheckmate(currentPlayer)) {
        gameOver = true;
        AisCheckmate = true;
        winner = (currentPlayer == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
    }
    else if (isStalemate(currentPlayer)) {
        gameOver = true;
        AisStalemate = true;
    }
}

int ChessGame::getPieceValue(PieceType type) const {
    switch (type) {
    case PieceType::PAWN: return PAWN_VALUE;
    case PieceType::KNIGHT: return KNIGHT_VALUE;
    case PieceType::BISHOP: return BISHOP_VALUE;
    case PieceType::ROOK: return ROOK_VALUE;
    case PieceType::QUEEN: return QUEEN_VALUE;
    default: return 0;
    }
}

int ChessGame::evaluateBoard() const {
    int score = 0;
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            if (!board[r][c].isEmpty()) {
                int value = getPieceValue(board[r][c].type);
                if (board[r][c].color == PieceColor::WHITE) {
                    score += value;
                }
                else {
                    score -= value;
                }
            }
        }
    }
    return score;
}

int ChessGame::minimax(int depth, int alpha, int beta, bool maximizing, PieceColor color) {
    if (depth == 0) return evaluateBoard();

    bool inCheck = isInCheck(color);
    std::vector<Move> moves;
    generatePseudoLegalMoves(color, moves);
    validateMoves(moves, color);

    if (moves.empty()) {
        if (inCheck) return -1000000 + depth; // Checkmate
        return 0; // Stalemate
    }

    // Save board state
    std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> savedBoard = board;
    bool savedWhiteKingMoved = whiteKingMoved;
    bool savedWhiteRookLeftMoved = whiteRookLeftMoved;
    bool savedWhiteRookRightMoved = whiteRookRightMoved;
    bool savedBlackKingMoved = blackKingMoved;
    bool savedBlackRookLeftMoved = blackRookLeftMoved;
    bool savedBlackRookRightMoved = blackRookRightMoved;
    int savedEnPassantRow = enPassantRow;
    int savedEnPassantCol = enPassantCol;

    if (maximizing) {
        int best = -10000000;
        for (const auto& move : moves) {
            applyMove(move);
            int val = minimax(depth - 1, alpha, beta, false, (color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE);
            // Restore board state
            board = savedBoard;
            whiteKingMoved = savedWhiteKingMoved;
            whiteRookLeftMoved = savedWhiteRookLeftMoved;
            whiteRookRightMoved = savedWhiteRookRightMoved;
            blackKingMoved = savedBlackKingMoved;
            blackRookLeftMoved = savedBlackRookLeftMoved;
            blackRookRightMoved = savedBlackRookRightMoved;
            enPassantRow = savedEnPassantRow;
            enPassantCol = savedEnPassantCol;

            best = std::max(best, val);
            alpha = std::max(alpha, best);
            if (beta <= alpha) break;
        }
        return best;
    }
    else {
        int best = 10000000;
        for (const auto& move : moves) {
            applyMove(move);
            int val = minimax(depth - 1, alpha, beta, true, (color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE);
            // Restore board state
            board = savedBoard;
            whiteKingMoved = savedWhiteKingMoved;
            whiteRookLeftMoved = savedWhiteRookLeftMoved;
            whiteRookRightMoved = savedWhiteRookRightMoved;
            blackKingMoved = savedBlackKingMoved;
            blackRookLeftMoved = savedBlackRookLeftMoved;
            blackRookRightMoved = savedBlackRookRightMoved;
            enPassantRow = savedEnPassantRow;
            enPassantCol = savedEnPassantCol;

            best = std::min(best, val);
            beta = std::min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }
}

Move ChessGame::aiChooseMove() {
    std::vector<Move> moves;
    generatePseudoLegalMoves(PieceColor::BLACK, moves);
    validateMoves(moves, PieceColor::BLACK);

    if (moves.empty()) return Move(-1, -1, -1, -1);

    if (difficulty == GameDifficulty::EASY) {
        return moves[GetRandomValue(0, (int)moves.size() - 1)];
    }

    int maxDepth = (difficulty == GameDifficulty::MEDIUM) ? 2 : 4;
    int bestVal = -10000000;
    Move bestMove = moves[0];

    // Save board state
    std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> savedBoard = board;
    bool savedWhiteKingMoved = whiteKingMoved;
    bool savedWhiteRookLeftMoved = whiteRookLeftMoved;
    bool savedWhiteRookRightMoved = whiteRookRightMoved;
    bool savedBlackKingMoved = blackKingMoved;
    bool savedBlackRookLeftMoved = blackRookLeftMoved;
    bool savedBlackRookRightMoved = blackRookRightMoved;
    int savedEnPassantRow = enPassantRow;
    int savedEnPassantCol = enPassantCol;

    for (const auto& move : moves) {
        applyMove(move);
        int val = minimax(maxDepth - 1, -10000000, 10000000, false, PieceColor::WHITE);
        // Restore board state
        board = savedBoard;
        whiteKingMoved = savedWhiteKingMoved;
        whiteRookLeftMoved = savedWhiteRookLeftMoved;
        whiteRookRightMoved = savedWhiteRookRightMoved;
        blackKingMoved = savedBlackKingMoved;
        blackRookLeftMoved = savedBlackRookLeftMoved;
        blackRookRightMoved = savedBlackRookRightMoved;
        enPassantRow = savedEnPassantRow;
        enPassantCol = savedEnPassantCol;

        if (val > bestVal) {
            bestVal = val;
            bestMove = move;
        }
    }

    return bestMove;
}

void ChessGame::aiTurn() {
    Move move = aiChooseMove();
    if (move.fromRow >= 0) {
        makeMove(move);
    }
}

void ChessGame::update(GameState& stateOut) {
    if (IsKeyPressed(KEY_M)) { stateOut = GameState::STATE_MENU; return; }
    if (IsKeyPressed(KEY_R)) { reset(); return; }

    if (gameOver) return;

    if (currentPlayer == PieceColor::WHITE) {
        // Human turn
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 m = GetMousePosition();
            int row, col;
            if (squareFromMouse(m, row, col)) {
                if (selectedRow == -1) {
                    // Select piece
                    if (!board[row][col].isEmpty() && board[row][col].color == PieceColor::WHITE) {
                        selectedRow = row;
                        selectedCol = col;
                        highlightedMoves.clear();
                        generateMovesForPiece(row, col, highlightedMoves);
                        validateMoves(highlightedMoves, PieceColor::WHITE);
                    }
                }
                else {
                    // Try to make move
                    bool moveFound = false;
                    for (const auto& move : highlightedMoves) {
                        if (move.toRow == row && move.toCol == col) {
                            makeMove(move);
                            moveFound = true;
                            break;
                        }
                    }
                    if (!moveFound) {
                        // Deselect or select new piece
                        if (!board[row][col].isEmpty() && board[row][col].color == PieceColor::WHITE) {
                            selectedRow = row;
                            selectedCol = col;
                            highlightedMoves.clear();
                            generateMovesForPiece(row, col, highlightedMoves);
                            validateMoves(highlightedMoves, PieceColor::WHITE);
                        }
                        else {
                            selectedRow = selectedCol = -1;
                            highlightedMoves.clear();
                        }
                    }
                }
            }
        }
    }
    else {
        // AI turn
        aiTurn();
    }
}

Color ChessGame::getSquareColor(int row, int col) const {
    bool isLight = (row + col) % 2 == 0;
    return isLight ? Color{ 240, 217, 181, 255 } : Color{ 181, 136, 99, 255 };
}

//const char* ChessGame::getPieceUnicode(PieceType type, PieceColor color) const {
//    if (color == PieceColor::WHITE) {
//        switch (type) {
//        case PieceType::KING: return "\u0041"; // ♔
//        case PieceType::QUEEN: return "\u265a"; // ♕
//        case PieceType::ROOK: return "\u2656"; // ♖
//        case PieceType::BISHOP: return "\u2657"; // ♗
//        case PieceType::KNIGHT: return "\uFA04"; // ♘
//        case PieceType::PAWN: return "\u03B7"; // ♙
//        default: return "";
//        }
//    }
//    else {
//        switch (type) {
//        case PieceType::KING: return "\u03B7"; // ♚
//        case PieceType::QUEEN: return "\u265B"; // ♛
//        case PieceType::ROOK: return "\u265C"; // ♜
//        case PieceType::BISHOP: return "\u265D"; // ♝
//        case PieceType::KNIGHT: return "\u265E"; // ♞
//        case PieceType::PAWN: return "\u265F"; // ♟
//        default: return "";
//        }
//    }
//}
int ChessGame::getPieceCodepoint(PieceType type, PieceColor color) const {
    if (color == PieceColor::WHITE) {
        switch (type) {
        case PieceType::KING: return 0x2654; // ♔
        case PieceType::QUEEN: return 0x2655; // ♕
        case PieceType::ROOK: return 0x2656; // ♖
        case PieceType::BISHOP: return 0x2657; // ♗
        case PieceType::KNIGHT: return 0x2658; // ♘
        case PieceType::PAWN: return 0x2659; // ♙
        default: return 0;
        }
    }
    else {
        switch (type) {
        case PieceType::KING: return 0x265A; // ♚
        case PieceType::QUEEN: return 0x265B; // ♛
        case PieceType::ROOK: return 0x265C; // ♜
        case PieceType::BISHOP: return 0x265D; // ♝
        case PieceType::KNIGHT: return 0x265E; // ♞
        case PieceType::PAWN: return 0x265F; // ♟
        default: return 0;
        }
    }
}
void ChessGame::draw() const {
    // Draw board squares
    for (int r = 0; r < BOARD_SIZE; ++r) {
        for (int c = 0; c < BOARD_SIZE; ++c) {
            Rectangle square = { boardRect.x + c * cellSize, boardRect.y + r * cellSize, cellSize, cellSize };
            Color squareColor = getSquareColor(r, c);

            // Highlight selected square
            if (selectedRow == r && selectedCol == c) {
                squareColor = Color{ 255, 255, 0, 180 };
            }

            DrawRectangleRec(square, squareColor);

            // Draw piece
            //if (!board[r][c].isEmpty()) {
            //    Vector2 center = squareCenter(r, c);
            //    const char* pieceChar = getPieceUnicode(board[r][c].type, board[r][c].color);
            //    Color pieceColor = (board[r][c].color == PieceColor::WHITE) ? RAYWHITE : Color{ 50, 50, 50, 255 };
            //    float fontSize = cellSize * 0.7f;
            //    Vector2 textSize = MeasureTextEx(uiFont, pieceChar, fontSize, 0);
            //    Vector2 textPos = { center.x - textSize.x * 0.5f-15, center.y - textSize.y * 0.5f-18 };
            //    DrawTextCodepoint(uiFont, pieceChar, textPos, fontSize, 0, pieceColor);
            //}
            if (!board[r][c].isEmpty()) {
                Vector2 center = squareCenter(r, c);
                int codepoint = getPieceCodepoint(board[r][c].type, board[r][c].color);
                Color pieceColor = (board[r][c].color == PieceColor::WHITE) ? RAYWHITE : Color{ 50, 50, 50, 255 };
                float fontSize = cellSize * 0.7f;

                // Convert codepoint to UTF-8 string for measurement
                char utf8Char[5] = { 0 };
                int byteCount = 0;
                if (codepoint < 0x80) {
                    utf8Char[0] = (char)codepoint;
                    byteCount = 1;
                }
                else if (codepoint < 0x800) {
                    utf8Char[0] = (char)(0xC0 | (codepoint >> 6));
                    utf8Char[1] = (char)(0x80 | (codepoint & 0x3F));
                    byteCount = 2;
                }
                else if (codepoint < 0x10000) {
                    utf8Char[0] = (char)(0xE0 | (codepoint >> 12));
                    utf8Char[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                    utf8Char[2] = (char)(0x80 | (codepoint & 0x3F));
                    byteCount = 3;
                }
                else {
                    utf8Char[0] = (char)(0xF0 | (codepoint >> 18));
                    utf8Char[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
                    utf8Char[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                    utf8Char[3] = (char)(0x80 | (codepoint & 0x3F));
                    byteCount = 4;
                }

                Vector2 textSize = MeasureTextEx(uiFont, utf8Char, fontSize, 0);
                Vector2 textPos = { center.x - textSize.x * 0.5f, center.y - textSize.y * 0.5f };
                DrawTextCodepoint(uiFont, codepoint, textPos, fontSize, pieceColor);
            }
        }
    }

    // Draw highlighted moves
    for (const auto& move : highlightedMoves) {
        Rectangle square = { boardRect.x + move.toCol * cellSize, boardRect.y + move.toRow * cellSize, cellSize, cellSize };
        DrawRectangleRec(square, Color{ 255, 255, 0, 100 });
    }

    // Draw board border
    DrawRectangleLinesEx(boardRect, 3.0f, Color{ 100, 100, 100, 255 });

    // Status text
    std::string status;
    if (gameOver) {
        if (AisCheckmate) {
            status = (winner == PieceColor::WHITE) ? "Checkmate! You Win!" : "Checkmate! AI Wins!";
        }
        else if (AisStalemate) {
            status = "Stalemate! Draw!";
        }
    }
    else {
        status = (currentPlayer == PieceColor::WHITE) ? "Your turn (White)" : "AI thinking (Black)...";
        if (AisInCheck) {
            status += " - CHECK!";
        }
    }

    Vector2 ts = MeasureTextEx(uiFont, status.c_str(), 24.0f, 2.0f);
    Vector2 tp = { GetScreenWidth() * 0.5f - ts.x * 0.5f, boardRect.y + boardRect.height + 20 };
    DrawTextEx(uiFont, status.c_str(), tp, 24.0f, 2.0f, RAYWHITE);

    // Title and hint
    const char* title = "Chess";
    Vector2 tSize = MeasureTextEx(uiFont, title, 32.0f, 2.0f);
    DrawTextEx(uiFont, title, { GetScreenWidth() * 0.5f - tSize.x * 0.5f, boardRect.y - 60 }, 32.0f, 2.0f, RAYWHITE);

    const char* hint = "Click a piece to select. R to restart. M for Menu.";
    Vector2 hSize = MeasureTextEx(uiFont, hint, 20.0f, 2.0f);
    DrawTextEx(uiFont, hint, { GetScreenWidth() * 0.5f - hSize.x * 0.5f-30, boardRect.y-25 }, 25.0f, 2.0f, Color{ 200, 210, 225, 255 });
}

