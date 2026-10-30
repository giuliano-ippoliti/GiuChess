#!/usr/bin/env python3
# This file is a part of the GiuChess Project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.

"""Estimates the Elo rating of GiuChess with matches against Stockfish
limited in strength (UCI_LimitStrength / UCI_Elo).

Each level plays every opening twice, once with each color. Games are
adjudicated by python-chess: mate, stalemate, threefold repetition, fifty
moves, insufficient material, illegal move, loss on time or engine crash.

Requirements: python-chess (pip install chess) and Stockfish
(brew install stockfish). Build the engine first with "make".

Example:
    python3 scripts/elo_match.py --levels 1320 1600 --log ELO_LOG.md
"""

import argparse, datetime, json, math, os, platform, shutil, subprocess, sys, time
from concurrent.futures import ProcessPoolExecutor, as_completed

try:
    import chess
except ImportError:
    sys.exit("python-chess is required: pip install chess")

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

OPENINGS = [
    "e2e4 e7e5 g1f3 b8c6 f1b5", "e2e4 e7e5 g1f3 b8c6 f1c4", "e2e4 c7c5 g1f3 d7d6 d2d4",
    "e2e4 c7c5 b1c3 b8c6 g2g3", "e2e4 e7e6 d2d4 d7d5 b1c3", "e2e4 c7c6 d2d4 d7d5 e4e5",
    "e2e4 d7d5 e4d5 d8d5 b1c3", "e2e4 g8f6 e4e5 f6d5 d2d4", "e2e4 d7d6 d2d4 g8f6 b1c3",
    "e2e4 e7e5 f2f4 e5f4 g1f3", "e2e4 e7e5 g1f3 g8f6 f3e5", "e2e4 e7e5 b1c3 g8f6 f1c4",
    "d2d4 d7d5 c2c4 e7e6 b1c3", "d2d4 d7d5 c2c4 d5c4 g1f3", "d2d4 g8f6 c2c4 e7e6 g1f3",
    "d2d4 g8f6 c2c4 g7g6 b1c3", "d2d4 g8f6 c2c4 c7c5 d4d5", "d2d4 f7f5 g2g3 g8f6 f1g2",
    "d2d4 d7d5 g1f3 g8f6 c1f4", "c2c4 e7e5 b1c3 g8f6 g1f3", "c2c4 c7c5 g1f3 b8c6 b1c3",
    "g1f3 d7d5 g2g3 g8f6 f1g2", "g1f3 g8f6 c2c4 b7b6 g2g3", "b2b3 e7e5 c1b2 b8c6 e2e3",
]
MAXPLIES = 400      # adjudicated as a draw beyond this


class Engine:
    def __init__(self, argv, options=()):
        self.p = subprocess.Popen(argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                  stderr=subprocess.DEVNULL, text=True, bufsize=1)
        self.send("uci")
        self.name = next((l[8:] for l in self.wait("uciok") if l.startswith("id name ")), argv[0])
        for o in options:
            self.send("setoption name " + o)
        self.send("isready")
        self.wait("readyok")

    def send(self, s):
        self.p.stdin.write(s + "\n")
        self.p.stdin.flush()

    def wait(self, prefix):
        lines = []
        while True:
            l = self.p.stdout.readline()
            if l == "":
                raise RuntimeError("engine died")
            l = l.strip()
            lines.append(l)
            if l.startswith(prefix):
                return lines

    def quit(self):
        try:
            self.send("quit")
            self.p.wait(timeout=5)
        except Exception:
            self.p.kill()


def stockfish_options(level):
    return ("Threads value 1", "Hash value 16", "UCI_LimitStrength value true",
            "UCI_Elo value %d" % level)


def play(job):
    """Plays one game; returns its result from GiuChess's point of view."""
    level, gid, opening, giu_white, giu_argv, sf_path, base, inc = job
    giu_color = chess.WHITE if giu_white else chess.BLACK
    engs = {giu_color: Engine(giu_argv), not giu_color: Engine([sf_path], stockfish_options(level))}
    names = {giu_color: "GiuChess", not giu_color: "Stockfish"}
    board = chess.Board()
    moves = opening.split()
    for m in moves:
        board.push_uci(m)
    clock = {chess.WHITE: base, chess.BLACK: base}
    depths = []
    try:
        while True:
            outcome = board.outcome(claim_draw=False)
            if outcome:
                result, reason = board.result(), outcome.termination.name
                break
            if board.is_repetition(3):
                result, reason = "1/2-1/2", "THREEFOLD_REPETITION"
                break
            if board.halfmove_clock >= 100:
                result, reason = "1/2-1/2", "FIFTY_MOVES"
                break
            if len(board.move_stack) >= MAXPLIES:
                result, reason = "1/2-1/2", "PLY_LIMIT"
                break
            side = board.turn
            eng = engs[side]
            eng.send("position startpos moves " + " ".join(moves))
            t0 = time.time()
            eng.send("go wtime %d btime %d winc %d binc %d"
                     % (clock[chess.WHITE], clock[chess.BLACK], inc, inc))
            lines = eng.wait("bestmove")
            clock[side] -= int((time.time() - t0) * 1000)
            if side == giu_color:
                info = [l for l in lines if l.startswith("info depth")]
                if info:
                    depths.append(int(info[-1].split()[2]))
            mv = lines[-1].split()[1]
            try:
                m = chess.Move.from_uci(mv)
                legal = m in board.legal_moves
            except ValueError:
                legal = False
            loser = "0-1" if side == chess.WHITE else "1-0"
            if not legal:
                result, reason = loser, "ILLEGAL_MOVE by %s (%s)" % (names[side], mv)
                break
            if clock[side] <= 0:
                result, reason = loser, "TIME_FORFEIT by %s" % names[side]
                break
            clock[side] += inc
            board.push(m)
            moves.append(mv)
    except RuntimeError:
        result = "0-1" if board.turn == chess.WHITE else "1-0"
        reason = "CRASH of %s" % names[board.turn]
    for e in engs.values():
        e.quit()
    score = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": 0.5}[result]
    if not giu_white:
        score = 1 - score
    return dict(level=level, gid=gid, opening=opening, giu_white=giu_white, result=result,
                reason=reason, plies=len(board.move_stack), score=score,
                depth=sum(depths) / len(depths) if depths else None, moves=" ".join(moves))


def expected(rating, opponent):
    return 1 / (1 + 10 ** ((opponent - rating) / 400))


def max_likelihood(games):
    """Rating (and 95% interval) that best explains the scores against all levels."""
    def ll(r):
        return sum(g["score"] * math.log(expected(r, g["level"]))
                   + (1 - g["score"]) * math.log(1 - expected(r, g["level"])) for g in games)
    grid = [x / 2 for x in range(0, 8000)]
    best = max(grid, key=ll)
    top = ll(best)
    inside = [r for r in grid if ll(r) >= top - 1.92]     # likelihood ratio, chi2(1) 95%
    return best, inside[0], inside[-1]


def summary(games):
    w = sum(g["score"] == 1 for g in games)
    d = sum(g["score"] == 0.5 for g in games)
    l = sum(g["score"] == 0 for g in games)
    return w, d, l, (w + d / 2) / len(games)


def git_describe():
    try:
        out = subprocess.run(["git", "-C", REPO, "log", "-1", "--format=%h %s"],
                             capture_output=True, text=True).stdout.strip()
        dirty = subprocess.run(["git", "-C", REPO, "status", "--porcelain", "--untracked-files=no"],
                               capture_output=True, text=True).stdout.strip()
        return out + (" (with uncommitted changes)" if dirty else "")
    except OSError:
        return "unknown"


def log_entry(games, levels, args, sf_name, elapsed):
    rows = []
    for level in levels:
        g = [x for x in games if x["level"] == level]
        w, d, l, p = summary(g)
        if 0 < p < 1:
            elo = "%.0f [%.0f, %.0f]" % max_likelihood(g)
        else:
            elo = "out of range (%s than %d)" % ("stronger" if p == 1 else "weaker", level)
        rows.append("| Stockfish %d | %d | %d | %d | %.1f%% | %s |" % (level, w, d, l, 100 * p, elo))
    r, lo, hi = max_likelihood(games)
    reasons = {}
    for g in games:
        key = g["reason"].split(" by ")[0]
        reasons[key] = reasons.get(key, 0) + 1
    depths = [g["depth"] for g in games if g["depth"]]
    return "\n".join([
        "## %s: %s" % (datetime.date.today().isoformat(), git_describe()),
        "",
        "**Estimated Elo: %.0f** (95%% interval %.0f to %.0f), %d games." % (r, lo, hi, len(games)),
        "",
        "| Opponent | Wins | Draws | Losses | Score | Elo [95%] |",
        "|---|---|---|---|---|---|",
        *rows,
        "",
        "- Setup: %s, UCI_LimitStrength, 1 thread; time control %g s + %g s; %d openings, both colors; %s %s."
        % (sf_name, args.base, args.inc, args.openings, platform.system(), platform.machine()),
        "- Endings: %s." % ", ".join("%s %d" % kv for kv in sorted(reasons.items(), key=lambda kv: -kv[1])),
        "- GiuChess average depth: %.1f. Duration: %d min." % (sum(depths) / len(depths), elapsed / 60),
        "", ""])


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--engine", default=os.path.join(REPO, "GiuChess-2.0"), help="GiuChess binary")
    ap.add_argument("--stockfish", default=shutil.which("stockfish"), help="Stockfish binary")
    ap.add_argument("--levels", type=int, nargs="+", default=[1320, 1600], help="Stockfish UCI_Elo levels")
    ap.add_argument("--base", type=float, default=10, help="base time per game, seconds")
    ap.add_argument("--inc", type=float, default=0.1, help="increment per move, seconds")
    ap.add_argument("--workers", type=int, default=max(1, (os.cpu_count() or 2) // 2),
                    help="games played in parallel")
    ap.add_argument("--openings", type=int, default=len(OPENINGS),
                    help="number of openings to play (each with both colors)")
    ap.add_argument("--json", help="write every game (moves included) to this file")
    ap.add_argument("--log", help="append a Markdown entry with the results to this file")
    args = ap.parse_args()

    if not args.stockfish:
        sys.exit("Stockfish not found: install it (brew install stockfish) or pass --stockfish")
    if not os.access(args.engine, os.X_OK):
        sys.exit("GiuChess not found at %s: build it with make, or pass --engine" % args.engine)
    sf = Engine([args.stockfish])
    sf_name = sf.name
    sf.quit()

    jobs = []
    for level in args.levels:
        for o in OPENINGS[:args.openings]:
            for giu_white in (True, False):
                jobs.append((level, len(jobs) + 1, o, giu_white, [args.engine, "uci"], args.stockfish,
                             int(args.base * 1000), int(args.inc * 1000)))
    print("%d games against %s at %s, %g s + %g s, %d in parallel"
          % (len(jobs), sf_name, ", ".join(map(str, args.levels)), args.base, args.inc, args.workers),
          flush=True)

    start = time.time()
    games = []
    with ProcessPoolExecutor(args.workers) as ex:
        for f in as_completed([ex.submit(play, j) for j in jobs]):
            g = f.result()
            games.append(g)
            done = [x for x in games if x["level"] == g["level"]]
            w, d, l, p = summary(done)
            print("SF%d game %3d GiuChess=%s %-7s %-22s plies=%3d depth=%s  | SF%d: +%d =%d -%d (%.1f%%)"
                  % (g["level"], g["gid"], "W" if g["giu_white"] else "B", g["result"], g["reason"],
                     g["plies"], "%.1f" % g["depth"] if g["depth"] else "-", g["level"], w, d, l, 100 * p),
                  flush=True)

    games.sort(key=lambda g: g["gid"])
    if args.json:
        with open(args.json, "w") as f:
            json.dump(games, f, indent=1)
    entry = log_entry(games, args.levels, args, sf_name, time.time() - start)
    print("\n" + entry)
    if args.log:
        with open(args.log, "a") as f:
            f.write(entry)
        print("Appended to %s" % args.log)


if __name__ == "__main__":
    main()
