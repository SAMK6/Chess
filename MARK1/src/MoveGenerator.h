#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include <stdint.h>
#include "BitBoard.h"


/*
    Moves will be from-to notation with 4 bits for extra info
    bits 0-5 are the start square, we can use these to bit shift and create the piece mask later
    bits 6-11 are the end square, 
    the remaining 4 bits are as follows:
        0 for quiet moves
        1 for double pawn push
        2 for kingside castle
        3 for queenside castle
        4 for captures
        5 for en passant captures
        8 for knight promo
        9 for bishop promo
        10 for rook promo
        11 for queen promo
        12 for knight promo-capture
        13 for bishop promo-capture
        14 for rook promo-capture
        15 for queen promo-capture

    need a use for 6 and 7 (maybe checks)

    the nullmove is all zeros

    I originally had this idea but didnt realise that bits 14-15 could still be used if the move is not a promotion,
    after reading CPW (https://www.chessprogramming.org/Encoding_Moves) I realised bits 14-15 could be used either way and 
    therefore my move representation is a modified version of the table used in the article linked
*/
typedef uint16_t Move;

// gonna define masks to get specific parts of the move object
#define startMask (Move)63 // first 6 bits (0-5)
#define endMask (Move)4032 // second 6 bits (6-11)
#define isCaptureMask (Move)16384 // 14th bit
#define isPromoMask (Move)32768 // 15th bit
#define pieceMask (Move)12288 // bits 12 and 13
#define miscMask (Move)61440 // bits 12-15

BitBoard makeMove(BitBoard, Move, char);

void generateMoves(BitBoard*, Move*); // functions receives a board and a preallocated array for the moves to be put into
void generateMovesBlack(BitBoard*, Move*);
void generateMovesWhite(BitBoard*, Move*);

#endif