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

#include "main.h" 

// Global variables definitions
char last_moved_color;
piece w[16], b[16];   //w et b sont des piece*
BITMASK brow[8], bcol[8], matrix[8][8];
int arr_row[64], arr_col[64];
BITMASK array[64];
int DEBUGG;
int depthmax;
int rule50;
int cont;


int main(int argc, char *argv[]) {
	init_array();

	init_matrix();
	
	init_rc_arr();
	
	iniz(w, b);
//log_bin(TOP);log_bin(TOP ^ RET_ERR);exit(1);
//test_game2b();exit(1);

/*MOVE_LIST lst;
crt_list(&lst);
lst = ins_head(&lst, 0, 'W', '-', w[0].bm_pos, lst);
lst = ins_head(&lst, 0, 'B', '-', b[0].bm_pos, lst);
lst = ins_head(&lst, 0, 'X', '-', b[0].bm_pos, lst);
print_lst (lst);
canc_list(&lst);
print_lst (lst);
exit(4);*/
	
	/*srand(2);
	test_game2();exit(1);*/

	/*int seed=0;
	while(1) {
		srand(seed);
		printf("%d\n", seed);
		init_array();

		init_matrix();
	
		init_rc_arr();
	
		iniz(w, b);
		test_game2();
		seed++;
	}*/

	// Check for UCI mode
	if (argc > 1 && strcmp(argv[1], "uci") == 0) {
		uci_loop();
	} else {
		// Default to XBoard mode
		wait_for_input();
	}
}
