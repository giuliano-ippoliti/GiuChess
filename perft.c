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
};

static float piece_value(char type) {
	switch (type) {
		case 'p': return PAWN_VAL;
		case 'n': return KNIGHT_VAL;
		case 'b': return BISHOP_VAL;
		case 'r': return ROOK_VAL;
		case 'q': return QUEEN_VAL;
	}
	return 0.0;
}

// Picks the array slot for a piece, following the engine's conventions:
// king in 4, queen-side rook in 0, king-side rook in 7, pawn in 8+column
// (en passant relies on it). Returns -1 if no slot fits.
static int pick_slot(piece *x, char type, int row, int col, int home_row) {
	int i;

	if (type == 'k')
		return (x[4].bm_pos == 0) ? 4 : -1;
	if (type == 'p')
		return (x[8+col].bm_pos == 0) ? 8+col : -1;
	if ((type == 'r') && (row == home_row) && (col == 0) && (x[0].bm_pos == 0))
		return 0;
	if ((type == 'r') && (row == home_row) && (col == 7) && (x[7].bm_pos == 0))
		return 7;
	for (i = 0; i <= 7; i++) {
		if ((i == 0) || (i == 4) || (i == 7))
			continue;
		if (x[i].bm_pos == 0)
			return i;
	}
	// rooks' slots are free if not used by castling rooks
	if (x[0].bm_pos == 0)
		return 0;
	if (x[7].bm_pos == 0)
		return 7;
	return -1;
}

// Loads a FEN into w[] and b[]. Returns the side to move (1 white,
// -1 black), or 0 if the position cannot be represented.
static int load_fen(const char *fen) {
	int i, row, col, slot, color;
	char c, type;
	piece *x;
	const char *p;

	for (i = 0; i <= 15; i++) {
		memset(&w[i], 0, sizeof(piece));
		memset(&b[i], 0, sizeof(piece));
		w[i].color = 'W';
		b[i].color = 'B';
		// pieces not given castling rights are considered as moved
		w[i].deja_moved = 1;
		b[i].deja_moved = 1;
	}

	p = fen;
	row = 7;
	col = 0;
	while ((c = *p++) != ' ') {
		if (c == '\0')
			return 0;
		if (c == '/') {
			row--;
			col = 0;
			continue;
		}
		if ((c >= '1') && (c <= '8')) {
			col += c - '0';
			continue;
		}
		if ((c >= 'A') && (c <= 'Z')) {
			x = w;
			type = c - 'A' + 'a';
			slot = pick_slot(w, type, row, col, 0);
		}
		else {
			x = b;
			type = c;
			slot = pick_slot(b, type, row, col, 7);
		}
		if (slot < 0)
			return 0;
		x[slot].type = type;
		x[slot].value = piece_value(type);
		conv_cases_bm(&x[slot].bm_pos, row, col);
		col++;
	}

	color = (*p == 'b') ? -1 : 1;
	p += 2;

	while ((c = *p++) != ' ') {
		if (c == '\0')
			return 0;
		if (c == 'K') { w[4].deja_moved = 0; w[7].deja_moved = 0; }
		if (c == 'Q') { w[4].deja_moved = 0; w[0].deja_moved = 0; }
		if (c == 'k') { b[4].deja_moved = 0; b[7].deja_moved = 0; }
		if (c == 'q') { b[4].deja_moved = 0; b[0].deja_moved = 0; }
	}

	if (*p != '-') {			// en passant target square
		col = p[0] - 'a';
		if (color == 1)
			b[8+col].last_double_move = 1;
		else
			w[8+col].last_double_move = 1;
	}

	// king value is the sum of the others, as in iniz()
	w[4].value = 0;
	b[4].value = 0;
	for (i = 0; i <= 15; i++) {
		if ((i != 4) && (w[i].bm_pos != 0))
			w[4].value += w[i].value;
		if ((i != 4) && (b[i].bm_pos != 0))
			b[4].value += b[i].value;
	}

	// a king "moving" on itself tests all the enemy pieces
	w[4].under_check = is_illegal(4, w, b);
	b[4].under_check = is_illegal(4, b, w);

	rule50 = 0;
	return color;
}

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
			printf("%-10s depth %d: %12llu  expected %12llu  %s\n", tests[t].name, d,
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
