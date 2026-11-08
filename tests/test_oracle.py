"""Independent differential checks. Optional dependency: chess==1.11.2.

Only used by tests; no third-party chess code is shipped in the game.
"""
import json
from pathlib import Path
import random
import subprocess
import sys

import chess

root = Path(__file__).resolve().parents[1]
binary = Path(sys.argv[1]) if len(sys.argv) > 1 else root / "build/oracle_bridge.exe"
rng = random.Random(1729)
positions = 0
transitions = 0
for _ in range(24):
    board = chess.Board()
    samples = []
    for _ in range(120):
        if board.is_game_over():
            break
        samples.append(board.fen(en_passant="fen"))
        board.push(rng.choice(list(board.legal_moves)))
    result = subprocess.run([str(binary)], input="\n".join(samples) + "\n", text=True, capture_output=True, check=True)
    records = result.stdout.splitlines()
    if len(records) != len(samples):
        raise RuntimeError("Wrong number of responses")
    for fen, record in zip(samples, records):
        board = chess.Board(fen)
        actual = json.loads(record)
        expected_moves = {move.uci() for move in board.legal_moves}
        actual_moves = {move["uci"] for move in actual["moves"]}
        if actual_moves != expected_moves:
            raise RuntimeError(f"Legal moves mismatch at {fen}: {actual_moves ^ expected_moves}")
        if actual["check"] != board.is_check() or actual["insufficient"] != board.is_insufficient_material():
            raise RuntimeError(f"Status mismatch at {fen}")
        for outcome in actual["moves"]:
            move = chess.Move.from_uci(outcome["uci"])
            san = board.san(move)
            board.push(move)
            expected_fen = board.fen(en_passant="fen")
            board.pop()
            if outcome["fen"] != expected_fen or outcome["san"] != san:
                raise RuntimeError(f"Transition/SAN mismatch at {fen}, {move}: {outcome} != {expected_fen}, {san}")
            transitions += 1
        positions += 1
print(f"PASS: {positions} positions and {transitions} transitions/SAN compared with python-chess (seed 1729).")
