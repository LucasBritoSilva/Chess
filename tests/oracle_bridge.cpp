// Test-only executable: reads FEN lines, writes legal moves and their outcomes.
#include "Chess.h"
#include <iostream>

int main() {
    std::string fen;
    while (std::getline(std::cin, fen)) {
        try {
            auto board = chess::Board::fromFen(fen);
            std::cout << "{\"check\":" << (board.inCheck(board.turn) ? "true" : "false")
                      << ",\"insufficient\":" << (board.insufficientMaterial() ? "true" : "false") << ",\"moves\":[";
            bool first = true;
            for (const auto& move : board.legalMoves()) {
                if (!first) std::cout << ',';
                first = false;
                chess::Game game(board);
                const bool played = game.play(move);
                std::cout << "{\"uci\":\"" << move.uci() << "\",\"fen\":\"" << board.after(move).fen()
                          << "\",\"san\":\"" << (played ? game.history().back().san : "") << "\"}";
            }
            std::cout << "]}\n";
        } catch (const std::exception& error) {
            std::cerr << "Invalid input: " << error.what() << '\n';
            return 1;
        }
    }
}
