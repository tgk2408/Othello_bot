/*
* @file botTemplate.cpp
* @author Arun Tejasvi Chaganty <arunchaganty@gmail.com>
* @date 2010-02-04
* Template for users to create their own bots
*/

#include "Othello.h"
#include "OthelloBoard.h"
#include "OthelloPlayer.h"
#include <cstdlib>
#include <ctime>
using namespace std;
using namespace Desdemona;

class MyBot: public OthelloPlayer
{
    public:
        /**
         * Initialisation routines here
         * This could do anything from open up a cache of "best moves" to
         * spawning a background processing thread. 
         */
        MyBot( Turn turn );

        /**
         * Play something 
         */
        Turn our_colour;
        int Heuristic_mat[8][8] = {{20, -4, 11, 8, 8, 11, -4, 20},
                                   {-4, -7, -4, 1, 1, -4, -7, -4},
                                   {11, -4, 2, 2, 2, 2, -4, 11},
                                   {8, 1, 2, -3, -3, 2, 1, 8},
                                   {8, 1, 2, -3, -3, 2, 1, 8},
                                   {11, -4, 2, 2, 2, 2, -4, 11},
                                   {-4, -7, -4, 1, 1, -4, -7, -4},
                                   {20, -4, 11, 8, 8, 11, -4, 20}};
        int horizontal_bias[8] = {-1, -1, 0, 1, 1, 1, 0, -1};
        int vertical_bias[8] = {0, 1, 1, 1, 0, -1, -1, -1};
        clock_t start_time;
        clock_t end_time;
        virtual int GetCoinCount(const OthelloBoard &board, Turn);
        virtual Move play(const OthelloBoard &board);
        virtual double TestMove(const OthelloBoard &board, Turn, int, double, double);
        virtual double Heuristic(const OthelloBoard &board, Turn);

    private:
};

MyBot::MyBot( Turn turn )
    : OthelloPlayer( turn )
{
    our_colour = turn;
}

int MyBot::GetCoinCount(const OthelloBoard &board, Turn colour) {
    if (colour == RED) {
        return board.getRedCount();
    } else if (colour == BLACK) {
        return board.getBlackCount();
    } else {
        return 0;
    }
}

Move MyBot::play( const OthelloBoard& board )
{
    start_time = clock();
    list<Move> moves = board.getValidMoves(turn);
    list<Move>::iterator it = moves.begin();

    Move best_move = *it;
    double alpha = -1e18;
    double beta = 1e18;
    for (; it != moves.end(); it++) {
        Move current_move = *it;
        OthelloBoard current_board = board;
        current_board.makeMove(our_colour, current_move);
        double parameter = TestMove(current_board, our_colour, 5, alpha, beta);
        if (parameter > alpha) {
            alpha = parameter;
            best_move = current_move;
        }
    }
    return best_move;
}

double MyBot::TestMove(const OthelloBoard &board, Turn colour, int depth, double alpha, double beta) {

    //do not take more than 2 sec
    end_time = clock();
    if (double(end_time - start_time) / CLOCKS_PER_SEC >= 1.98) {
        if (colour == our_colour) {
            return alpha;
        } else {
            return beta;
        }
    }

    list<Move> moves = board.getValidMoves(other(colour));
    list<Move>::iterator it = moves.begin();

    //termination
    if (depth == 0 || moves.size() == 0) {
        return Heuristic(board, our_colour);
    }

    if (colour == our_colour) { //check beta
        for (; it != moves.end(); it++) {
            Move current_move = *it;
            OthelloBoard current_board = board;
            current_board.makeMove(other(colour), current_move);
            beta = min(beta, TestMove(current_board, other(colour), depth - 1, alpha, beta));
            if (alpha >= beta) {
                return alpha;
            }
        }
        return beta;
    } else { //check alpha
        for (; it != moves.end(); it++) {
            Move current_move = *it;
            OthelloBoard current_board = board;
            current_board.makeMove(other(colour), current_move);
            alpha = max(alpha, TestMove(current_board, other(colour), depth - 1, alpha, beta));
            if (alpha >= beta) {
                return beta;
            }
        }
        return alpha;
    }
}

double MyBot::Heuristic(const OthelloBoard &board, Turn colour) {

    double piece_difference_heuristic = 0;
    double corner_heuristic = 0;
    double frontier_heuristic = 0;
    double disk_heuristic = 0;
    double corner_adjacent_heuristic = 0;
    double mobility_heuristic = 0;

    int our_frontier_tiles = 0, opponent_frontier_tiles = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board.get(i, j) == colour) {
                disk_heuristic += Heuristic_mat[i][j];
            } else if (board.get(i, j) == other(colour)) {
                disk_heuristic -= Heuristic_mat[i][j];
            } else {
                ;
            }
            if (board.get(i, j) != EMPTY) {
                for (int k = 0; k < 8; k++) {
                    int x = i + horizontal_bias[k];
                    int y = j + vertical_bias[k];
                    if (x >= 0 && x < 8 && y >= 0 && y < 8 && board.get(x, y) == EMPTY) {
                        if (board.get(i, j) == colour) {
                            our_frontier_tiles++;
                        } else if (board.get(i, j) == other(colour)) {
                            opponent_frontier_tiles++;
                        }
                        break;
                    }
                }
            }
        }
    }

    piece_difference_heuristic = (100 * GetCoinCount(board, colour)) / (GetCoinCount(board, colour) + GetCoinCount(board, other(colour)));
    if (piece_difference_heuristic <= 50) {
        piece_difference_heuristic -= 100;
    }

    if (our_frontier_tiles + opponent_frontier_tiles > 0) {
        frontier_heuristic = -(100.0 * our_frontier_tiles) / (our_frontier_tiles + opponent_frontier_tiles);
        if (frontier_heuristic > -50) {
            frontier_heuristic = 100 + frontier_heuristic;
        }
    }

    if (board.get(0, 0) == colour) {
        corner_heuristic++;
    } else if (board.get(0, 0) == other(colour)) {
        corner_heuristic--;
    } else {
        if (board.get(0, 1) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(0, 1) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(1, 1) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(1, 1) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(1, 0) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(1, 0) == other(colour)) {
            corner_adjacent_heuristic--;
        }
    }
    if (board.get(0, 7) == colour) {
        corner_heuristic++;
    } else if (board.get(0, 7) == other(colour)) {
        corner_heuristic--;
    } else {
        if (board.get(0, 6) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(0, 6) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(1, 6) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(1, 6) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(1, 7) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(1, 7) == other(colour)) {
            corner_adjacent_heuristic--;
        }
    }
    if (board.get(7, 7) == colour) {
        corner_heuristic++;
    } else if (board.get(7, 7) == other(colour)) {
        corner_heuristic--;
    } else {
        if (board.get(6, 7) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(6, 7) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(6, 6) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(6, 6) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(7, 6) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(7, 6) == other(colour)) {
            corner_adjacent_heuristic--;
        }
    }
    if (board.get(7, 0) == colour) {
        corner_heuristic++;
    } else if (board.get(7, 0) == other(colour)) {
        corner_heuristic--;
    } else {
        if (board.get(7, 1) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(7, 1) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(6, 1) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(6, 1) == other(colour)) {
            corner_adjacent_heuristic--;
        }
        if (board.get(6, 0) == colour) {
            corner_adjacent_heuristic++;
        } else if (board.get(6, 0) == other(colour)) {
            corner_adjacent_heuristic--;
        }
    }
    corner_heuristic *= 25;
    corner_adjacent_heuristic *= -12.5;

    if (board.getValidMoves(colour).size() + board.getValidMoves(other(colour)).size() != 0) {
        mobility_heuristic = (100 * board.getValidMoves(colour).size()) / (board.getValidMoves(colour).size() + board.getValidMoves(other(colour)).size());
        if (mobility_heuristic <= 50) {
            mobility_heuristic -= 100;
        }
    }

    double score = 10 * piece_difference_heuristic + 801.724 * corner_heuristic + 382.026 * corner_adjacent_heuristic + 78.922 * mobility_heuristic + 74.396 * frontier_heuristic + 10 * disk_heuristic;
    return score;
}

// The following lines are _very_ important to create a bot module for Desdemona

extern "C" {
    OthelloPlayer* createBot( Turn turn )
    {
        return new MyBot( turn );
    }

    void destroyBot( OthelloPlayer* bot )
    {
        delete bot;
    }
}


