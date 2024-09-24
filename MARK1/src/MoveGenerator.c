#include <stdint.h>
#include <stdio.h>
#include "BitBoard.h"
#include "MoveGenerator.h"
#include "BitMasks.h"


/*

    the general movegen strat will be to obtain a mask for the given piece,
    check if that piece is pinned to the king, obtain a movement mask for the peice, 
    depening on if it is pinned to the king and which squares are taken by friendly pieces we determine the possible moves

*/


// just found a new bug where if a rook is captured before that rook or the king moves
// the side who had it's rook captired will not loose castling privledges even though it should
BitBoard makeMove(BitBoard board, Move move, char piece){

    RawBoard *friendlyPieces, *enemyPieces;
    uint64_t *movedPiece;

    // break the move down into its parts
    uint64_t startSquare = 1ull << (move & startMask);
    uint64_t endSquare = 1ull << ((move & endMask) >> 6);
    uint64_t moveMask = startSquare | endSquare;
    uint16_t isCapture = move & isCaptureMask;
    uint16_t isPromo = move & isPromoMask;
    uint16_t promoPiece = (move & pieceMask) >> 12;
    uint16_t misc = (move & miscMask) >> 12;

    // choce the appropriate pieces for the move
    switch(piece){
        case 'K':
            friendlyPieces = &board.white;
            enemyPieces = &board.black;
            movedPiece = &friendlyPieces->k;
            break;
        case 'Q':
            friendlyPieces = &board.white;
            enemyPieces = &board.black;
            movedPiece = &friendlyPieces->q;
            break;
        case 'R':
            friendlyPieces = &board.white;
            enemyPieces = &board.black;
            movedPiece = &friendlyPieces->r;
            break;
        case 'B':
            friendlyPieces = &board.white;
            enemyPieces = &board.black;
            movedPiece = &friendlyPieces->b;
            break;
        case 'N':
            friendlyPieces = &board.white;
            enemyPieces = &board.black;
            movedPiece = &friendlyPieces->n;
            break;
        case 'P':
            friendlyPieces = &board.white;
            enemyPieces = &board.black;
            movedPiece = &friendlyPieces->p;
            break;
        case 'k':
            friendlyPieces = &board.black;
            enemyPieces = &board.white;
            movedPiece = &friendlyPieces->k;
            break;
        case 'q':
            friendlyPieces = &board.black;
            enemyPieces = &board.white;
            movedPiece = &friendlyPieces->q;
            break;
        case 'r':
            friendlyPieces = &board.black;
            enemyPieces = &board.white;
            movedPiece = &friendlyPieces->r;
            break;
        case 'b':
            friendlyPieces = &board.black;
            enemyPieces = &board.white;
            movedPiece = &friendlyPieces->b;
            break;
        case 'n':
            friendlyPieces = &board.black;
            enemyPieces = &board.white;
            movedPiece = &friendlyPieces->n;
            break;
        case 'p':
            friendlyPieces = &board.black;
            enemyPieces = &board.white;
            movedPiece = &friendlyPieces->p;
            break;
        default:
            break;
    }

    uint64_t enemyRooks = enemyPieces->r;

    // the move mask has a 1 on the start and end square and 0s everywhere else, 
    // so XORing it with the bitboard of the moved piece toggels off the start bit where the piece started 
    // and toggles on the end square where the piece ends
    *movedPiece ^= moveMask;

    // now if the move was a castling move we have to also move the appropriate rook
    // use the predefined masks for this since there are only 4 castling moves
    if(misc == (uint16_t)2){ // kingside castle
        if(piece == 'K'){
            friendlyPieces->r ^= whiteKingsideCastle;
        }
        else{
            friendlyPieces->r ^= blackKingsideCastle;
        }
    }

    if(misc == (uint16_t)3){ // queenside castle
        if(piece == 'K'){
            friendlyPieces->r ^= whiteQueensideCastle;
        }
        else{
            friendlyPieces->r ^= blackQueensideCastle;
        }
    }

    if(isCapture){ // if the move is a capture we have to remove the enemy piece from the square
        
        uint64_t notEndSquare;

        if(misc == (uint16_t)5){ // this is an enpassant capture

            if(board.whiteToMove){ // we are playing as white
                notEndSquare = ~(endSquare >> 8);
            }
            else{ // we are playing as black
                notEndSquare = ~(endSquare << 8);
            }
        }
        else{
            notEndSquare = ~endSquare;
        }

        // we can simply turn off this bit for every enemy piece because there cant be a piece there 
        // and this avoids control flow which would end up being more work overall
        enemyPieces->k &= notEndSquare;
        enemyPieces->q &= notEndSquare;
        enemyPieces->r &= notEndSquare;
        enemyPieces->b &= notEndSquare;
        enemyPieces->n &= notEndSquare;
        enemyPieces->p &= notEndSquare;

        // since we are already checking if the move is a capture move here and the board has been updated
        // we will also check if it was a rook that was captured and update castling rights
        if(enemyRooks != enemyPieces->r){ // an enemy rook was captured
            if(endSquare == H1){
                board.castling &= ~((uint8_t)8);
            }
            else if(endSquare == A1){
                board.castling &= ~((uint8_t)4);
            }
            else if(endSquare == H8){
                board.castling &= ~((uint8_t)2);
            }
            else if(endSquare == A8){
                board.castling &= ~((uint8_t)1);
            }
        }
    }

    if(isPromo){ // if the move is a promo we have to add a new piece and remove the pawn from the end of the board
        
        *movedPiece &= ~endSquare; // first remove the pawn

        switch(promoPiece){ // now based on the piece code add the piece we are promoting to

            case (uint16_t)0:
                friendlyPieces->n |= endSquare;
                break;
            case (uint16_t)1:
                friendlyPieces->b |= endSquare;
                break;
            case (uint16_t)2:
                friendlyPieces->r |= endSquare;
                break;
            case (uint16_t)3:
                friendlyPieces->q |= endSquare;
                break;
            default:
                break;

        }
    }

    // update the other data starting with move counter
    
    board.moves += board.whiteToMove ? 0 : 1; // if it was black that just moved we update the move counter

    // update the halfmove clock according to if the move was a pawn move capture or neither
    if(piece == 'p' || piece == 'P' || isCapture){
        board.halfMoves = 0;
    }
    else{
        board.halfMoves++;
    }

    // update the en passant square if it was a double pawn push
    if(misc == (uint16_t)1){
        if(board.whiteToMove){ // white just pushed a pawn 2 spaces
            board.enPassant = endSquare >> 8;
        }
        else{ // black just pushed a pawn 2 spaces
            board.enPassant = endSquare << 8;
        }
    }
    else{
        board.enPassant = 0ull;
    }

    // update castling rights
    if(board.castling){ // if noone can castle castling rights don't need to be updated
        if(piece == 'K'){
            board.castling &= ~((uint8_t)12);
        }
        else if(piece == 'k'){
            board.castling &= ~((uint8_t)3);
        }
        else if(startSquare == H1){
            board.castling &= ~((uint8_t)8);
        }
        else if(startSquare == A1){
            board.castling &= ~((uint8_t)4);
        }
        else if(startSquare == H8){
            board.castling &= ~((uint8_t)2);
        }
        else if(startSquare == A8){
            board.castling &= ~((uint8_t)1);
        }

    }
    
    // finally we switch the player to move
    board.whiteToMove = !board.whiteToMove;

    return board;

}

int isKingSafe(BitBoard *board){

    uint64_t targetKing = board->whiteToMove ? board->black.k : board->white.k;
    RawBoard *friendlyPieces = board->whiteToMove ? &board->white : &board->black;
    RawBoard *enemyPieces = board->whiteToMove ? &board->black : &board->white;

    uint64_t wholeBoard = enemyPieces->p | enemyPieces->n | enemyPieces->b | enemyPieces->r | enemyPieces->q | enemyPieces->k | friendlyPieces->p | friendlyPieces->n | friendlyPieces->b | friendlyPieces->r | friendlyPieces->q | friendlyPieces->k;

    uint8_t kingPos = __bulitin_ctzll(targetKing);

    if(basicKnightMasks[kingPos] & friendlyPieces->n) return 0; // check if we can take the king with a knight

    uint64_t bishopAttacks = generateBishopAttacks(kingPos, wholeBoard);
    if(bishopAttacks & friendlyPieces->b) return 0; // check if we can take the king with a bishop

    uint64_t rookAttacks = generateRookAttacks(kingPos, wholeBoard);
    if(rookAttacks & friendlyPieces->r) return 0; // check if we can take the king with a rook

    if((rookAttacks | bishopAttacks) & friendlyPieces->q) return 0; // check if we can take the king with a queen

    uint64_t pawnAttacks = board->whiteToMove ? generatePawnMaskBlack(kingPos) : generatePawnMaskWhite(kingPos);
    if(pawnAttacks & friendlyPieces->p) return 0;

    return 1;

}

int generateMovesWhite(BitBoard *board, MoveBoard *moves){

    // a board that simply has the position of all oponnent pieces
    uint64_t opBoard = board->black.k | board->black.q | board->black.r | board->black.b | board->black.n | board->black.p;
    // a board that contains all friendly pieces
    uint64_t myBoard = board->white.k | board->white.q | board->white.r | board->white.b | board->white.n | board->white.p;
    // a board that contains all pieces
    uint64_t wholeBoard = opBoard | myBoard;


    RawBoard friendlyPieces = board->white;
    RawBoard enemyPieces = board->black;

    int pos = 0; // where in the movelist to put moves


    uint64_t currentSquare;
    for(uint8_t i = 0; i < 64; i++){

        currentSquare = 1ull << i;

        if(currentSquare & friendlyPieces.p){ // found a pawn

            if(!((currentSquare << 8) & wholeBoard)){ // there is nothing stopping a single pawn push
                if(i < (uint8_t)48){ // no promotion
                    
                    (moves + pos)->move = buildMove(i, i + 8, 0);
                    (moves + pos)->board = makeMove(*board, (moves + pos)->move, 'P');
                    pos++;

                }
                else{

                    (moves + pos)->move = buildMove(i, i + 8, 8);
                    (moves + pos)->board = makeMove(*board, (moves + pos)->move, 'P');
                    pos++;

                    (moves + pos)->move = buildMove(i, i + 8, 9);
                    (moves + pos)->board = makeMove(*board, (moves + pos)->move, 'P');
                    pos++;

                    (moves + pos)->move = buildMove(i, i + 8, 10);
                    (moves + pos)->board = makeMove(*board, (moves + pos)->move, 'P');
                    pos++;

                    (moves + pos)->move = buildMove(i, i + 8, 11);
                    (moves + pos)->board = makeMove(*board, (moves + pos)->move, 'P');
                    pos++;
                }
            }

        }
        else if(currentSquare & friendlyPieces.n){ // found a knight

        }
        else if(currentSquare & friendlyPieces.b){ // found a bishop

        }
        else if(currentSquare & friendlyPieces.r){ // found a rook

        }
        else if(currentSquare & friendlyPieces.q){ // found a queen

        }
        else if(currentSquare & friendlyPieces.k){ // found a king

        }

    }


}

int generateMovesBlack(BitBoard *board, MoveBoard *moves){




    
}
