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

#include "eval_pos.h"

#define INTERVAL 0.1
//#define RULE50THR 99
#define RULE50THR 49

float eval_material() {
	return w[4].value - b[4].value;
}

float centre(int color) {
	float centre = 0.0;
	int j;
	
	if (color == 1) {
		for (j=0; j<=7; j++) {
			if (j == 4)
				continue;
			if (b[j].bm_pos == 0)
				continue;
			if ((b[j].bm_pos & PERBOR) != 0 )
				centre += 0.01;
			else if ((b[j].bm_pos & PERIF) != 0 )
				centre += 0.1;
			else if ((b[j].bm_pos & CENTRE) != 0 )
				centre += 0.15;
		}
		for (j=10; j<=13; j++) {
			if (b[j].bm_pos == 0)
				continue;
			/*if ((b[j].bm_pos & PERBOR) != 0 )
				centre += 0.01;*/
			else if ((b[j].bm_pos & PERIF) != 0 )
				centre += 0.1;
			else if ((b[j].bm_pos & CENTRE) != 0 )
				centre += 0.3;
		}
	}
	else {
		for (j=0; j<=7; j++) {
			if (j == 4)
				continue;
			if (w[j].bm_pos == 0)
				continue;
			if ((w[j].bm_pos & PERBOR) != 0 )
				centre += 0.01;
			else if ((w[j].bm_pos & PERIF) != 0 )
				centre += 0.1;
			else if ((w[j].bm_pos & CENTRE) != 0 )
				centre += 0.15;
		}
		for (j=10; j<=13; j++) {
			if (w[j].bm_pos == 0)
				continue;
			/*if ((w[j].bm_pos & PERBOR) != 0 )
				centre += 0.01;*/
			else if ((w[j].bm_pos & PERIF) != 0 )
				centre += 0.1;
			else if ((w[j].bm_pos & CENTRE) != 0 )
				centre += 0.3;
		}
	}
	return centre;
}


float evaluate(int color) {		//color which made the last move
//llog("eval\n");
	if (color == 1) {			//white
		if (black_exists_leg_moves(w, b) == 0) {
			if (b[4].under_check)
				return -99.0;			//ATTENTION: - !!!!
			else
				return 0.0;
		}
		else if (rule50 >= RULE50THR)
				return 0.0;
		else
			return (b[4].value - w[4].value + centre(1) + (rand()%10)/50.0 );	// -eval_material(); //+ centre(1)
	}
	else {						//black
		if (white_exists_leg_moves(w, b) == 0) {
			if (w[4].under_check)
				return -99.0;
			else
				return 0.0;
		}
		else if (rule50 >= RULE50THR)
				return 0.0;
		else
			return (w[4].value - b[4].value + centre(-1) + (rand()%10)/50.0 );		// eval_material(); // + centre(-1)
	}
}

#define MAXPLY 64

typedef struct {
	int order;
	char newpiece;
	BITMASK bm_move;
} PV_MOVE;

unsigned long long search_nodes;
static PV_MOVE pv[MAXPLY][MAXPLY];	//principal variation found from each ply
static int pv_len[MAXPLY];
static PV_MOVE prev_pv[MAXPLY];		//principal variation of the previous iteration
static int prev_pv_len, follow_pv;
static int search_stopped;
static long search_limit_ms;
static struct timeval search_start;

static long elapsed_ms() {
	struct timeval now;

	gettimeofday(&now, NULL);
	return (now.tv_sec - search_start.tv_sec)*1000 + (now.tv_usec - search_start.tv_usec)/1000;
}

static void check_time() {
	//depth 1 is always completed, so that there is a move to play
	if ((search_limit_ms > 0) && (depthmax > 1) && (elapsed_ms() >= search_limit_ms))
		search_stopped = 1;
}

// While following the previous principal variation, puts its move for this
// ply at the head of the list
static void pv_to_head(MOVE_LIST *lst, int ply) {
	MOVE_LIST *pos, found;

	if (!follow_pv)
		return;
	if (ply < prev_pv_len) {
		for (pos = lst; *pos != NULL; pos = &(*pos)->next) {
			if (((*pos)->order == prev_pv[ply].order) &&
			    ((*pos)->bm_move == prev_pv[ply].bm_move) &&
			    ((*pos)->newpiece == prev_pv[ply].newpiece)) {
				found = *pos;
				*pos = found->next;
				found->next = *lst;
				*lst = found;
				return;
			}
		}
	}
	follow_pv = 0;
}

static void update_pv(int ply, MOVE_LIST mv) {
	int i;

	pv[ply][0].order = mv->order;
	pv[ply][0].newpiece = mv->newpiece;
	pv[ply][0].bm_move = mv->bm_move;
	for (i = 0; i < pv_len[ply+1]; i++)
		pv[ply][i+1] = pv[ply+1][i];
	pv_len[ply] = pv_len[ply+1] + 1;
}

#define HIST_MAX 2048

typedef struct {
	BITMASK pieces[2][6];		//squares of each piece type, white then black
	int rights;			//castling rights and en passant
} POS_KEY;

static POS_KEY hist[HIST_MAX];		//positions of the game, then of the search path
static int hist_len, hist_root;

static void position_key(POS_KEY *key) {
	const char *types = "pnbrqk";
	piece *x;
	int c, i;

	memset(key, 0, sizeof(POS_KEY));
	for (c = 0; c <= 1; c++) {
		x = (c == 0) ? w : b;
		for (i = 0; i <= 15; i++) {
			if (x[i].bm_pos == 0)
				continue;
			key->pieces[c][strchr(types, x[i].type) - types] |= x[i].bm_pos;
			if ((i >= 8) && x[i].last_double_move)
				key->rights |= 1 << (c*8 + i-8);
		}
		if (!x[4].deja_moved && !x[7].deja_moved && (x[7].bm_pos != 0))
			key->rights |= 1 << (16 + c*2);		//short castle
		if (!x[4].deja_moved && !x[0].deja_moved && (x[0].bm_pos != 0))
			key->rights |= 1 << (17 + c*2);		//long castle
	}
}

// Starts a new game history with the current position
void reset_history() {
	hist_len = 0;
	record_position();
}

// Records the current position of the game (after each move played)
void record_position() {
	if (hist_len < HIST_MAX - MAXPLY)
		position_key(&hist[hist_len++]);
}

// Number of earlier occurrences of the position at index last, among the
// positions since the last capture or pawn move (same side to move)
static int count_repetitions(POS_KEY *key, int last, int from) {
	int i, count = 0;

	for (i = last - 2; (i >= from) && (i >= 0); i -= 2) {
		if (memcmp(&hist[i], key, sizeof(POS_KEY)) == 0)
			count++;
	}
	return count;
}

// XBoard: the current game position occurred three times
int threefold() {
	if (hist_len == 0)
		return 0;
	return count_repetitions(&hist[hist_len-1], hist_len-1, hist_len-1 - rule50) >= 2;
}

// Called after a move in the search: returns 1 if the new position is a
// draw by repetition (already met in the search tree, or a third occurrence
// counting the game), else records it and returns 0
static int repeated_or_push() {
	POS_KEY key;
	int i;

	position_key(&key);
	for (i = hist_len - 2; (i >= hist_len - rule50) && (i >= 0); i -= 2) {
		if (memcmp(&hist[i], &key, sizeof(POS_KEY)) == 0) {
			if (i >= hist_root)
				return 1;
			if (count_repetitions(&key, hist_len, hist_len - rule50) >= 2)
				return 1;
			break;
		}
	}
	hist[hist_len++] = key;
	return 0;
}

float alphabeta(int depth, float alpha, float beta, int color, char *bestmove) {
	float alphaL, val;
	int nlgmv, ply;
	MOVE_LIST tmplist, mvlst, curmove;
	
	piece wbck[16], bbck[16];
	char thismove[6];
	int rule50bck;
	
	ply = depthmax - depth;
	if (ply == 0)
		hist_root = hist_len - 1;	//the root is the last game position
	pv_len[ply] = 0;
	if (search_stopped)
		return 0.0;
	if ((++search_nodes & 1023) == 0)
		check_time();

	if (depth == 0)
		return evaluate(color);
	alphaL = alpha;
	
	if (color == 1) {				//white
		black_leg_mov(w, b);
		crt_list(&mvlst);
		nlgmv = push_leg_mov(b, &mvlst);
		order_moves(&mvlst);
		pv_to_head(&mvlst, ply);
		if (nlgmv == 0) {
			if (b[4].under_check) {		//white's move mated black
				canc_list(&mvlst);
				return -99.0-depth;
			}
			else {
				canc_list(&mvlst);
				return 0.0;           //stalemate
			}
		}
		else {
			//backup
			memcpy(wbck, w, sizeof(w));
			memcpy(bbck, b, sizeof(b));
			rule50bck = rule50;

			tmplist = mvlst;
			while ( tmplist != NULL ) {
				curmove = tmplist;
				if (depth == depthmax) {
					conv_list_mov(thismove, tmplist);
//llog("C1: %s\n", thismove);
				}
//conv_list_mov(thismove2, tmplist);
//llog("C1 - d=%d : %s éxécuting\n", depth, thismove2);
				list_execute_move(tmplist);
//llog("C1 - d=%d : %s éxécuté\n", depth, thismove2);

				if (repeated_or_push())
					val = 0.0;		//draw by repetition
				else {
					val = -alphabeta(depth-1, -beta, -alphaL, -color, bestmove);
					hist_len--;
				}
//llog("d=%d val(%s) = %f\n", depth, thismove2, val);
/*
if (depth == depthmax) {
	llog("%f\n", val);
}*/

				// undo backup
				memcpy(w, wbck, sizeof(w));
				memcpy(b, bbck, sizeof(b));
				rule50 = rule50bck;
				follow_pv = 0;		//the principal variation is only the first move

				if (search_stopped) {
					canc_list(&mvlst);
					return 0.0;
				}
				
				tmplist = tmplist->next;
				
				if (val >= beta) {
					canc_list(&mvlst);
					return beta;
				}
				if (val > alphaL) {
					alphaL = val;
					update_pv(ply, curmove);
					
					if (depth == depthmax) {
						strcpy(bestmove, thismove);
					}
				}
				
			}
			canc_list(&mvlst);
			return alphaL;
		}
	}
	else {
		white_leg_mov(w, b);
		crt_list(&mvlst);
		nlgmv = push_leg_mov(w, &mvlst);
		order_moves(&mvlst);
		pv_to_head(&mvlst, ply);
		if (nlgmv == 0) {
			if (w[4].under_check) {		//white's move mated black
				canc_list(&mvlst);
				return -99.0-depth;
			}
			else {
				canc_list(&mvlst);
				return 0.0;           //stalemate
			}
		}
		else {
			//backup
			memcpy(wbck, w, sizeof(w));
			memcpy(bbck, b, sizeof(b));
			rule50bck = rule50;

			tmplist = mvlst;
			while ( tmplist != NULL ) {
				curmove = tmplist;
				if (depth == depthmax) {
					conv_list_mov(thismove, tmplist);
//llog("C-1: %s\n", thismove);
				}
				
//conv_list_mov(thismove2, tmplist);
//llog("C-1 - d=%d : %s éxécuting\n", depth, thismove2);
				list_execute_move(tmplist);
//llog("C-1 - d=%d : %s éxécuté\n", depth, thismove2);
				if (repeated_or_push())
					val = 0.0;		//draw by repetition
				else {
					val = -alphabeta(depth-1, -beta, -alphaL, -color, bestmove);
					hist_len--;
				}
//llog("d=%d val(%s) = %f\n", depth, thismove2, val);
/*
if (depth == depthmax) {
	llog("%f\n", val);
}*/

				// undo backup
				memcpy(w, wbck, sizeof(w));
				memcpy(b, bbck, sizeof(b));
				rule50 = rule50bck;
				follow_pv = 0;		//the principal variation is only the first move

				if (search_stopped) {
					canc_list(&mvlst);
					return 0.0;
				}
				
				tmplist = tmplist->next;
				
				if (val >= beta) {
					canc_list(&mvlst);
					return beta;
				}
				if (val > alphaL) {
					alphaL = val;
					update_pv(ply, curmove);
					
					if (depth == depthmax) {
						strcpy(bestmove, thismove);
					}
				}
			}
			canc_list(&mvlst);
			return alphaL;
		}
	}
}

// Iterative deepening: searches depth 1, 2, ... maxdepth, trying the previous
// iteration's principal variation first. With limit_ms > 0, stops when the
// time is up and keeps the best move of the last completed depth.
// Returns the last completed depth.
int search(int maxdepth, int color, char *bestmove, long limit_ms) {
	char itermove[10];
	int depth, done;

	if (maxdepth > MAXPLY-1)
		maxdepth = MAXPLY-1;

	gettimeofday(&search_start, NULL);
	search_limit_ms = limit_ms;
	search_stopped = 0;
	search_nodes = 0;
	prev_pv_len = 0;
	done = 0;
	itermove[0] = '\0';

	for (depth = 1; depth <= maxdepth; depth++) {
		depthmax = depth;
		follow_pv = 1;
		alphabeta(depth, -200, 200, color, itermove);
		if (search_stopped)
			break;

		strcpy(bestmove, itermove);
		memcpy(prev_pv, pv[0], pv_len[0]*sizeof(PV_MOVE));
		prev_pv_len = pv_len[0];
		done = depth;

		//a deeper search would most likely not finish in time
		if ((limit_ms > 0) && (elapsed_ms() >= limit_ms/2))
			break;
	}

	return done;
}

void conv_list_mov(char *move, MOVE_LIST movelist) {		//appelé 20/30 fois
	int order;
        char color, newpiece;
	int oldrow, oldcol, newrow, newcol;
        BITMASK bm_old, bm_new;

	order = movelist->order;
	color = movelist->color;
	newpiece = movelist->newpiece;
	bm_new = movelist->bm_move;
	
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
}
