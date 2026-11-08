#include "Chess.h"

namespace {
chess::Game game;
std::string response;
}

// A returned string is valid until the next call to chess_state().
extern "C" {
const char* chess_state() { response = game.json(); return response.c_str(); }
void chess_reset() { game.reset(); }
int chess_move(int from, int to, int promotion) {
    if (promotion != 0 && promotion != 'q' && promotion != 'r' && promotion != 'b' && promotion != 'n') return 0;
    return game.play({from,to,static_cast<char>(promotion)}) ? 1 : 0;
}
int chess_claim(int from, int to, int promotion) {
    if (from == -1 && to == -1 && promotion == 0) return game.claimDraw() ? 1 : 0;
    if (promotion != 0 && promotion != 'q' && promotion != 'r' && promotion != 'b' && promotion != 'n') return 0;
    chess::Move announced{from,to,static_cast<char>(promotion)};
    return game.claimDraw(&announced) ? 1 : 0;
}
int chess_agree() { return game.agreeDraw() ? 1 : 0; }
}
