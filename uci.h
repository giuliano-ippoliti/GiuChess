// This file is a part of the GiuChess Project.
//
// Copyright (c) 2005-2025 Giuliano Ippoliti aka JSorel (ippo@linuxmail.org)
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

#ifndef _UCI_H
#define _UCI_H

#include "main.h"

// UCI protocol functions
void uci_loop();
void uci_send_id();
void uci_send_options();
void uci_parse_position(char *input);
void uci_parse_go(char *input);
void uci_search_and_move();
void uci_set_position_startpos();
void uci_set_position_fen(char *fen);
void uci_make_moves(char *moves_str);
int uci_parse_move(char *move_str);

// UCI utility functions
void uci_send(char *message);
char* uci_move_to_string(int from_square, int to_square, char promotion);
int uci_string_to_square(char *square_str);
void uci_square_to_string(int square, char *square_str);

// UCI search parameters
typedef struct {
    int depth;
    int movetime;
    int wtime;
    int btime;
    int winc;
    int binc;
    int movestogo;
    int infinite;
} UCI_SEARCH_PARAMS;

extern UCI_SEARCH_PARAMS uci_search_params;
extern int uci_mode;

#endif
