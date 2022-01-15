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

char last_moved_color;
piece w[16], b[16];
BITMASK brow[8], bcol[8], matrix[8][8];
int arr_row[64], arr_col[64];
BITMASK array[64];
int DEBUGG;
int depthmax;

int main() {
  init_array();
  init_matrix();
  init_rc_arr();
  iniz(w, b);
  wait_for_input();
}
