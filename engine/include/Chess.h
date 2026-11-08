#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace chess {

enum class Color { White, Black };
Color opposite(Color color);
bool isWhite(char piece);
bool belongsTo(char piece, Color color);
std::string squareName(int square);
int squareIndex(const std::string& name);

struct Move {
    int from = -1;
    int to = -1;
    char promotion = 0; // q, r, b, n; zero for an ordinary move.
    bool operator==(const Move& other) const;
    std::string uci() const;
};

// a1 = 0, h8 = 63. Uppercase pieces are white; '.' is empty.
class Board {
public:
    std::array<char, 64> squares{};
    Color turn = Color::White;
    int castling = 15; // white king/queen, black king/queen: 1/2/4/8
    int enPassant = -1;
    int halfmove = 0;
    int fullmove = 1;

    Board();
    static Board fromFen(const std::string& fen);
    std::string fen() const;
    bool attacked(int square, Color by) const;
    int king(Color color) const;
    bool inCheck(Color color) const;
    std::vector<Move> legalMoves() const;
    // Internal transition: caller must supply a move from legalMoves().
    Board after(const Move& move) const;
    bool insufficientMaterial() const;
private:
    std::vector<Move> pseudoMoves() const;
};

struct PlayedMove {
    Move move;
    std::string san;
    Color player = Color::White;
    int number = 1;
};

class Game {
public:
    Game();
    explicit Game(const Board& position);
    void reset();
    const Board& board() const { return board_; }
    const std::vector<PlayedMove>& history() const { return history_; }
    const std::string& status() const { return status_; }
    bool finished() const { return status_ != "playing"; }
    bool play(const Move& move);
    std::string json() const;
private:
    Board board_;
    std::vector<PlayedMove> history_;
    std::string status_ = "playing";
    void evaluate();
    std::string notation(const Move& move, const std::vector<Move>& legal) const;
};

std::uint64_t perft(const Board& board, int depth);

} // namespace chess
