#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <cstdio>
#include <deque>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN 1
#define WINDOWS_NO_MINMAX 1
#include <windows.h>
#endif

using namespace std;

struct piece {
    bitset<2> color;
    char piece;
    int worth;
};
piece emptypiece = {0b00, ' ', 0};

map<char, string> piecesmap = {
    {'r', "♖"}, {'n', "♘"}, {'b', "♗"}, {'q', "♕"}, {'k', "♔"}, {'p', "♙"},

    {'R', "♜"}, {'N', "♞"}, {'B', "♝"}, {'Q', "♛"}, {'K', "♚"}, {'P', "♟"},

    {' ', " "},
};

struct move {
    piece *piece;
    unsigned int fromrow : 3;
    unsigned int fromcol : 3;

    unsigned int torow : 3;
    unsigned int tocol : 3;

    int enpassantrow : 4 = -2;
    int enpassantcol : 4 = -2;

    char promotion = ' ';

    bool castling : 1 = false;

    bool attacking : 1 = true;

    int fap[2]{-2, -2};

    int eval = 0;
};

struct board {
    piece pieces[8][8]{
        {{0b01, 'r', 500},
         {0b01, 'n', 300},
         {0b01, 'b', 300},
         {0b01, 'q', 100000},
         {0b01, 'k', 900},
         {0b01, 'b', 300},
         {0b01, 'n', 300},
         {0b01, 'r', 500}},
        {{0b01, 'p', 100},
         {0b01, 'p', 100},
         {0b01, 'p', 100},
         {0b01, 'p', 100},
         {0b01, 'p', 100},
         {0b01, 'p', 100},
         {0b01, 'p', 100},
         {0b01, 'p', 100}},

        {{0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0}},
        {{0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0}},
        {{0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0}},
        {{0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0},
         {0b00, ' ', 0}},

        {{0b11, 'p', 100},
         {0b11, 'p', 100},
         {0b11, 'p', 100},
         {0b11, 'p', 100},
         {0b11, 'p', 100},
         {0b11, 'p', 100},
         {0b11, 'p', 100},
         {0b11, 'p', 100}},
        {{0b11, 'r', 500},
         {0b11, 'n', 300},
         {0b11, 'b', 300},
         {0b11, 'q', 100000},
         {0b11, 'k', 900},
         {0b11, 'b', 300},
         {0b11, 'n', 300},
         {0b11, 'r', 500}},
    };

    bool whosetomove = false;

    bool whoami = false;

    bitset<4> castling{"1111"};

    int enpassant[2]{-2, -2};

    void move(string const &move) {
        ::move cmove;

        if (pieces[move[1] - '1'][move[0] - 'a'].piece == 'k' &&
            (move == "e1h1" || move == "e1a1" || move == "e8h8" || move == "e8a8")) {
            // castling
            cmove = {&pieces[move[1] - '1'][move[0] - 'a'],
                     static_cast<unsigned int>(move[1] - '1'),
                     static_cast<unsigned int>(move[0] - 'a'),
                     static_cast<unsigned int>(move[3] - '1'),
                     static_cast<unsigned int>(move[2] - 'a'),
                     -2,
                     -2,
                     ' ',
                     true};
        } else if (pieces[move[1] - '1'][move[0] - 'a'].piece == 'p' && move[0] != move[2] &&
                   pieces[move[3] - '1'][move[2] - 'a'].piece == ' ') { // (if capturing empty piece)
            // enpassant
            cmove = {&pieces[move[1] - '1'][move[0] - 'a'],
                     static_cast<unsigned int>(move[1] - '1'),
                     static_cast<unsigned int>(move[0] - 'a'),
                     static_cast<unsigned int>(move[3] - '1'),
                     static_cast<unsigned int>(move[2] - 'a'),
                     move[3] - '1',
                     move[2] - 'a',
                     ' ',
                     false};
        } else if (move.length() == 5) {
            // promotion
            cmove = {&pieces[move[1] - '1'][move[0] - 'a'],
                     static_cast<unsigned int>(move[1] - '1'),
                     static_cast<unsigned int>(move[0] - 'a'),
                     static_cast<unsigned int>(move[3] - '1'),
                     static_cast<unsigned int>(move[2] - 'a'),
                     -2,
                     -2,
                     move[4],
                     false};
        } else {
            // if not any of the weird cases, just move the piece
            cmove = {&pieces[move[1] - '1'][move[0] - 'a'], static_cast<unsigned int>(move[1] - '1'),
                     static_cast<unsigned int>(move[0] - 'a'), static_cast<unsigned int>(move[3] - '1'),
                     static_cast<unsigned int>(move[2] - 'a')};
        }

        this->move(cmove);
    }

    void move(::move const &move) {
        // auto revoke enpassantability
        enpassant[0] = -2;
        enpassant[1] = -2;

        // auto grant enpassantability
        if (move.piece->piece == 'p') {
            if (move.fromrow == 1 && move.torow == 3) {
                enpassant[0] = move.fromcol;
                enpassant[1] = move.fromrow + 1;
            } else if (move.fromrow == 6 && move.torow == 4) {
                enpassant[0] = move.fromcol;
                enpassant[1] = move.fromrow - 1;
            }
        }

        // auto revoke castling rights
        if (move.piece->piece == 'k') {
            if (move.piece->color[1] == 0) {
                castling[0] = 0;
                castling[1] = 0;
            } else {
                castling[2] = 0;
                castling[3] = 0;
            }
        } else if (move.piece->piece == 'r') {
            if (move.piece->color[1] == 0) {
                if (move.fromrow == 0 && move.fromcol == 0) {
                    // left white rook
                    castling[1] = 0;
                } else if (move.fromrow == 0 && move.fromcol == 7) {
                    // right white rook
                    castling[0] = 0;
                }
            } else {
                if (move.fromrow == 7 && move.fromcol == 0) {
                    // left black rook
                    castling[3] = 0;
                } else if (move.fromrow == 7 && move.fromcol == 7) {
                    // right black rook
                    castling[2] = 0;
                }
            }
        }
        if (move.torow == 0 && move.tocol == 0) {
            // left white rook
            castling[1] = 0;
        } else if (move.torow == 0 && move.tocol == 7) {
            // right white rook
            castling[0] = 0;
        } else if (move.torow == 7 && move.tocol == 0) {
            // left black rook
            castling[3] = 0;
        } else if (move.torow == 7 && move.tocol == 7) {
            // right black rook
            castling[2] = 0;
        }

        if (move.castling) {
            // castling
            if (move.tocol == 7) {
                // kingside

                // move king
                pieces[move.torow][6] = *move.piece;
                pieces[move.fromrow][move.fromcol] = emptypiece;

                // move rook
                pieces[move.torow][5] = pieces[move.torow][7];
                pieces[move.torow][7] = emptypiece;
            } else if (move.tocol == 0) {
                // queenside

                // move king
                pieces[move.torow][2] = *move.piece;
                pieces[move.fromrow][move.fromcol] = emptypiece;

                // move rook
                pieces[move.torow][3] = pieces[move.torow][0];
                pieces[move.torow][0] = emptypiece;
            }
        } else if (move.enpassantcol != -2) {
            // enpassant

            // move pawn
            pieces[move.torow][move.tocol] = *move.piece;
            pieces[move.fromrow][move.fromcol] = emptypiece;

            // remove captured pawn
            pieces[move.enpassantrow == 5 ? 4 : 3][move.enpassantcol] = emptypiece;
        } else if (move.promotion != ' ') {
            // promotion

            // move pawn
            pieces[move.torow][move.tocol] = *move.piece;
            pieces[move.fromrow][move.fromcol] = emptypiece;

            // change piece.piece to requested promotion piece
            pieces[move.torow][move.tocol].piece = (char)tolower(move.promotion);
        }

        else {
            // if not any of the weird cases, just move the piece
            pieces[move.torow][move.tocol] = *move.piece;
            pieces[move.fromrow][move.fromcol] = emptypiece;
        }

        // whosetomove = !whosetomove;

        // cout << "Moving from " << (char)(move.fromcol + 'a') << (char)(move.fromrow + '1') << "→"
        //      << (char)(move.tocol + 'a') << (char)(move.torow + '1') << endl;
    }

    void actuallyMove(::move const &move) {
        cout << (char)(move.fromcol + 'a') << (char)(move.fromrow + '1') << (char)(move.tocol + 'a')
             << (char)(move.torow + '1');

        if (move.promotion != ' ') {
            cout << (char)toupper(move.promotion);
        }

        this->move(move);
        toggleWhoseToMove();

        cout << endl;
    }

    void toggleWhoseToMove() {
        whosetomove = !whosetomove;
    };

    void draw() {
        // return;

        cerr << "row\r\n↓";
        for (int i = 0; i < 9; i++) {
            cerr << "\x1B[49m" << endl;
            if (i == 8) {
                // cerr << "\x1B[49m  \uFF10\uFF11\uFF12\uFF13\uFF14\uFF15\uFF16\uFF17 ← col  " << endl;
                cerr << "\x1B[49m  \uFF41\uFF42\uFF43\uFF44\uFF45\uFF46\uFF47\uFF48 ← col  " << endl;
            } else {
                // string numbs[8] = {"\uFF10", "\uFF11", "\uFF12", "\uFF13", "\uFF14", "\uFF15", "\uFF16", "\uFF17"};
                string numbs[8] = {"\uFF11", "\uFF12", "\uFF13", "\uFF14", "\uFF15", "\uFF16", "\uFF17", "\uFF18"};
                cerr << "\x1B[49m" << numbs[7 - i];
                for (int k = 0; k < 8; ++k) {
                    piece &piece = pieces[7 - i][k];
                    cerr << ((i % 2 ? !(k % 2) : (k % 2)) ? "\x1B[40m" : "\x1B[100m")                      // color
                         << piecesmap[(char)(piece.color[1] ? piece.piece : toupper(piece.piece))] << " "; // piece
                }
            }
        }
    }

    // int calcworth(int row, int col) {
    //     return 0;
    // }

    // warning: highly inefficient function, it was more or less only meant for testing, don't even try to read it,
    // it'll definitely hurt your eyes
    string toFen() {
        char fen[90];

        struct {
            unsigned int fenpos : 7;
            unsigned int empty : 3;
        } state = {0, 0};

        // for (piece(&row)[8] : pieces) {
        for (int i = 0; i < 8; i++) {
            piece(&row)[8] = pieces[7 - i];
            for (piece &piece : row) {
                cout << state.fenpos << " " << state.empty << endl;
                if (piece.color[0] == 0) {
                    state.empty++;
                    continue;
                }
                if (state.empty) {
                    fen[state.fenpos++] = state.empty + '0';
                    state.empty = 0;
                }
                if (piece.color[1] == 0) {
                    fen[state.fenpos++] = toupper(piece.piece);
                    continue;
                } else if (piece.color[1] == 1) {
                    fen[state.fenpos++] = tolower(piece.piece);
                    continue;
                }
            }
            if (state.empty) {
                fen[state.fenpos++] = state.empty + '0';
                state.empty = 0;
            }
            if (i < 7)
                fen[state.fenpos++] = '/';
            else
                fen[state.fenpos++] = ' ';
        }
        fen[state.fenpos++] = whosetomove ? 'b' : 'w';
        fen[state.fenpos++] = ' ';
        if (castling[0])
            fen[state.fenpos++] = 'K';
        if (castling[1])
            fen[state.fenpos++] = 'Q';
        if (castling[2])
            fen[state.fenpos++] = 'k';
        if (castling[3])
            fen[state.fenpos++] = 'q';
        if (fen[state.fenpos - 1] == ' ')
            fen[state.fenpos++] = '-';

        return fen;
    };

    [[clang::always_inline]] inline void possibleMoves(const int rownumb, const int colnumb, deque<::move> &movesm,
                                                       deque<::move> &moveso, int &kingrow, int &kingcol) {
        piece(&row)[8] = pieces[rownumb];
        piece &piece = row[colnumb];

        // cout << whosetomove << endl;
        if (piece.color[0] == 0)
            return;

        deque<::move> &moves = (piece.color[1] != whosetomove) ? moveso : movesm;
        bool imo = piece.color[1] != whosetomove;
        // bool imo = false;

        // if (piece.color[1] != whosetomove)
        // continue;

        switch (piece.piece) {
        case 'p': {
            if (piece.color[1]) {
                // black

                // simply 1 forward
                if (rownumb - 1 > -1 && pieces[rownumb - 1][colnumb].color[0] == 0) {
                    if (rownumb - 1 == 0) {
                        // promotion
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'q', false, false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'n', false, false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'b', false, false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'r', false, false});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, ' ', false, false});
                    }

                    // 2 forward, but only if on starting row & not going through another piece
                    if (rownumb == 6 && rownumb - 2 > -1 && pieces[rownumb - 2][colnumb].color[0] == 0) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 2), static_cast<unsigned int>(colnumb), -2,
                                         -2, ' ', false, false});
                    }
                };

                // capturing, left or right
                if (rownumb - 1 > -1 && colnumb - 1 > -1 &&
                    ((pieces[rownumb - 1][colnumb - 1].color[0] == 1 &&
                      pieces[rownumb - 1][colnumb - 1].color[1] != piece.color[1]) ||
                     imo)) {
                    if (rownumb - 1 == 0) {
                        // promotion
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'q', false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'n', false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'b', false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'r', false});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1),
                                         static_cast<unsigned int>(colnumb - 1)});
                    }
                }
                if (rownumb - 1 > -1 && colnumb + 1 < 8 &&
                    ((pieces[rownumb - 1][colnumb + 1].color[0] == 1 &&
                      pieces[rownumb - 1][colnumb + 1].color[1] != piece.color[1]) ||
                     imo)) {
                    if (rownumb - 1 == 0) {
                        // promotion
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'q', false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'n', false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'b', false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'r', false});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - 1),
                                         static_cast<unsigned int>(colnumb + 1)});
                    }
                }

                // en passant
                if (rownumb == 3 && ((colnumb - 1) == enpassant[0] || (colnumb + 1) == enpassant[0])) {
                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(2), static_cast<unsigned int>(enpassant[0]), 2,
                                     enpassant[0]});
                }
            } else {
                // white

                // moving, simply 1 forward
                if (rownumb + 1 < 8 && pieces[rownumb + 1][colnumb].color[0] == 0) {
                    if (rownumb + 1 == 7) {
                        // promotion
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'q', false, false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'n', false, false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'b', false, false});

                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, 'r', false, false});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb), -2,
                                         -2, ' ', false, false});
                    }

                    // 2 forward, but only if on starting row & not going through another piece
                    if (rownumb == 1 && rownumb + 2 < 8 && pieces[rownumb + 2][colnumb].color[0] == 0) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 2), static_cast<unsigned int>(colnumb), -2,
                                         -2, ' ', false, false});
                    }
                };

                // capturing, left or right
                if (rownumb + 1 < 8 && colnumb - 1 > -1 &&
                    ((pieces[rownumb + 1][colnumb - 1].color[0] == 1 &&
                      pieces[rownumb + 1][colnumb - 1].color[1] != piece.color[1]) ||
                     imo)) {
                    // left
                    if (rownumb == 6) {
                        // promotion
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'q', false});
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'n', false});
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'b', false});
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb - 1),
                                         -2, -2, 'r', false});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1),
                                         static_cast<unsigned int>(colnumb - 1)});
                    }
                };
                if (rownumb + 1 < 8 && colnumb + 1 < 8 &&
                    ((pieces[rownumb + 1][colnumb + 1].color[0] == 1 &&
                      pieces[rownumb + 1][colnumb + 1].color[1] != piece.color[1]) ||
                     imo)) {
                    // right
                    if (rownumb == 6) {
                        // promotion
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'q', false});
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'n', false});
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'b', false});
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb + 1),
                                         -2, -2, 'r', false});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + 1),
                                         static_cast<unsigned int>(colnumb + 1)});
                    };
                };

                // en passant
                if (rownumb == 4 && ((colnumb - 1) == enpassant[0] || (colnumb + 1) == enpassant[0])) {
                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(5), static_cast<unsigned int>(enpassant[0]), 5,
                                     enpassant[0]});
                }
            }
            break;
        }
        case 'k': {
            if (!imo) {
                kingrow = rownumb;
                kingcol = colnumb;
            }
        }
        case 'q':
        case 'r': {
            // left
            for (int i = colnumb - 1; i > -1; i--) {
                if (imo) {
                    if (pieces[rownumb][i].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb), static_cast<unsigned int>(i)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb), static_cast<unsigned int>(i),
                                         .fap = {rownumb, i}});

                        // if (!(pieces[rownumb][i].piece == 'k' && pieces[rownumb][i].color !=
                        // piece.color))

                        if (!(pieces[rownumb][i].piece == 'k' && pieces[rownumb][i].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[rownumb][i].color[0] == 0b1 /* kenobi */ &&
                        pieces[rownumb][i].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(rownumb), static_cast<unsigned int>(i)});

                    if (pieces[rownumb][i].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            // right
            for (int i = colnumb + 1; i < 8; i++) {
                if (imo) {
                    if (pieces[rownumb][i].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb), static_cast<unsigned int>(i)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb), static_cast<unsigned int>(i),
                                         .fap = {rownumb, i}});

                        if (!(pieces[rownumb][i].piece == 'k' && pieces[rownumb][i].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[rownumb][i].color[0] == 0b1 /* kenobi */ &&
                        pieces[rownumb][i].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(rownumb), static_cast<unsigned int>(i)});

                    if (pieces[rownumb][i].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            // up
            for (int i = rownumb + 1; i < 8; i++) {
                if (imo) {
                    if (pieces[i][colnumb].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(i), static_cast<unsigned int>(colnumb)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(i), static_cast<unsigned int>(colnumb),
                                         .fap = {i, colnumb}});

                        if (!(pieces[i][colnumb].piece == 'k' && pieces[i][colnumb].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[i][colnumb].color[0] == 0b1 /* kenobi */ &&
                        pieces[i][colnumb].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(i), static_cast<unsigned int>(colnumb)});

                    if (pieces[i][colnumb].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            // down
            for (int i = rownumb - 1; i > -1; i--) {
                if (imo) {
                    if (pieces[i][colnumb].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(i), static_cast<unsigned int>(colnumb)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(i), static_cast<unsigned int>(colnumb),
                                         .fap = {i, colnumb}});

                        if (!(pieces[i][colnumb].piece == 'k' && pieces[i][colnumb].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[i][colnumb].color[0] == 0b1 /* kenobi */ &&
                        pieces[i][colnumb].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(i), static_cast<unsigned int>(colnumb)});

                    if (pieces[i][colnumb].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            if (piece.piece == 'r')
                break;

            [[fallthrough]];
        }
        case 'b': {
            // left up
            for (int i = 1; i <= min(7 - rownumb, colnumb); i++) {
                if (imo) {
                    if (pieces[rownumb + i][colnumb - i].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + i),
                                         static_cast<unsigned int>(colnumb - i)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + i), static_cast<unsigned int>(colnumb - i),
                                         .fap = {rownumb + i, colnumb - i}});

                        if (!(pieces[rownumb + i][colnumb - i].piece == 'k' &&
                              pieces[rownumb + i][colnumb - i].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[rownumb + i][colnumb - i].color[0] == 0b1 /* kenobi */ &&
                        pieces[rownumb + i][colnumb - i].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(rownumb + i), static_cast<unsigned int>(colnumb - i)});

                    if (pieces[rownumb + i][colnumb - i].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            // left down
            for (int i = 1; i <= min(rownumb, colnumb); i++) {
                if (imo) {
                    if (pieces[rownumb - i][colnumb - i].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - i),
                                         static_cast<unsigned int>(colnumb - i)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - i), static_cast<unsigned int>(colnumb - i),
                                         .fap = {rownumb - i, colnumb - i}});

                        if (!(pieces[rownumb - i][colnumb - i].piece == 'k' &&
                              pieces[rownumb - i][colnumb - i].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[rownumb - i][colnumb - i].color[0] == 0b1 /* kenobi */ &&
                        pieces[rownumb - i][colnumb - i].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(rownumb - i), static_cast<unsigned int>(colnumb - i)});

                    if (pieces[rownumb - i][colnumb - i].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            // right up
            for (int i = 1; i <= min(7 - rownumb, 7 - colnumb); i++) {
                if (imo) {
                    if (pieces[rownumb + i][colnumb + i].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + i),
                                         static_cast<unsigned int>(colnumb + i)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb + i), static_cast<unsigned int>(colnumb + i),
                                         .fap = {rownumb + i, colnumb + i}});

                        if (!(pieces[rownumb + i][colnumb + i].piece == 'k' &&
                              pieces[rownumb + i][colnumb + i].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[rownumb + i][colnumb + i].color[0] == 0b1 /* kenobi */ &&
                        pieces[rownumb + i][colnumb + i].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(rownumb + i), static_cast<unsigned int>(colnumb + i)});

                    if (pieces[rownumb + i][colnumb + i].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            // right down
            for (int i = 1; i <= min(rownumb, 7 - colnumb); i++) {
                if (imo) {
                    if (pieces[rownumb - i][colnumb + i].color == 0b00) {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - i),
                                         static_cast<unsigned int>(colnumb + i)});
                    } else {
                        moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                         static_cast<unsigned int>(rownumb - i), static_cast<unsigned int>(colnumb + i),
                                         .fap = {rownumb - i, colnumb + i}});

                        if (!(pieces[rownumb - i][colnumb + i].piece == 'k' &&
                              pieces[rownumb - i][colnumb + i].color != piece.color))
                            break;
                    }
                } else {
                    if (pieces[rownumb - i][colnumb + i].color[0] == 0b1 /* kenobi */ &&
                        pieces[rownumb - i][colnumb + i].color[1] == piece.color[1])
                        break;

                    moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                     static_cast<unsigned int>(rownumb - i), static_cast<unsigned int>(colnumb + i)});

                    if (pieces[rownumb - i][colnumb + i].color[0] == 0b1)
                        break;
                }

                if (piece.piece == 'k')
                    break;
            }

            break;
        }
        case 'n': {
            //  _   (rownumb + 2, colnumb - 1)
            //   |
            //   |
            if (rownumb + 2 < 8 && colnumb - 1 > -1 &&
                (pieces[rownumb + 2][colnumb - 1].color[0] == 0b0 ||
                 pieces[rownumb + 2][colnumb - 1].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb + 2), static_cast<unsigned int>(colnumb - 1)});

            //  _   (rownumb + 2, colnumb + 1)
            // |
            // |
            if (rownumb + 2 < 8 && colnumb + 1 < 8 &&
                (pieces[rownumb + 2][colnumb + 1].color[0] == 0b0 ||
                 pieces[rownumb + 2][colnumb + 1].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb + 2), static_cast<unsigned int>(colnumb + 1)});

            // |__ (colnumb - 2, rownumb + 1)
            if (colnumb - 2 > -1 && rownumb + 1 < 8 &&
                (pieces[rownumb + 1][colnumb - 2].color[0] == 0b0 ||
                 pieces[rownumb + 1][colnumb - 2].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb - 2)});

            //  __ (colnumb - 2, rownumb - 1)
            // |
            if (colnumb - 2 > -1 && rownumb - 1 > -1 &&
                (pieces[rownumb - 1][colnumb - 2].color[0] == 0b0 ||
                 pieces[rownumb - 1][colnumb - 2].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb - 2)});

            // |
            // |__ (colnumb + 1, rownumb - 2)
            if (colnumb + 1 < 8 && rownumb - 2 > -1 &&
                (pieces[rownumb - 2][colnumb + 1].color[0] == 0b0 ||
                 pieces[rownumb - 2][colnumb + 1].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb - 2), static_cast<unsigned int>(colnumb + 1)});

            // |
            // |
            //_  (colnumb - 1, rownumb - 2)
            if (colnumb - 1 > -1 && rownumb - 2 > -1 &&
                (pieces[rownumb - 2][colnumb - 1].color[0] == 0b0 ||
                 pieces[rownumb - 2][colnumb - 1].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb - 2), static_cast<unsigned int>(colnumb - 1)});

            // __| (colnumb + 2, rownumb + 1)
            if (colnumb + 2 < 8 && rownumb + 1 < 8 &&
                (pieces[rownumb + 1][colnumb + 2].color[0] == 0b0 ||
                 pieces[rownumb + 1][colnumb + 2].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb + 1), static_cast<unsigned int>(colnumb + 2)});

            // __
            //   | (colnumb + 2, rownumb - 1)
            if (colnumb + 2 < 8 && rownumb - 1 > -1 &&
                (pieces[rownumb - 1][colnumb + 2].color[0] == 0b0 ||
                 pieces[rownumb - 1][colnumb + 2].color[1] != piece.color[1] || imo))
                moves.push_back({&piece, static_cast<unsigned int>(rownumb), static_cast<unsigned int>(colnumb),
                                 static_cast<unsigned int>(rownumb - 1), static_cast<unsigned int>(colnumb + 2)});
            break;
        }
        }
    }

    deque<::move> possibleMoves(const vector<tuple<int, int>> &selectPieces = {{-2, -2}}, bool select = false) {
        deque<::move> movesm;
        deque<::move> moveso;

        int kingrow = -2;
        int kingcol = -2;

        // normal piece movement
        if (!select) {
            for (int rownumb = 0; rownumb < 8; ++rownumb) {
                for (int colnumb = 0; colnumb < 8; ++colnumb) {
                    possibleMoves(rownumb, colnumb, movesm, moveso, kingrow, kingcol);
                }
            }
        } else {
            for (auto &sP : selectPieces) {
                possibleMoves(get<0>(sP), get<1>(sP), movesm, moveso, kingrow, kingcol);
            }
        }
        if (select)
            return moveso;

        deque<::move> &moves = movesm;

        // legality check
        bool kingIsInCheck = false;
        vector<tuple<int, int>> toselectPieces;
        if (kingcol != -2) {
            for (::move &move : moveso) {
                if (move.attacking && move.torow == kingrow && move.tocol == kingcol) {
                    toselectPieces.push_back({static_cast<int>(move.fromrow), static_cast<int>(move.fromcol)});

                    kingIsInCheck = true;
                    // break;
                }
            }
        }

        // for (::move &move : movesm) {
        //     if (move.fromrow == 2 && move.fromcol == 5 && move.torow == 1 && move.tocol == 4) {
        //         printMoveList(moveso);
        //     }
        // }

        auto it = remove_if(moves.begin(), moves.end(), [&](::move &move) {
            if (kingIsInCheck) {
                if (move.piece->piece == 'k') {
                    bool isIllegal = false;
                    for (::move &movea : moveso) {
                        if (movea.attacking && movea.torow == move.torow && movea.tocol == move.tocol) {
                            isIllegal = true;
                            break;
                        }
                    }

                    return isIllegal;
                } else {
                    // make copy of board
                    board boardc = *this;
                    boardc.move(move);

                    deque<::move> possibleMoves = boardc.possibleMoves(toselectPieces, true);

                    vector<tuple<int, int>> fappers;
                    for (::move &movea : moveso) {
                        if (movea.fap[0] == move.fromrow && movea.fap[1] == move.fromcol) {
                            fappers.push_back({static_cast<int>(movea.fromrow), static_cast<int>(movea.fromcol)});
                        }
                    }
                    deque<::move> possibleMoves2 = boardc.possibleMoves(fappers, true);

                    bool isIllegal = false;
                    // cout << possibleMoves.size() << endl;
                    for (::move &move : possibleMoves) {
                        if (move.attacking && move.torow == kingrow && move.tocol == kingcol) {
                            isIllegal = true;
                            break;
                        }
                    };
                    if (!isIllegal) {
                        for (::move &move : possibleMoves2) {
                            if (move.attacking && move.torow == kingrow && move.tocol == kingcol) {
                                isIllegal = true;
                                break;
                            }
                        };
                    }

                    return isIllegal;
                }
            } else {
                if (move.piece->piece == 'k') {
                    bool isIllegal = false;
                    for (::move &movea : moveso) {
                        if (movea.attacking && movea.torow == move.torow && movea.tocol == move.tocol) {
                            isIllegal = true;
                            break;
                        }
                    }

                    return isIllegal;
                } else {
                    // make copy of board
                    board boardc = *this;
                    boardc.move(move);

                    vector<tuple<int, int>> fappers;
                    for (::move &movea : moveso) {
                        if ((movea.fap[0] == move.fromrow && movea.fap[1] == move.fromcol) ||
                            (move.enpassantcol != -2 && move.fromrow == movea.fap[0] && move.tocol == movea.fap[1])) {
                            fappers.push_back({static_cast<int>(movea.fromrow), static_cast<int>(movea.fromcol)});
                        }
                    }
                    // cout << fappers.size() << endl;

                    deque<::move> possibleMoves = boardc.possibleMoves(fappers, true);

                    bool isIllegal = false;
                    // cout << possibleMoves.size() << endl << endl;
                    for (::move &move : possibleMoves) {
                        if (move.attacking && move.torow == kingrow && move.tocol == kingcol) {
                            isIllegal = true;
                            break;
                        }
                    };

                    return isIllegal;
                }
            }
        });
        moves.erase(it, moves.end());

        // castling
        if (whosetomove) {
            // black
            if (castling[2]) {
                // black kingside
                if (pieces[7][5].color[0] == 0b0 && pieces[7][6].color[0] == 0b0) {
                    // castling might be possible

                    // check if moves in between are legal
                    bool isLegal = true;
                    for (::move &move : moveso) {
                        if (move.torow == 7 && move.tocol == 4) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 7 && move.tocol == 5) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 7 && move.tocol == 6) {
                            isLegal = false;
                            break;
                        }
                    }

                    if (isLegal)
                        moves.push_back({&pieces[7][4], static_cast<unsigned int>(7), static_cast<unsigned int>(4),
                                         static_cast<unsigned int>(7), static_cast<unsigned int>(7), -2, -2, ' ',
                                         true});
                }
            }
            if (castling[3]) {
                // black queenside
                if (pieces[7][1].color[0] == 0b0 && pieces[7][2].color[0] == 0b0 && pieces[7][3].color[0] == 0b0) {
                    // castling might be possible

                    // check if moves in between are legal
                    bool isLegal = true;
                    for (::move &move : moveso) {
                        if (move.torow == 7 && move.tocol == 4) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 7 && move.tocol == 3) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 7 && move.tocol == 2) {
                            isLegal = false;
                            break;
                        }
                    }

                    if (isLegal)
                        moves.push_back({&pieces[7][4], static_cast<unsigned int>(7), static_cast<unsigned int>(4),
                                         static_cast<unsigned int>(7), static_cast<unsigned int>(0), -2, -2, ' ',
                                         true});
                }
            }
        } else {
            // white
            if (castling[0]) {
                // white kingside
                if (pieces[0][5].color[0] == 0b0 && pieces[0][6].color[0] == 0b0) {
                    // castling might be possible

                    // check if moves in between are legal
                    bool isLegal = true;
                    for (::move &move : moveso) {
                        if (move.torow == 0 && move.tocol == 4) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 0 && move.tocol == 5) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 0 && move.tocol == 6) {
                            isLegal = false;
                            break;
                        }
                    }

                    if (isLegal)
                        moves.push_back({&pieces[0][4], static_cast<unsigned int>(0), static_cast<unsigned int>(4),
                                         static_cast<unsigned int>(0), static_cast<unsigned int>(7), -2, -2, ' ',
                                         true});
                }
            }
            if (castling[1]) {
                // white queenside
                if (pieces[0][1].color[0] == 0b0 && pieces[0][2].color[0] == 0b0 && pieces[0][3].color[0] == 0b0) {
                    // castling might be possible

                    // check if moves in between are legal
                    bool isLegal = true;
                    for (::move &move : moveso) {
                        if (move.torow == 0 && move.tocol == 4) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 0 && move.tocol == 3) {
                            isLegal = false;
                            break;
                        }
                        if (move.torow == 0 && move.tocol == 2) {
                            isLegal = false;
                            break;
                        }
                    }

                    if (isLegal)
                        moves.push_back({&pieces[0][4], static_cast<unsigned int>(0), static_cast<unsigned int>(4),
                                         static_cast<unsigned int>(0), static_cast<unsigned int>(0), -2, -2, ' ',
                                         true});
                }
            }
        }

        if (moves.size() == 0) {
            // cout << "0 moves!" << endl;
            if (kingIsInCheck) {
            }
            // cerr << "checkmate!" << endl;
            // moves.push_back({&emptypiece, static_cast<unsigned int>(0), static_cast<unsigned int>(0),
            //                  static_cast<unsigned int>(0), static_cast<unsigned int>(0)});
            else
                moves.push_back({&pieces[kingrow][kingcol], static_cast<unsigned int>(kingrow),
                                 static_cast<unsigned int>(kingcol), static_cast<unsigned int>(kingrow),
                                 static_cast<unsigned int>(kingcol)});
        }

        for (::move &move : moves) {
            board boardc = *this;
            boardc.move(move);

            // int eval = 0;

            // for every piece on the board
            for (::piece(&row)[8] : boardc.pieces) {
                for (::piece &piece : row) {
                    if (piece.color[1] == 0b1) {
                        // black
                        if (piece.piece == 'p') {
                            move.eval += -100;
                        }
                        if (piece.piece == 'n') {
                            move.eval += -305;
                        }
                        if (piece.piece == 'b') {
                            move.eval += -333;
                        }
                        if (piece.piece == 'r') {
                            move.eval += -563;
                        }
                        if (piece.piece == 'q') {
                            move.eval += -950;
                        }
                        if (piece.piece == 'k') {
                            move.eval += -100000;
                        }
                    } else {
                        // white
                        if (piece.piece == 'p') {
                            move.eval += 100;
                        }
                        if (piece.piece == 'n') {
                            move.eval += 305;
                        }
                        if (piece.piece == 'b') {
                            move.eval += 333;
                        }
                        if (piece.piece == 'r') {
                            move.eval += 563;
                        }
                        if (piece.piece == 'q') {
                            move.eval += 950;
                        }
                        if (piece.piece == 'k') {
                            move.eval += 100000;
                        }
                    }
                }
            }

            if (whoami)
                move.eval = -1 * move.eval;

            // move.eval = eval;
        }

        // if (size(moves) == 0)
        // throw runtime_error("no moves");

        return moves;
    }

    void loadfenstring_view(string_view fen) {
        string s{fen};
        loadfen(s);
    }
    void loadfen(string fen) {
        // clear board
        for (piece(&row)[8] : pieces) {
            for (piece &piece : row) {
                piece = emptypiece;
            }
        }
        castling = bitset<4>{"0000"};

        int row = 7;
        int col = 0;
        int stage = 0;
        for (char &c : fen) {
            if (stage == 0) {
                if (c == '/') {
                    col = 0;
                    row--;
                    continue;
                }
                if (c > 96 && c < 123) {
                    // lowercase char, so it's black
                    pieces[row][col] = {0b11, (char)tolower(c), 0};
                    // pieces[7 - row][col] = {0b01, (char)tolower(c), 0};
                    col++;
                    continue;
                }
                if (c > 64 && c < 91) {
                    // uppercase char, so it's white
                    pieces[row][col] = {0b01, (char)tolower(c), 0};
                    // pieces[7 - row][col] = {0b11, (char)tolower(c), 0};
                    col++;
                    continue;
                }
                if (c > 47 && c < 58) {
                    // number, so skip that many squares
                    col += c - 48;
                    continue;
                }

                if (c == ' ') {
                    stage++;
                    continue;
                }
            } else if (stage == 1) {
                if (c == 'w') {
                    whosetomove = false;
                    continue;
                } else if (c == 'b') {
                    whosetomove = true;
                    continue;
                } else if (c == ' ') {
                    stage++;
                    continue;
                }
            } else if (stage == 2) {
                if (c == '-') {
                    continue;
                } else if (c == 'K') {
                    castling[0] = 1;
                    continue;
                } else if (c == 'Q') {
                    castling[1] = 1;
                    continue;
                } else if (c == 'k') {
                    castling[2] = 1;
                    continue;
                } else if (c == 'q') {
                    castling[3] = 1;
                    continue;
                } else if (c == ' ') {
                    stage++;
                    continue;
                }
            } else if (stage == 3) {
                if (c == '-') {
                    continue;
                } else if (c > '`' && c < '{') {
                    enpassant[0] = c - 'a';
                } else if (c > '/' && c < ':') {
                    enpassant[1] = c - '1';
                } else if (c == ' ') {
                    stage++;
                    // cout << enpassant[0] << enpassant[1] << endl;
                    continue;
                }
            }
        }
    }

    void printMoveList(deque<::move> &moves) {
        // return;
        for (::move &move : moves) {
            // board.printMove(move);
            cerr
                << piecesmap[move.piece->color[1] ? (char)tolower(move.piece->piece) : (char)toupper(move.piece->piece)]
                << " " << (char)(move.fromcol + 'a') << (char)(move.fromrow + '1') << "→" << (char)(move.tocol + 'a')
                << (char)(move.torow + '1') << "\x1B[0;32m"
                << (string(1, move.promotion) != " "
                        ? string(1, move.promotion)
                        : (move.enpassantrow != -2
                               ? "*"
                               : (move.castling
                                      ? (move.piece->color[1] ? "\x1B[0m" + piecesmap['r'] : "\x1B[0m" + piecesmap['R'])
                                      : "")))
                << (move.fap[0] != -2 ? "\x1B[90m•" : (move.attacking ? "\x1B[37m•" : ""))
                << (move.attacking || move.fap[0] != -2
                        ? piecesmap[move.piece->color[1] ? (char)tolower(pieces[move.torow][move.tocol].piece)
                                                         : (char)toupper(pieces[move.torow][move.tocol].piece)]
                        : "")
                << "\x1B[0m"
                << "\x1B[2m\x1B[90m  \t   " /* << move.fromrow << move.fromcol << " " << move.torow << move.tocol */
                << move.eval << "\x1B[0m\x1B[49m" << endl;
        };
    }

    int eEval() {
        // check for immobility
        auto moves = possibleMoves();
        if (moves.size() == 1 && moves[0].fromcol == moves[0].tocol && moves[0].fromrow == moves[0].torow) {
            // check for mate
            if (moves[0].piece->piece != 'k') {
                return 200000;
            }

            // else stalemate
            return 0;
        }

        return -1;
    };

    ::move findMoveDepth(int depth, atomic<int> &counter, int x, int y) {
        bool iamoriginal = counter == 0;

        // list moves
        deque<::move> moves = possibleMoves();

        // wait for all threads to finish
        if (iamoriginal) {
            vector<tuple<thread *, board *>> threads;
            // for (::move &move : moves) {
            for (int i = 0; i < moves.size(); i++) {
                // if (stillMates(moves[i])) {
                //     moves[i].eval = 696970;
                //     // moves[i].eval = 0;
                //     // continue;
                // }

                // if (!(moves[i].fromcol == 0 && moves[i].fromrow == 5 && moves[i].tocol == 1 && moves[i].torow == 3))
                //     continue;

                // cout << moves[i].fromcol << moves[i].fromrow << moves[i].tocol << moves[i].torow << endl;

                counter++;

                board *boardb = new board(*this);

                thread *t = new thread{([boardb, i, &moves, depth, &counter]() {
                    ::move &move = moves[i];

                    board &boardc = *boardb;
                    boardc.move(move);
                    boardc.toggleWhoseToMove();

                    int e = boardc.eEval();
                    if (e != -1) {
                        moves[i].eval = e;
                        return;
                    }

                    ::move oe = boardc.findMoveDepth(depth - 1, counter, 0, 0);
                    if (oe.eval != 696969)
                        moves[i].eval = oe.eval;
                })};

                threads.push_back({t, boardb});
            }

            // cout << endl << threads.size() << endl;
            for (tuple<thread *, board *> t : threads) {
                if (get<0>(t)->joinable())
                    get<0>(t)->join();
            }

            // desctruct everything properly
            // for every new a delete
        } else {
            // for every move, think of opponent response
            for (::move &move : moves) {
                int xx = x;
                int yy = y;

                if (this->pieces[move.torow][move.tocol].piece == ' ') {
                    xx++;
                    yy++;
                } else
                    xx = 0;

                if (depth > 0 && xx <= 3 && yy <= 4) { // 5 12    3 8
                    board boardc = *this;
                    boardc.move(move);
                    counter++;

                    boardc.toggleWhoseToMove();

                    ::move oe = boardc.findMoveDepth(depth - 1, counter, xx, yy);

                    if (oe.eval != 696969) {
                        move.eval = oe.eval;
                    }
                }
            }
        }

        // sort moves by eval
        sort(
            moves.begin(), moves.end(),
            whoami == whosetomove ? [](const ::move &a, const ::move &b) { return a.eval > b.eval; }
                                  : [](const ::move &a, const ::move &b) { return a.eval < b.eval; });

        if (moves.size() == 0) {
            return {.eval = 696969};
        }

        if (iamoriginal) {
            printMoveList(moves);

            int h = 0;
            for (::move &move : moves) {
                if (move.eval == moves[0].eval)
                    h++;
                else
                    break;
            }

            return moves[std::rand() % h];
        } else {
            return moves[0];
        }
    }
};

namespace sehe_tests {
using namespace std::chrono_literals;
static constexpr auto now = std::chrono::steady_clock::now;
#define TIMED_CHECK(v, c)                                                                                              \
    do {                                                                                                               \
        auto s = now();                                                                                                \
        auto w = (v);                                                                                                  \
        bool ok = (w == c);                                                                                            \
        std::cerr << "Timed:" << std::setw(7) << (now() - s) / 1ms << "ms for " << #v << " == " #c << "\t"             \
                  << (ok ? "OK" : ("FAIL")) << " " << w << std::endl;                                                  \
    } while (0)

static int perft(board &b, unsigned depth) {
    switch (depth--) {
    case 0:
        return 1;
    case 1:
        return b.possibleMoves().size();
    }

    int n = 0;
    for (auto const &m : b.possibleMoves()) {
        auto tmp = b;
        tmp.move(m);
        tmp.toggleWhoseToMove();
        auto l = perft(tmp, depth);
        n += l;
        // if (depth == 1)
        //     cout << (char)(m.fromcol + 'a') << (char)(m.fromrow + '1') << (char)(m.tocol + 'a') << (char)(m.torow +
        //     '1')
        //          << ":" << l << endl;
    }
    return n;
}

static void testPerft() {
    // reference values from https://www.chessprogramming.org/Perft_Results

    // {
    //     board setup_f2f3_e7e5;
    //     setup_f2f3_e7e5.move("f2f3");
    //     setup_f2f3_e7e5.toggleWhoseToMove();
    //     setup_f2f3_e7e5.move("e7e5");
    //     setup_f2f3_e7e5.toggleWhoseToMove();
    //     setup_f2f3_e7e5.move("g2g4");
    //     setup_f2f3_e7e5.toggleWhoseToMove();
    //     setup_f2f3_e7e5.move("d8h4");
    //     setup_f2f3_e7e5.toggleWhoseToMove();
    //     auto pMoves = setup_f2f3_e7e5.possibleMoves();
    //     setup_f2f3_e7e5.printMoveList(pMoves);
    //     TIMED_CHECK(perft(setup_f2f3_e7e5, 2, 2), 11'679);

    //     exit(EXIT_SUCCESS);
    // }

    if (1) {
        board setup;
        // TIMED_CHECK(perft(setup, 0), 1);
        // TIMED_CHECK(perft(setup, 1), 20);
        // TIMED_CHECK(perft(setup, 2), 400);
        // TIMED_CHECK(perft(setup, 3), 8'902);
        // TIMED_CHECK(perft(setup, 4), 197'281);
        TIMED_CHECK(perft(setup, 5), 4'865'609);
        // TIMED_CHECK(perft(setup, 6), 119'060'324);
        // TIMED_CHECK(perft(setup, 7), 3'195'901'860); // unvalidated (michess)
        // TIMED_CHECK(perft(setup, 8), 84'998'978'956); // unvalidated (michess)
        // TIMED_CHECK(perft(setup, 9), 2'439'530'234'167); // unvalidated (michess)
    }
    if (1) {
        board kiwiPete;
        kiwiPete.loadfen({"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"});
        // TIMED_CHECK(perft(kiwiPete, 1), 48);
        // TIMED_CHECK(perft(kiwiPete, 2), 2'039);
        // TIMED_CHECK(perft(kiwiPete, 3), 97'862);
        TIMED_CHECK(perft(kiwiPete, 4), 4'085'603);
        // TIMED_CHECK(perft(kiwiPete, 5), 193'690'690);
        // TIMED_CHECK(perft(kiwiPete, 6), 8'031'647'685);
    }
    if (1) {
        board pos3;
        pos3.loadfen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");
        // TIMED_CHECK(perft(pos3, 1), 14);
        // TIMED_CHECK(perft(pos3, 2), 191);
        // TIMED_CHECK(perft(pos3, 3), 2'812);
        // TIMED_CHECK(perft(pos3, 4), 43'238);
        // TIMED_CHECK(perft(pos3, 5), 674'624);
        TIMED_CHECK(perft(pos3, 6), 11'030'083);
        // TIMED_CHECK(perft(pos3, 7), 178'633'661);
        // TIMED_CHECK(perft(pos3, 8), 3'009'794'393); // unvalidated (michess)
    }
    if (1) {
        {
            board pos4w;
            pos4w.loadfen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
            // TIMED_CHECK(perft(pos4w, 1), 6);
            // TIMED_CHECK(perft(pos4w, 2), 264);
            // TIMED_CHECK(perft(pos4w, 3), 9'467);
            // TIMED_CHECK(perft(pos4w, 4), 422'333);
            TIMED_CHECK(perft(pos4w, 5), 15'833'292);
            // TIMED_CHECK(perft(pos4w, 6), 706'045'033); // sehe unvalidated (michess)
        }
        { // mirrored
            board pos4b;
            pos4b.loadfen("r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1");
            // TIMED_CHECK(perft(pos4b, 1), 6);
            // TIMED_CHECK(perft(pos4b, 2), 264);
            // TIMED_CHECK(perft(pos4b, 3), 9'467);
            // TIMED_CHECK(perft(pos4b, 4), 422'333);
            // TIMED_CHECK(perft(pos4b, 5), 15'833'292);
            // TIMED_CHECK(perft(pos4b, 6), 706'045'033); // unvalidated (michess)
        }
    }
    if (1) {
        board pos5;
        pos5.loadfen("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
        // TIMED_CHECK(perft(pos5, 1), 44);
        // TIMED_CHECK(perft(pos5, 2), 1'486);
        // TIMED_CHECK(perft(pos5, 3), 62'379);
        // TIMED_CHECK(perft(pos5, 4), 2'103'487);
        // TIMED_CHECK(perft(pos5, 5), 89'941'194);
    }
}
} // namespace sehe_tests

namespace micha_tests {
static int perft(board &b, unsigned depth, unsigned depth0) {
    switch (depth--) {
    case 0:
        return 1;
    }

    int n = 0;
    for (auto const &m : b.possibleMoves()) {
        auto tmp = b;
        tmp.move(m);
        tmp.toggleWhoseToMove();
        auto l = perft(tmp, depth, depth0);
        n += l;
        if (depth == depth0 - 1)
            cout << (char)(m.fromcol + 'a') << (char)(m.fromrow + '1') << (char)(m.tocol + 'a') << (char)(m.torow + '1')
                 << ": " << l << endl;
    }

    if (depth == 1) {
        return b.possibleMoves().size();
    }
    return n;
}

static void testPerft(string_view a, unsigned depth) {
    board fromfen;
    fromfen.loadfenstring_view(a);

    perft(fromfen, depth, depth);
}
} // namespace micha_tests

int main(int argc, char *argv[]) {
    std::srand(std::time(nullptr));

#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    board board;

    // board.loadfen("rnb4r/3k1p1p/pp3np1/2pp1p2/8/5PP1/3bq1KP/3N1R2 w - - 2 28");

    // for (int i = 0; i < 1000; i++) {
    //     ::board boardc = board;
    //     boardc.move("d1f2");
    //     boardc.toggleWhoseToMove();

    //     atomic<int> counter = 0;
    //     auto start = chrono::steady_clock::now();

    //     ::move bestMove = boardc.findMoveDepth(depth, counter, 0, 0);
    //     boardc.actuallyMove(bestMove);
    //     cerr << counter / ((chrono::steady_clock::now() - start) / 1ms) / 1000.0 << "M positions per second" << endl
    //          << "(" << counter << " total)" << endl;

    //     // boardc.draw();
    // }

    // return 0;

    // board.loadfen("1r3bk1/5p1p/pp1N2p1/2ppP3/P5PP/1P1P2K1/2P5/4R3 w - - 2 31");
    // 5.80697M positions per second 5.5M
    // (63023041 total)

    // for (std::string mv :
    //      {"c2c3", "g8h6", "d2d4", "b7b6", "c1d2", "h6g4", "d2e3", "g4e3", "b1d2", "e3d1", "d2b3", "d1b2",
    //       "a2a3", "b8a6", "b3d2", "g7g5", "f2f3", "c7c6", "d4d5", "c6d5", "d2b3", "f7f6", "g1h3", "b2c4",
    //       /* "e1c1", "g5g4", "h1g1", "g4h3", "c1c2", "h3g2", "f1g2", "c4e3", "c2b1", "e3g2", "b3d4", "g2f4",
    //       "b1b2", "e8f7", "d1a1", "d7d6", "b2a2", "d8c7", "g1e1", "c7c3", "e1d1", "c8b7", "h2h3", "f4h3",
    //       "d1g1", "h3g1", "a1d1", "c3d4", "d1f1", "d4c4", "a2b1", "c4b3", "b1c1", "g1e2", "c1d2", "e2f4",
    //       "d2e1", "b3e3", "e1d1", "e3a3", "f1h1", "a3c3", "h1h7" */}) {
    //     board.move(mv);
    //     board.toggleWhoseToMove();
    // }

    set<string_view> args{argv + 1, argv + argc};
    if (args.contains("--perft")) {
        sehe_tests::testPerft();
        return 0;
    } else if (args.contains("--perftdiv")) {
        if (next(args.find("--perftdiv")) == args.end() || next(next(args.find("--perftdiv"))) == args.end()) {
            cerr << "--perftdiv DEPTH FEN" << endl;
            return 1;
        }

        string depthstr{*next(args.find("--perftdiv"))};
        int depth = stoi(depthstr);
        string_view fen = *next(next(args.find("--perftdiv")));

        cout << "perftdiv " << fen << " " << depth << endl;
        micha_tests::testPerft(fen, depth);

        return 0;
    }

    int depth = 5;

    if (args.contains("--stupid")) {
        cerr << "started in stupid mode" << endl;
        depth = 3;
    } else if (args.contains("--ultrastupid")) {
        cerr << "started in ultrastupid mode" << endl;
        depth = 2;
    }

    if (args.contains("--black")) {
        // cout << "I'm black" << endl;
        board.whoami = true;

        // board.loadfen("3RK3/8/8/4Q3/8/b1R5/3k4/8 b - - 3 2");
        // // board.loadfen("r3k3/8/8/4q3/8/2r5/3K4/8 b q - 1 1");

        // atomic<int> counter = 0;
        // auto start = chrono::steady_clock::now();

        // // board.move("c7c8Q");
        // // board.toggleWhoseToMove();
        // board.draw();

        // ::move bestMove = board.findMoveDepth(stupid ? 3 : 5, counter, 0, 0);
        // board.actuallyMove(bestMove);
        // cerr << counter / ((chrono::steady_clock::now() - start) / 1ms) / 1000.0 << "M positions per second" << endl
        //      << "(" << counter << " total)" << endl;

        // board.draw();
        // exit(EXIT_SUCCESS);
    } else {
        board.actuallyMove({&board.pieces[1][4], 1, 4, 3, 4, -2, -2, ' ', false, false, {-2, -2}, 0});
        // board.loadfen("8/2p5/3p4/KP5r/5p2/1R2PkP1/8/8 b - - 2 3");
        // board.draw();

        // atomic<int> counter = 0;
        // auto start = chrono::steady_clock::now();

        // // ::move bestMove = board.findMoveDepth(depth, counter, 0, 0);
        // cerr << endl << endl;
        // auto pMoves = board.possibleMoves();
        // board.printMoveList(pMoves);
        // ::move bestMove = pMoves[0];
        // board.actuallyMove(bestMove);

        // cerr << fixed << setprecision(1)
        //      << (counter / 1000.0) / ((chrono::steady_clock::now() - start) / chrono::duration<double>(1.0))
        //      << "k positions per second" << endl
        //      << "(" << counter << " total)" << endl;

        board.draw();

        // exit(EXIT_SUCCESS);
    }

    // let's for now just assume I'm white
    for (string input_move; true;) {
        getline(cin, input_move);
        // input_move = "f6f4";
        if (input_move.length() != 4 && input_move.length() != 5) {
            cerr << "Invalid move: " << input_move << " of length " << input_move.length() << endl;
            // exit(EXIT_FAILURE);
            continue;
        }

        board.move(input_move);
        board.toggleWhoseToMove();

        // example with possibilities for pawns to go sideways, forward, promote (also capturing promotion),
        // castle (not the pawns), en passant, discovery checks

        // to be added: field attacked preventing castling, checkmate, stalemate, 50 move rule, 3 fold repetition
        // field attacked preventing piece to move to it, and being in check, forced to resolve that issue first
        // board.loadfen("1rbk2nr/p2q1Ppp/2p1p3/Pp1p2N1/n1PP4/3B2bR/1P3PPP/RNBQK3 w Q b6 4 13");
        // board.loadfen("1rbk2nr/p2q1Ppp/2p1p3/Pp1p2N1/n1PP4/3B1b1R/1P3PPP/RNBQK3 w Q b6 4 13");
        // board.loadfen("1rbk2nr/p2q1Ppp/2p1p3/Pp1p2Np/n1PP4/3B2bR/1P3P1P/RNBQK2R w QK b6 4 13");

        // board.loadfen("1k6/8/2Q5/pp1B4/7P/6PK/PP6/8 w - - 2 56");

        // both legality check and definition of check check
        // board.loadfen("2R5/8/3k4/R4q2/8/B1K5/8/4R3 b - - 0 1");

        // fap, but not too often
        // board.loadfen("8/8/3k4/5q2/2N2r2/2K5/8/8 b - - 0 1");

        // board.move("e1h1");

        // move move
        // board.loadfen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq - 0 1");
        // board.loadfen("r3k2r/pppppppp/8/P7/8/8/1PPPPPPP/R3K2R b KQkq - 0 1");

        // castling legality check
        // board.loadfen("8/3k4/8/5q2/1B6/7B/8/1R2K2R w K - 0 1");

        // implementing bishops
        // board.loadfen("8/8/2B1B3/8/2B5/8/8/8 w - b6 4 13");

        // implementing knights
        // board.loadfen("8/8/1N6/N2N4/2n5/8/8/8 w - - 4 13");

        // implementing kings
        // board.loadfen("8/8/4k3/2qP4/1RK5/8/8/8 w - - 4 13");

        // relatively complicated legality check
        // board.loadfen("k7/8/8/2rr1n2/4b3/4p3/3K4/2r5 w - - 2 2");
        // board.loadfen("8/4k3/8/2q5/1B6/4B3/5K2/1R6 w - - 0 1");
        // board.loadfen("1k6/8/8/3r1b2/8/3K1Q2/8/8 w - - 1 1");
        // board.move(input_move);
        // board.toggleWhoseToMove();

        // board.move("a5b6");

        // deque<::move> possibleMoves = board.possibleMoves();
        // for (::move &move : possibleMoves) {
        //     // board.printMove(move);
        //     cerr
        //         << piecesmap[move.piece->color[1] ? (char)tolower(move.piece->piece) :
        //         (char)toupper(move.piece->piece)]
        //         << " " << (char)(move.fromcol + 'a') << (char)(move.fromrow + '1') << "→" << (char)(move.tocol +
        //         'a')
        //         << (char)(move.torow + '1') << "\x1B[0;32m"
        //         << (string(1, move.promotion) != " "
        //                 ? string(1, move.promotion)
        //                 : (move.enpassantrow != -2
        //                        ? "*"
        //                        : (move.castling
        //                               ? (move.piece->color[1] ? "\x1B[0m" + piecesmap['r'] : "\x1B[0m" +
        //                               piecesmap['R']) : "")))
        //         << (move.fap[0] != -2 ? "\x1B[90m•" : (move.attacking ? "\x1B[37m•" : ""))
        //         << (move.attacking || move.fap[0] != -2
        //                 ? piecesmap[move.piece->color[1] ?
        //                 (char)tolower(board.pieces[move.torow][move.tocol].piece)
        //                                                  :
        //                                                  (char)toupper(board.pieces[move.torow][move.tocol].piece)]
        //                 : "")
        //         << "\x1B[0m"
        //         << "\x1B[2m\x1B[90m  \t   " /* << move.fromrow << move.fromcol << " " << move.torow << move.tocol
        //         */
        //         << move.eval << "\x1B[0m\x1B[49m" << endl;
        // };

        // amout of possible moves
        // cerr << possibleMoves.size() << endl << endl;

        atomic<int> counter = 0;
        auto start = chrono::steady_clock::now();

        ::move bestMove = board.findMoveDepth(depth, counter, 0, 0);
        board.actuallyMove(bestMove);

        cerr << fixed << setprecision(1)
             << (counter / 1000.0) / ((chrono::steady_clock::now() - start) / chrono::duration<double>(1.0))
             << "k positions per second" << endl
             << "(" << counter << " total)" << endl;

        board.draw();

        // cout << board.eval() << endl;

        // exit(EXIT_SUCCESS);
    }
}