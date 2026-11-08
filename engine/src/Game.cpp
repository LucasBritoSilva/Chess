#include "Chess.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>

namespace chess {
namespace {
char type(char p) { return static_cast<char>(std::tolower(static_cast<unsigned char>(p))); }
void writeMove(std::ostream& out, const Move& m) {
    out << "{\"from\":" << m.from << ",\"to\":" << m.to << ",\"promotion\":\"";
    if (m.promotion) out << m.promotion;
    out << "\"}";
}
}

Game::Game() : Game(Board{}) {}
Game::Game(const Board& b) : board_(b) { evaluate(); }
void Game::reset() { *this = Game{}; }
void Game::evaluate() {
    const auto legal = board_.legalMoves();
    if (legal.empty()) status_ = board_.inCheck(board_.turn) ? "checkmate" : "stalemate";
    else if (board_.insufficientMaterial()) status_ = "insufficient";
    else status_ = "playing";
}
std::string Game::notation(const Move& m, const std::vector<Move>& legal) const {
    char p = board_.squares[m.from];
    std::string san;
    if (type(p) == 'k' && std::abs(m.to-m.from) == 2) san = m.to > m.from ? "O-O" : "O-O-O";
    else {
        const bool capture = board_.squares[m.to] != '.' || (type(p) == 'p' && m.from%8 != m.to%8);
        if (type(p) != 'p') {
            san += static_cast<char>(std::toupper(p));
            bool ambiguous = false, sameFile = false, sameRank = false;
            for (const auto& other : legal) if (other.to == m.to && other.from != m.from && board_.squares[other.from] == p) {
                ambiguous = true;
                sameFile |= other.from%8 == m.from%8;
                sameRank |= other.from/8 == m.from/8;
            }
            if (ambiguous) {
                if (!sameFile) san += squareName(m.from)[0];
                else if (!sameRank) san += squareName(m.from)[1];
                else san += squareName(m.from);
            }
        } else if (capture) san += squareName(m.from)[0];
        if (capture) san += 'x';
        san += squareName(m.to);
        if (m.promotion) { san += '='; san += static_cast<char>(std::toupper(m.promotion)); }
    }
    const Board next = board_.after(m);
    if (next.inCheck(next.turn)) san += next.legalMoves().empty() ? '#' : '+';
    return san;
}
bool Game::play(const Move& m) {
    if (finished()) return false;
    const auto legal = board_.legalMoves();
    if (std::find(legal.begin(),legal.end(),m) == legal.end()) return false;
    history_.push_back({m,notation(m,legal),board_.turn,board_.fullmove});
    board_ = board_.after(m);
    evaluate();
    return true;
}
std::string Game::json() const {
    std::ostringstream out;
    const bool check = board_.inCheck(board_.turn);
    out << "{\"board\":\"" << std::string(board_.squares.begin(),board_.squares.end())
        << "\",\"turn\":\"" << (board_.turn == Color::White ? "white" : "black")
        << "\",\"status\":\"" << status_ << "\",\"winner\":\"";
    if (status_ == "checkmate") out << (board_.turn == Color::White ? "black" : "white");
    out << "\",\"check\":" << (check ? "true" : "false") << ",\"checkSquare\":" << (check ? board_.king(board_.turn) : -1)
        << ",\"fullmove\":" << board_.fullmove << ",\"legalMoves\":[";
    bool first = true;
    if (!finished()) for (const auto& m : board_.legalMoves()) { if (!first) out << ','; first = false; writeMove(out,m); }
    out << "],\"history\":[";
    first = true;
    for (const auto& h : history_) {
        if (!first) out << ',';
        first = false;
        out << "{\"san\":\"" << h.san << "\",\"player\":\""
            << (h.player == Color::White ? "white" : "black") << "\",\"number\":" << h.number << '}';
    }
    out << "],\"lastMove\":";
    if (history_.empty()) out << "null"; else writeMove(out,history_.back().move);
    out << '}';
    return out.str();
}
} // namespace chess
