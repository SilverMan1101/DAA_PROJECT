#pragma once

// Input board is a flat 9-length char array with values: ' ', 'X', 'O'
// Returns index 0..8 for best move for 'O' (AI). Returns -1 if no moves left.
int AI_FindBestMove(const char board[9]);


