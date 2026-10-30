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


// FEN loader, shared by the UCI "position fen" command and the perft test.

#include "fen.h"

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
// king in 4, rooks in 0 and 7, pawn in 8+column (en passant relies on
// it). Falls back on any free slot (doubled pawns, promoted pieces);
// load_fen() then moves the castling rooks and the en passant pawn into
// their slots. Returns -1 if no slot is free.
static int pick_slot(piece *x, char type, int col) {
	int i;

	if (type == 'k')
		return (x[4].bm_pos == 0) ? 4 : -1;
	if ((type == 'p') && (x[8+col].bm_pos == 0))
		return 8+col;
	if (type != 'p') {
		for (i = 0; i <= 7; i++) {
			if ((i == 0) || (i == 4) || (i == 7))
				continue;
			if (x[i].bm_pos == 0)
				return i;
		}
		if (x[0].bm_pos == 0)
			return 0;
		if (x[7].bm_pos == 0)
			return 7;
	}
	for (i = 8; i <= 15; i++)
		if (x[i].bm_pos == 0)
			return i;
	for (i = 0; i <= 7; i++)
		if ((i != 4) && (x[i].bm_pos == 0))
			return i;
	return -1;
}

// Moves the piece standing on bm (if any) into slot, swapping both slots
static void move_to_slot(piece *x, BITMASK bm, int slot) {
	int i;
	piece tmp;

	for (i = 0; i <= 15; i++) {
		if ((i != slot) && (x[i].bm_pos == bm)) {
			tmp = x[slot];
			x[slot] = x[i];
			x[i] = tmp;
			return;
		}
	}
}

// Loads a FEN into w[] and b[], sets the side to move, the fifty moves
// counter and restarts the repetition history. Returns the side to move
// (1 white, -1 black), or 0 if the position cannot be represented.
int load_fen(const char *fen) {
	int i, row, col, slot, color, halfmove;
	char c, type;
	piece *x;
	const char *p;
	BITMASK bm;

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
		if ((row < 0) || (col > 7))
			return 0;
		if ((c >= 'A') && (c <= 'Z')) {
			x = w;
			type = c - 'A' + 'a';
		}
		else {
			x = b;
			type = c;
		}
		if (!strchr("kqrbnp", type))
			return 0;
		slot = pick_slot(x, type, col);
		if (slot < 0)
			return 0;
		x[slot].type = type;
		x[slot].value = piece_value(type);
		conv_cases_bm(&x[slot].bm_pos, row, col);
		col++;
	}
	if ((w[4].bm_pos == 0) || (b[4].bm_pos == 0))
		return 0;

	if ((*p != 'w') && (*p != 'b'))
		return 0;
	color = (*p == 'b') ? -1 : 1;
	p++;
	while (*p == ' ')
		p++;

	// castling rights: the rook must be in its slot (0: a-file, 7: h-file)
	while ((*p != ' ') && (*p != '\0')) {
		c = *p++;
		if (c == 'K') { conv_cases_bm(&bm, 0, 7); move_to_slot(w, bm, 7); w[4].deja_moved = 0; w[7].deja_moved = 0; }
		if (c == 'Q') { conv_cases_bm(&bm, 0, 0); move_to_slot(w, bm, 0); w[4].deja_moved = 0; w[0].deja_moved = 0; }
		if (c == 'k') { conv_cases_bm(&bm, 7, 7); move_to_slot(b, bm, 7); b[4].deja_moved = 0; b[7].deja_moved = 0; }
		if (c == 'q') { conv_cases_bm(&bm, 7, 0); move_to_slot(b, bm, 0); b[4].deja_moved = 0; b[0].deja_moved = 0; }
	}
	while (*p == ' ')
		p++;

	// en passant target square: the pawn that just moved must be in 8+column
	if ((*p >= 'a') && (*p <= 'h')) {
		col = p[0] - 'a';
		x = (color == 1) ? b : w;
		conv_cases_bm(&bm, (color == 1) ? 4 : 3, col);
		move_to_slot(x, bm, 8+col);
		if ((x[8+col].type == 'p') && (x[8+col].bm_pos == bm))
			x[8+col].last_double_move = 1;
	}
	while ((*p != ' ') && (*p != '\0'))
		p++;

	// halfmove clock (optional)
	halfmove = 0;
	sscanf(p, "%d", &halfmove);

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

	rule50 = halfmove;
	last_moved_color = (color == 1) ? 'B' : 'W';
	reset_history();
	return color;
}
