#include "Chess.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace chess {
namespace {
char type(char p) { return static_cast<char>(std::tolower(static_cast<unsigned char>(p))); }
bool inside(int f, int r) { return f >= 0 && f < 8 && r >= 0 && r < 8; }
constexpr int knightSteps[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
constexpr int directions[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
}

Color opposite(Color c) { return c == Color::White ? Color::Black : Color::White; }
bool isWhite(char p) { return p >= 'A' && p <= 'Z'; }
bool belongsTo(char p, Color c) { return p != '.' && isWhite(p) == (c == Color::White); }
std::string squareName(int s) {
    if (s < 0 || s >= 64) return "-";
    return std::string{static_cast<char>('a' + s % 8), static_cast<char>('1' + s / 8)};
}
int squareIndex(const std::string& name) {
    if (name.size() != 2 || name[0] < 'a' || name[0] > 'h' || name[1] < '1' || name[1] > '8') return -1;
    return (name[1] - '1') * 8 + name[0] - 'a';
}
bool Move::operator==(const Move& m) const { return from == m.from && to == m.to && promotion == m.promotion; }
std::string Move::uci() const { return squareName(from) + squareName(to) + (promotion ? std::string(1, promotion) : ""); }

Board::Board() {
    const std::string initial = "RNBQKBNRPPPPPPPP................................pppppppprnbqkbnr";
    std::copy(initial.begin(), initial.end(), squares.begin());
}

Board Board::fromFen(const std::string& fen) {
    Board b;
    b.squares.fill('.');
    std::istringstream input(fen);
    std::string placement, side, rights, ep, extra;
    if (!(input >> placement >> side >> rights >> ep >> b.halfmove >> b.fullmove) || (input >> extra))
        throw std::invalid_argument("FEN: expected six fields");
    int rank = 7, file = 0;
    for (char p : placement) {
        if (p == '/') {
            if (file != 8 || rank == 0) throw std::invalid_argument("FEN: invalid rank");
            --rank; file = 0;
        } else if (p >= '1' && p <= '8') {
            file += p - '0';
        } else {
            if (std::string("PNBRQKpnbrqk").find(p) == std::string::npos || file >= 8)
                throw std::invalid_argument("FEN: invalid piece");
            if (type(p) == 'p' && (rank == 0 || rank == 7)) throw std::invalid_argument("FEN: pawn on last rank");
            b.squares[rank * 8 + file++] = p;
        }
        if (file > 8) throw std::invalid_argument("FEN: rank overflow");
    }
    if (rank != 0 || file != 8 || (side != "w" && side != "b") || b.halfmove < 0 || b.fullmove < 1)
        throw std::invalid_argument("FEN: invalid position");
    b.turn = side == "w" ? Color::White : Color::Black;
    b.castling = 0;
    if (rights != "-") for (char c : rights) {
        auto bit = std::string("KQkq").find(c);
        if (bit == std::string::npos || (b.castling & (1 << bit))) throw std::invalid_argument("FEN: castling rights");
        b.castling |= 1 << bit;
    }
    if (std::count(b.squares.begin(), b.squares.end(), 'K') != 1 ||
        std::count(b.squares.begin(), b.squares.end(), 'k') != 1)
        throw std::invalid_argument("FEN: expected two kings");
    for (int i = 0; i < 4; ++i) {
        const int rookSquares[] = {7,0,63,56};
        if ((b.castling & (1 << i)) &&
            (b.squares[i < 2 ? 4 : 60] != (i < 2 ? 'K' : 'k') || b.squares[rookSquares[i]] != (i < 2 ? 'R' : 'r')))
            throw std::invalid_argument("FEN: missing castling piece");
    }
    b.enPassant = ep == "-" ? -1 : squareIndex(ep);
    if (ep != "-") {
        const int step = b.turn == Color::White ? 8 : -8;
        if (b.enPassant < 0 || b.enPassant / 8 != (b.turn == Color::White ? 5 : 2) ||
            b.squares[b.enPassant] != '.' || b.squares[b.enPassant - step] != (b.turn == Color::White ? 'p' : 'P') ||
            b.squares[b.enPassant + step] != '.' || b.halfmove != 0)
            throw std::invalid_argument("FEN: invalid en passant");
    }
    if (b.inCheck(opposite(b.turn))) throw std::invalid_argument("FEN: previous mover is in check");
    return b;
}

std::string Board::fen() const {
    std::ostringstream out;
    for (int r = 7; r >= 0; --r) {
        int empty = 0;
        for (int f = 0; f < 8; ++f) {
            char p = squares[r * 8 + f];
            if (p == '.') ++empty;
            else { if (empty) out << empty; empty = 0; out << p; }
        }
        if (empty) out << empty;
        if (r) out << '/';
    }
    out << (turn == Color::White ? " w " : " b ");
    if (!castling) out << '-';
    for (int i = 0; i < 4; ++i) if (castling & (1 << i)) out << "KQkq"[i];
    out << ' ' << squareName(enPassant) << ' ' << halfmove << ' ' << fullmove;
    return out.str();
}

int Board::king(Color c) const {
    const auto it = std::find(squares.begin(), squares.end(), c == Color::White ? 'K' : 'k');
    return it == squares.end() ? -1 : static_cast<int>(it - squares.begin());
}
bool Board::attacked(int square, Color by) const {
    if (square < 0 || square >= 64) return false;
    const int f = square % 8, r = square / 8;
    const int pawnRank = r + (by == Color::White ? -1 : 1);
    for (int df : {-1,1}) if (inside(f + df, pawnRank) && squares[pawnRank * 8 + f + df] == (by == Color::White ? 'P' : 'p')) return true;
    for (auto& d : knightSteps) if (inside(f+d[0],r+d[1]) && squares[(r+d[1])*8+f+d[0]] == (by == Color::White ? 'N' : 'n')) return true;
    for (int i = 0; i < 8; ++i) {
        const auto& d = directions[i];
        for (int distance = 1; ; ++distance) {
            int nf = f + d[0]*distance, nr = r + d[1]*distance;
            if (!inside(nf,nr)) break;
            const char p = squares[nr*8+nf];
            if (p == '.') continue;
            if (belongsTo(p, by) && (type(p) == 'q' || (i < 4 ? type(p) == 'r' : type(p) == 'b') || (distance == 1 && type(p) == 'k'))) return true;
            break;
        }
    }
    return false;
}
bool Board::inCheck(Color c) const { return attacked(king(c), opposite(c)); }

std::vector<Move> Board::pseudoMoves() const {
    std::vector<Move> moves;
    auto add = [&](int from, int to) {
        if (type(squares[from]) == 'p' && (to/8 == 0 || to/8 == 7)) {
            for (char p : std::string("qrbn")) moves.push_back({from,to,p});
        } else moves.push_back({from,to,0});
    };
    for (int from = 0; from < 64; ++from) {
        char p = squares[from];
        if (!belongsTo(p,turn)) continue;
        const int f = from % 8, r = from / 8;
        auto target = [&](int nf, int nr) {
            if (!inside(nf,nr)) return false;
            char dest = squares[nr*8+nf];
            if (belongsTo(dest,turn) || type(dest) == 'k') return false;
            add(from,nr*8+nf);
            return dest == '.';
        };
        if (type(p) == 'p') {
            const int step = turn == Color::White ? 1 : -1;
            if (inside(f,r+step) && squares[(r+step)*8+f] == '.') {
                add(from,(r+step)*8+f);
                if (r == (turn == Color::White ? 1 : 6) && squares[(r+2*step)*8+f] == '.') add(from,(r+2*step)*8+f);
            }
            for (int df : {-1,1}) if (inside(f+df,r+step)) {
                int to = (r+step)*8+f+df;
                if ((belongsTo(squares[to],opposite(turn)) && type(squares[to]) != 'k') ||
                    (to == enPassant && squares[to] == '.' && squares[to-step*8] == (turn == Color::White ? 'p' : 'P'))) add(from,to);
            }
        } else if (type(p) == 'n') {
            for (auto& d : knightSteps) target(f+d[0],r+d[1]);
        } else {
            for (int i = 0; i < 8; ++i) {
                if ((type(p) == 'b' && i < 4) || (type(p) == 'r' && i >= 4)) continue;
                const auto& d = directions[i];
                for (int n = 1; n <= (type(p) == 'k' ? 1 : 7); ++n) if (!target(f+d[0]*n,r+d[1]*n)) break;
            }
            if (type(p) == 'k') {
                const int base = turn == Color::White ? 0 : 56;
                const int rights = turn == Color::White ? 1 : 4;
                const char rook = turn == Color::White ? 'R' : 'r';
                if (from != base+4 || inCheck(turn)) continue;
                for (bool kingside : {true,false}) {
                    const int rookSquare = base + (kingside ? 7 : 0);
                    const int through = base + (kingside ? 5 : 3), to = base + (kingside ? 6 : 2);
                    if (!(castling & (kingside ? rights : rights*2)) || squares[rookSquare] != rook ||
                        squares[through] != '.' || squares[to] != '.' || (!kingside && squares[base+1] != '.')) continue;
                    // Evaluate transit with the king removed from its original square.
                    if (!after({from,through,0}).inCheck(turn) && !after({from,to,0}).inCheck(turn)) add(from,to);
                }
            }
        }
    }
    return moves;
}

Board Board::after(const Move& m) const {
    Board next = *this;
    const char p = squares[m.from];
    bool capture = squares[m.to] != '.';
    if (type(p) == 'p' && m.to == enPassant && squares[m.to] == '.' && m.from%8 != m.to%8) {
        next.squares[m.to + (turn == Color::White ? -8 : 8)] = '.';
        capture = true;
    }
    next.squares[m.to] = m.promotion ? (turn == Color::White ? static_cast<char>(std::toupper(m.promotion)) : m.promotion) : p;
    next.squares[m.from] = '.';
    if (type(p) == 'k') {
        next.castling &= turn == Color::White ? ~3 : ~12;
        if (std::abs(m.to-m.from) == 2) {
            const int rookFrom = m.to > m.from ? m.from+3 : m.from-4;
            const int rookTo = m.to > m.from ? m.from+1 : m.from-1;
            next.squares[rookTo] = next.squares[rookFrom]; next.squares[rookFrom] = '.';
        }
    }
    const int corners[] = {7,0,63,56};
    for (int i = 0; i < 4; ++i) if (m.from == corners[i] || m.to == corners[i]) next.castling &= ~(1 << i);
    next.enPassant = type(p) == 'p' && std::abs(m.to-m.from) == 16 ? (m.from+m.to)/2 : -1;
    next.halfmove = type(p) == 'p' || capture ? 0 : halfmove+1;
    next.fullmove += turn == Color::Black ? 1 : 0;
    next.turn = opposite(turn);
    return next;
}
std::vector<Move> Board::legalMoves() const {
    auto moves = pseudoMoves();
    moves.erase(std::remove_if(moves.begin(),moves.end(),[&](const Move& m) { return after(m).inCheck(turn); }),moves.end());
    return moves;
}
bool Board::insufficientMaterial() const {
    int knights = 0, bishops = 0, bishopColor = -1;
    bool sameColor = true;
    for (int s = 0; s < 64; ++s) {
        char p = type(squares[s]);
        if (p == '.' || p == 'k') continue;
        if (p == 'n') ++knights;
        else if (p == 'b') {
            ++bishops;
            const int color = (s/8+s%8)%2;
            if (bishopColor != -1 && bishopColor != color) sameColor = false;
            bishopColor = color;
        } else return false;
    }
    return (bishops == 0 && knights <= 1) || (knights == 0 && sameColor);
}
std::uint64_t perft(const Board& b, int depth) {
    if (depth <= 0) return 1;
    const auto moves = b.legalMoves();
    if (depth == 1) return moves.size();
    std::uint64_t nodes = 0;
    for (const auto& move : moves) nodes += perft(b.after(move),depth-1);
    return nodes;
}
} // namespace chess
