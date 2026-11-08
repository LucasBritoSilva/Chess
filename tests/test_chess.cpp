#include "Chess.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <stdexcept>

using namespace chess;
namespace {
int checks = 0;
void check(bool value, const std::string& message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
Move move(const std::string& uci) {
    return {squareIndex(uci.substr(0,2)),squareIndex(uci.substr(2,2)),uci.size() == 5 ? uci[4] : char(0)};
}
bool has(const Board& b, const std::string& uci) {
    auto legal = b.legalMoves();
    return std::find(legal.begin(),legal.end(),move(uci)) != legal.end();
}
void play(Game& g, const std::string& uci) { check(g.play(move(uci)), "Expected legal move: " + uci); }
void cycle(Game& g) { for (const auto& m : {"g1f3","g8f6","f3g1","f6g8"}) play(g,m); }

void initialAndPerft() {
    Board b;
    check(b.fen() == "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Initial FEN");
    check(Board::fromFen(b.fen()).fen() == b.fen(), "FEN roundtrip");
    for (auto [depth,nodes] : std::vector<std::pair<int,std::uint64_t>>{{1,20},{2,400},{3,8902},{4,197281}})
        check(perft(b,depth) == nodes, "Initial perft " + std::to_string(depth));
    const auto kiwipete = Board::fromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    check(perft(kiwipete,1) == 48, "Kiwipete depth 1");
    check(perft(kiwipete,2) == 2039, "Kiwipete depth 2");
    check(perft(kiwipete,3) == 97862, "Kiwipete depth 3");
    const auto ending = Board::fromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    check(perft(ending,4) == 43238, "Endgame perft depth 4");
    for (const auto& fen : {"bad", "8/8/8/8/8/8/8/8 w - - 0 1", "7k/8/8/8/8/8/8/K7 w K - 0 1", "7k/8/8/8/8/8/8/K7 w - e6 0 1"}) {
        bool rejected = false;
        try { Board::fromFen(fen); } catch (const std::invalid_argument&) { rejected = true; }
        check(rejected,"Reject malformed FEN");
    }
}
void movement() {
    Game g;
    check(!g.play({-1,100,0}), "Reject out of bounds");
    check(!g.play(move("e7e5")), "White starts");
    check(!g.play(move("a1a3")), "Rook blocked");
    check(!g.play(move("c1h6")), "Bishop blocked");
    check(!g.play(move("d1d3")), "Queen blocked");
    check(!g.play(move("e1e2")), "Own piece blocks king");
    check(!g.play(move("e2f3")), "Pawn cannot capture empty square");
    check(!g.play(move("b1b3")), "Knight invalid move");
    check(g.history().empty(), "Illegal moves leave history unchanged");
    play(g,"e2e4"); check(g.board().turn == Color::Black, "Alternate turn");
    play(g,"d7d5"); play(g,"e4d5");
    check(g.board().squares[squareIndex("d5")] == 'P', "Capture replaces opponent");
    check(g.history().back().captured == 'p' && g.history().back().san == "exd5", "Pawn capture history");
    play(g,"g8f6"); play(g,"f1b5");
    check(g.board().inCheck(Color::Black), "Bishop check");
    check(!g.play(move("a7a6")), "Cannot ignore check");
    play(g,"c7c6");
    const Board open = Board::fromFen("8/7k/8/8/3Q4/8/8/K7 w - - 0 1");
    for (const auto& m : {"d4d7","d4a4","d4g7","d4a1"}) check(has(open,m) == (std::string(m) != "d4a1"), "Queen rays/own king");
    const Board pin = Board::fromFen("4r2k/8/8/8/8/8/4R3/4K3 w - - 0 1");
    check(!has(pin,"e2f2") && has(pin,"e2e8"), "Pinned rook");
    const Board kings = Board::fromFen("8/8/8/8/8/4k3/8/4K3 w - - 0 1");
    check(!has(kings,"e1e2") && has(kings,"e1d1"), "Kings cannot touch");
    g.reset();
    check(g.board().fen() == Board{}.fen() && g.history().empty() && !g.finished(), "Reset all state");
}
void specialMoves() {
    Game castle(Board::fromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"));
    check(has(castle.board(),"e1g1") && has(castle.board(),"e1c1"), "Both white castlings");
    play(castle,"e1g1");
    check(castle.board().squares[5] == 'R' && castle.board().squares[6] == 'K' && castle.board().squares[7] == '.', "Castling moves rook");
    check(castle.history().back().san == "O-O", "Castling notation");
    play(castle,"e8c8"); check(castle.board().squares[59] == 'r', "Black queenside castling");
    check(!has(Board::fromFen("r3kr1r/8/8/8/8/8/8/R3K2R w KQ - 0 1"),"e1g1"), "No castling through check");
    check(!has(Board::fromFen("r3k1rr/8/8/8/8/8/8/R3K2R w KQ - 0 1"),"e1g1"), "No castling into check");
    check(!has(Board::fromFen("1k2r3/8/8/8/8/8/8/R3K2R w KQ - 0 1"),"e1c1"), "No castling out of check");
    check(!has(Board::fromFen("r3k2r/8/8/8/8/8/8/RN2K2R w KQ - 0 1"),"e1c1"), "Queenside b-file must be empty");
    Game rights(Board::fromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"));
    play(rights,"h1h2"); play(rights,"h8h7"); play(rights,"h2h1"); play(rights,"h7h8");
    check(!has(rights.board(),"e1g1"), "Rook return cannot restore castling");
    play(rights,"a1a8"); check(!(rights.board().castling & 8), "Rook capture removes castling right");
    Game ep;
    for (const auto& m : {"e2e4","a7a6","e4e5","d7d5"}) play(ep,m);
    check(has(ep.board(),"e5d6"), "En passant available");
    auto expired = ep.board().after(move("g1f3")).after(move("a6a5"));
    check(!has(expired,"e5d6"), "En passant expires");
    play(ep,"e5d6");
    check(ep.board().squares[squareIndex("d5")] == '.' && ep.history().back().captured == 'p', "En passant removes pawn");
    const auto pinnedEp = Board::fromFen("7k/8/8/r4pPK/8/8/8/8 w - f6 0 1");
    check(!has(pinnedEp,"g5f6"), "En passant cannot expose king");
    auto noEp = pinnedEp; noEp.enPassant = -1;
    check(noEp.repetitionKey() == pinnedEp.repetitionKey(), "Unusable en passant ignored in repetition");
    auto usable = Board::fromFen("7k/8/8/5pP1/8/8/8/K7 w - f6 0 1");
    noEp = usable; noEp.enPassant = -1;
    check(noEp.repetitionKey() != usable.repetitionKey(), "Usable en passant distinguishes position");
    for (char p : std::string("qrbn")) {
        Game promotion(Board::fromFen("7k/P7/8/8/8/8/8/7K w - - 0 1"));
        check(!promotion.play(move("a7a8")), "Promotion requires choice");
        play(promotion,std::string("a7a8")+p);
        check(promotion.board().squares[56] == std::toupper(p), "All promotion pieces");
    }
    Game blackPromotion(Board::fromFen("7k/8/8/8/8/8/p7/1R5K b - - 0 1"));
    play(blackPromotion,"a2b1n");
    check(blackPromotion.board().squares[1] == 'n' && blackPromotion.history().back().captured == 'R', "Black capture promotion");
}
void endings() {
    Game mate;
    for (const auto& m : {"f2f3","e7e5","g2g4","d8h4"}) play(mate,m);
    check(mate.status() == "checkmate" && mate.history().back().san == "Qh4#", "Fool's mate");
    check(!mate.play(move("e2e4")), "Cannot move after game ends");
    check(mate.json().find("\"winner\":\"black\"") != std::string::npos, "Winner is black");
    Game stale(Board::fromFen("7k/5K2/6Q1/8/8/8/8/8 b - - 0 1"));
    check(stale.status() == "stalemate", "Stalemate");
    for (const auto& fen : {"7k/8/8/8/8/8/8/K7 w - - 0 1", "7k/8/8/8/8/8/8/KN6 w - - 0 1", "7k/8/8/8/8/8/8/KB6 w - - 0 1", "7k/8/8/8/4b3/8/8/KB6 w - - 0 1"})
        check(Game(Board::fromFen(fen)).status() == "insufficient", "Basic dead material");
    check(!Board::fromFen("7k/8/8/8/8/8/8/KNN5 w - - 0 1").insufficientMaterial(), "Two knights can mate cooperatively");
    check(!Board::fromFen("7k/8/8/8/5b2/8/8/KB6 w - - 0 1").insufficientMaterial(), "Opposite bishops not automatically dead");
    Game repetition;
    check(!repetition.claimDraw(), "Reject premature claim");
    cycle(repetition);
    for (const auto& m : {"g1f3","g8f6","f3g1"}) play(repetition,m);
    check(!repetition.canClaimDraw(), "Third repetition not yet on board");
    const Move announced = move("f6g8");
    auto announcedGame = repetition;
    check(announcedGame.claimDraw(&announced) && announcedGame.history().size() == 7, "Claim intended third repetition without executing move");
    play(repetition,"f6g8");
    check(repetition.canClaimDraw() && !repetition.finished(), "Threefold is claimable, not automatic");
    auto claim = repetition;
    check(claim.claimDraw() && claim.status() == "claimed", "Claim repetition");
    cycle(repetition); cycle(repetition);
    check(repetition.status() == "fivefold", "Fivefold automatic");
    Game fifty(Board::fromFen("7k/8/8/8/8/8/R7/K7 w - - 99 1"));
    check(!fifty.canClaimDraw() && !fifty.claimMoves().empty(), "Intended 50th move");
    play(fifty,"a2b2");
    check(fifty.canClaimDraw() && fifty.claimDraw(), "50 move claim");
    Game seventyfive(Board::fromFen("7k/8/8/8/8/8/R7/K7 w - - 149 1"));
    play(seventyfive,"a2b2"); check(seventyfive.status() == "seventyfive", "75 moves automatic");
    Game priority(Board::fromFen("7k/8/5KQ1/8/8/8/8/8 w - - 149 1"));
    play(priority,"g6g7"); check(priority.status() == "checkmate", "Mate takes precedence over 75 moves");
    Game pawnReset(Board::fromFen("7k/8/8/8/8/8/P7/K7 w - - 99 1"));
    play(pawnReset,"a2a3"); check(pawnReset.board().halfmove == 0 && !pawnReset.canClaimDraw(), "Pawn resets halfmove");
    Game agreement;
    check(agreement.agreeDraw() && agreement.status() == "agreement" && !agreement.agreeDraw(), "Agreement ends game once");
    agreement.reset(); check(agreement.status() == "playing" && !agreement.canClaimDraw(), "Reset result/repetitions");
}
}

int main() {
    try {
        initialAndPerft(); movement(); specialMoves(); endings();
        std::cout << "PASS: " << checks << " checks (including perft); NDEBUG-safe.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "FAIL after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}
