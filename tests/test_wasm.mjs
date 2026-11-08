import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import createChess from '../web/generated/chess.mjs';

const module = await createChess({ wasmBinary: await readFile(new URL('../web/generated/chess.wasm', import.meta.url)) });
const state = module.cwrap('chess_state', 'string', []);
const play = module.cwrap('chess_move', 'number', ['number', 'number', 'number']);
const reset = module.cwrap('chess_reset', null, []);
const claim = module.cwrap('chess_claim', 'number', ['number', 'number', 'number']);
const agree = module.cwrap('chess_agree', 'number', []);
const get = () => JSON.parse(state());
const square = s => s.charCodeAt(0) - 97 + (Number(s[1]) - 1) * 8;
const move = uci => assert.equal(play(square(uci.slice(0, 2)), square(uci.slice(2, 4)), uci[4]?.charCodeAt(0) || 0), 1, uci);

assert.equal(get().board.length, 64);
assert.equal(get().legalMoves.length, 20);
assert.equal(get().turn, 'white');
assert.equal(play(-1, 999, 0), 0);
assert.equal(play(12, 28, 369), 0, 'Reject promotion integer that truncates to q');
assert.equal(claim(-1, -1, 0), 0);
assert.equal(play(52, 36, 0), 0);
const initial = state();
assert.equal(play(0, 24, 0), 0);
assert.equal(state(), initial, 'Rejected move leaves complete state unchanged');
for (const m of ['f2f3', 'e7e5', 'g2g4', 'd8h4']) move(m);
assert.equal(get().status, 'checkmate');
assert.equal(get().winner, 'black');
assert.equal(get().history.at(-1).san, 'Qh4#');
assert.deepEqual(get().legalMoves, []);
assert.equal(play(12, 28, 0), 0);
reset();
assert.equal(state(), initial);
for (let i = 0; i < 2; i++) for (const m of ['g1f3', 'g8f6', 'f3g1', 'f6g8']) move(m);
assert.equal(get().canClaim, true);
assert.equal(claim(-1, -1, 0), 1);
assert.equal(get().status, 'claimed');
reset();
for (const m of ['e2e4', 'a7a6', 'e4e5', 'd7d5', 'e5d6']) move(m);
assert.equal(get().board[square('d5')], '.');
assert.equal(get().history.at(-1).captured, 'p');
reset();
for (const m of ['g1f3', 'g8f6', 'e2e3', 'e7e6', 'f1e2', 'f8e7', 'e1g1']) move(m);
assert.equal(get().board[5], 'R');
assert.equal(get().history.at(-1).san, 'O-O');
reset();
for (const m of ['a2a4', 'h7h5', 'a4a5', 'h5h4', 'a5a6', 'h4h3', 'a6b7', 'h3g2']) move(m);
assert.equal(get().legalMoves.filter(m => m.from === square('b7') && m.to === square('a8')).length, 4);
assert.equal(play(square('b7'), square('a8'), 0), 0);
move('b7a8n');
assert.equal(get().board[square('a8')], 'N');
assert.equal(get().history.at(-1).captured, 'r');
assert.equal(agree(), 1);
assert.equal(get().status, 'agreement');
reset();
// Deterministic games exercise repeated JSON serialization and the actual ABI.
let seed = 1729;
for (let game = 0; game < 8; game++) {
    reset();
    for (let ply = 0; ply < 150 && get().status === 'playing'; ply++) {
        seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
        const s = get();
        const m = s.legalMoves[seed % s.legalMoves.length];
        assert.equal(play(m.from, m.to, m.promotion.charCodeAt(0) || 0), 1);
        const next = get();
        assert.notEqual(next.turn, s.turn);
        assert.equal(next.history.length, s.history.length + 1);
        assert.equal([...next.board].filter(p => p === 'K').length, 1);
        assert.equal([...next.board].filter(p => p === 'k').length, 1);
    }
}
console.log('PASS: WebAssembly ABI, JSON, special moves, results, reset and 8 deterministic games.');
