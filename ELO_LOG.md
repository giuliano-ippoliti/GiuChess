# Elo log

Strength of GiuChess over its development, measured with
[`scripts/elo_match.py`](scripts/elo_match.py): matches against Stockfish
limited in strength (`UCI_LimitStrength` / `UCI_Elo`), every opening played
with both colors. Newest entries at the bottom.

To add an entry after a change (about 20 minutes with the default settings):
```bash
make
python3 scripts/elo_match.py --log ELO_LOG.md
```
It needs python-chess (`pip install chess`) and Stockfish (`brew install stockfish`).
Keep the same settings between entries so they stay comparable, and run it on
a committed state: the entry records the current commit.

These are engine-scale ratings, roughly on the CCRL scale on which Stockfish
calibrates `UCI_Elo`, at a short time control. They are not Lichess or FIDE
ratings. The 95% intervals show the measurement noise: a difference smaller
than the intervals is not significant.

## 2026-10-30: 32d25a3 Merge pull request #25 from giuliano-ippoliti/fix/24-uci-fen

**Estimated Elo: 1472** (95% interval 1398 to 1548), 96 games.

| Opponent | Wins | Draws | Losses | Score | Elo [95%] |
|---|---|---|---|---|---|
| Stockfish 1320 | 31 | 7 | 10 | 71.9% | 1483 [1378, 1598] |
| Stockfish 1600 | 15 | 0 | 33 | 31.2% | 1463 [1352, 1566] |

- Setup: Stockfish 19, UCI_LimitStrength, 1 thread; time control 10 s + 0.1 s; 24 openings, both colors; Darwin arm64.
- Endings: CHECKMATE 89, FIFTY_MOVES 3, INSUFFICIENT_MATERIAL 2, THREEFOLD_REPETITION 2.
- GiuChess average depth: 5.9. Duration: not recorded (run before this script existed, with the same settings).

