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
}
