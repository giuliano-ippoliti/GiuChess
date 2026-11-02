// This file is a part of the GiuChess Project.
//
// Copyright (c) 2005 Giuliano Ippoliti aka JSorel (ippo@linuxmail.org)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

// Perft test: counts leaf nodes of the legal move tree using the engine's
// own move generation and compares them to known reference values.
// Build and run with "make perft". Exits non-zero on any mismatch.

#include "main.h"

// Global variables definitions (main.c is not linked in this program)
char last_moved_color;
piece w[16], b[16];
BITMASK brow[8], bcol[8], matrix[8][8];
int arr_row[64], arr_col[64];
BITMASK array[64];
int DEBUGG;
int depthmax;
int rule50;
int cont;

typedef struct {
	const char *name;
	const char *fen;		// NULL: engine start position (iniz)
	int maxdepth;
	unsigned long long expected[6];	// expected[d-1] = nodes at depth d
} PERFT_TEST;

// Reference values: https://www.chessprogramming.org/Perft_Results
static const PERFT_TEST tests[] = {
	{ "startpos", NULL, 5,
	  { 20ULL, 400ULL, 8902ULL, 197281ULL, 4865609ULL } },
	{ "kiwipete", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 3,
	  { 48ULL, 2039ULL, 97862ULL } },
	{ "position3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 5,
	  { 14ULL, 191ULL, 2812ULL, 43238ULL, 674624ULL } },
	// Promotions (with and without capture) and castling rights lost to captures
	{ "position4", "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", 4,
	  { 6ULL, 264ULL, 9467ULL, 422333ULL } },
	// Position 4 mirrored, black to move: must give the same counts
	{ "position4b", "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ -", 4,
	  { 6ULL, 264ULL, 9467ULL, 422333ULL } },
	// Promotion on d8 with capture, discovered checks
	{ "position5", "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", 4,
	  { 44ULL, 1486ULL, 62379ULL, 2103487ULL } },
};

static unsigned long long perft(int depth, int color) {
	unsigned long long nodes;
	int nlgmv;
	MOVE_LIST mvlst, tmplist;
	piece wbck[16], bbck[16];
	int rule50bck;

	if (color == 1)
		white_leg_mov(w, b);
	else
		black_leg_mov(w, b);
	crt_list(&mvlst);
	nlgmv = push_leg_mov((color == 1) ? w : b, &mvlst);

	if (depth == 1) {
		canc_list(&mvlst);
		return nlgmv;
	}

	memcpy(wbck, w, sizeof(w));
	memcpy(bbck, b, sizeof(b));
	rule50bck = rule50;

	nodes = 0;
	for (tmplist = mvlst; tmplist != NULL; tmplist = tmplist->next) {
		list_execute_move(tmplist);
		nodes += perft(depth-1, -color);
		memcpy(w, wbck, sizeof(w));
		memcpy(b, bbck, sizeof(b));
		rule50 = rule50bck;
	}

	canc_list(&mvlst);
	return nodes;
}

int main(void) {
	int t, d, color, failures;
	unsigned long long nodes;

	init_array();
	init_matrix();
	init_rc_arr();

	failures = 0;
	for (t = 0; t < (int)(sizeof(tests)/sizeof(tests[0])); t++) {
		for (d = 1; d <= tests[t].maxdepth; d++) {
			if (tests[t].fen == NULL) {
				iniz(w, b);
				color = 1;
			}
			else {
				color = load_fen(tests[t].fen);
				if (color == 0) {
					printf("%s: cannot load FEN\n", tests[t].name);
					failures++;
					break;
				}
			}
			nodes = perft(d, color);
			printf("%-11s depth %d: %12llu  expected %12llu  %s\n", tests[t].name, d,
			       nodes, tests[t].expected[d-1],
			       (nodes == tests[t].expected[d-1]) ? "OK" : "MISMATCH");
			if (nodes != tests[t].expected[d-1])
				failures++;
		}
	}

	if (failures) {
		printf("perft: %d failure(s)\n", failures);
		return 1;
	}
	printf("perft: all OK\n");
	return 0;
}
