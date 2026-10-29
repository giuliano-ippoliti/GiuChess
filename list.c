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

#include "list.h"

void crt_list(MOVE_LIST *lst) {
	*lst = NULL;
}

int list_void(MOVE_LIST *lst) {
	return (lst == NULL);
}

void *ins_head (MOVE_LIST *new_lst, int order, char color, char newpiece, BITMASK bm_move, MOVE_LIST old_lst) {	//new_list is modified
	MOVE_LIST tmp_lst;
top:
	tmp_lst = (MOVE_LIST)malloc(sizeof(MOVE_LEG));
	
	if (tmp_lst != NULL) {
		tmp_lst->order = order;
		tmp_lst->color = color;
		tmp_lst->newpiece = newpiece;
		tmp_lst->bm_move = bm_move;
		tmp_lst->next = old_lst;
		*new_lst = tmp_lst;
	}
	else {
		llog("ORRORE\n");
		goto top;
	}
		
	return (tmp_lst);
}

void canc_list1(MOVE_LIST *lst) {
	MOVE_LIST tmp_lst;
	
	while (*lst != NULL) {
		tmp_lst = *lst;
		*lst = tmp_lst->next;
		free(tmp_lst);
	}
}

void canc_list(MOVE_LIST *lst) {
	MOVE_LIST tmp_lst;
	
	if (*lst != NULL) {
		tmp_lst = (*lst)->next;
		free(*lst);
		canc_list(&tmp_lst);
		*lst = NULL;
	}
}

void print_lst (MOVE_LIST lst) {
	MOVE_LIST tmp_lst;
	
	tmp_lst = lst;
	while (tmp_lst != NULL) {
		printf("%c\n", tmp_lst->color);
		tmp_lst = tmp_lst->next;
	}
}

void log_lst (MOVE_LIST lst) {
	MOVE_LIST tmp_lst;
	char move[6];
	
	int order;
	char color, newpiece;
	int oldrow, oldcol, newrow, newcol;
	BITMASK bm_old, bm_new;
	
	tmp_lst = lst;
	while (tmp_lst != NULL) {
		
		order = tmp_lst->order;
		color = tmp_lst->color;
		newpiece = tmp_lst->newpiece;
		bm_new = tmp_lst->bm_move;
		
		if (color == 'W')
			bm_old = w[order].bm_pos;
		else
			bm_old = b[order].bm_pos;
			
		conv_bm_cases(bm_old, &oldrow, &oldcol);
		conv_bm_cases(bm_new, &newrow, &newcol);
		
		move[0] = 97+oldcol;
		move[1] = 49+oldrow;
		move[2] = 97+newcol;
		move[3] = 49+newrow;
		
		if (newpiece == '-')
			move[4] = '\0';
		else {
			move[4] = newpiece;
			move[5] = '\0';
		}
		//llog("%d\n", order);
		//llog("%c\n", color);
		llog("%s\n", move);
		tmp_lst = tmp_lst->next;
	}
}

void log_lst2 (MOVE_LIST lst) {
	MOVE_LIST tmp_lst;
	char move[6];
	
	int order;
	char color, newpiece;
	int oldrow, oldcol, newrow, newcol;
	BITMASK bm_old, bm_new;
	
	tmp_lst = lst;
	while (tmp_lst != NULL) {
		
		order = tmp_lst->order;
		color = tmp_lst->color;
		newpiece = tmp_lst->newpiece;
		bm_new = tmp_lst->bm_move;
		
		if (color == 'W')
			bm_old = w[order].bm_pos;
		else
			bm_old = b[order].bm_pos;
			
		conv_bm_cases(bm_old, &oldrow, &oldcol);
		conv_bm_cases(bm_new, &newrow, &newcol);
		
		move[0] = 97+oldcol;
		move[1] = 49+oldrow;
		move[2] = 97+newcol;
		move[3] = 49+newrow;
		
		if (newpiece == '-')
			move[4] = '\0';
		else {
			move[4] = newpiece;
			move[5] = '\0';
		}

		llog("%s	%f\n", move, tmp_lst->evaluation);
		tmp_lst = tmp_lst->next;
	}
}

// MVV-LVA score of a capture (most valuable victim, least valuable
// attacker), always > 0; -1 for a quiet move
float capture_score(MOVE_LIST mv) {
	piece *friends, *enems;
	BITMASK bm_orig, bm_new;
	int i;

	if (mv->color == 'W') {
		friends = w;
		enems = b;
	}
	else {
		friends = b;
		enems = w;
	}
	bm_new = mv->bm_move;

	for (i=0; i<=15; i++) {
		if (enems[i].bm_pos == bm_new)
			return 10*enems[i].value - friends[mv->order].value;
	}

	//en passant: diagonal pawn move to an empty square
	if (friends[mv->order].type == 'p') {
		bm_orig = friends[mv->order].bm_pos;
		if ((bm_new != (bm_orig << 8)) && (bm_new != (bm_orig << 16)) &&
		    (bm_new != (bm_orig >> 8)) && (bm_new != (bm_orig >> 16)))
			return 10*PAWN_VAL - PAWN_VAL;
	}

	return -1;
}

// Puts captures first, best MVV-LVA score first; quiet moves keep their order
void order_moves(MOVE_LIST *lst) {
	MOVE_LIST caps, quiets, tmp_lst;
	MOVE_LIST *pos, *qtail;

	caps = NULL;
	quiets = NULL;
	qtail = &quiets;

	while (*lst != NULL) {
		tmp_lst = *lst;
		*lst = tmp_lst->next;
		tmp_lst->evaluation = capture_score(tmp_lst);

		if (tmp_lst->evaluation < 0) {
			tmp_lst->next = NULL;
			*qtail = tmp_lst;
			qtail = &tmp_lst->next;
		}
		else {				//sorted insertion, stable for equal scores
			pos = &caps;
			while ((*pos != NULL) && ((*pos)->evaluation >= tmp_lst->evaluation))
				pos = &(*pos)->next;
			tmp_lst->next = *pos;
			*pos = tmp_lst;
		}
	}

	pos = &caps;
	while (*pos != NULL)
		pos = &(*pos)->next;
	*pos = quiets;
	*lst = caps;
}
