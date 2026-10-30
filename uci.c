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

#include "uci.h"

UCI_SEARCH_PARAMS uci_search_params;
int uci_mode = 0;

void uci_send(char *message) {
    printf("%s\n", message);
    fflush(stdout);
}

void uci_send_id() {
    uci_send("id name GiuChess-2.0");
    uci_send("id author Giuliano Ippoliti");
}

void uci_send_options() {
    uci_send("option name Hash type spin default 32 min 1 max 1024");
    uci_send("option name Threads type spin default 1 min 1 max 1");
    uci_send("option name Depth type spin default 5 min 1 max 10");
}

void uci_set_position_startpos() {
    // Reset to starting position
    iniz(w, b);
    last_moved_color = 'B'; // White to move initially
}

void uci_set_position_fen(char *fen) {
    if (!load_fen(fen))
        uci_set_position_startpos(); // Position that cannot be represented
}

int uci_string_to_square(char *square_str) {
    if (strlen(square_str) != 2) return -1;
    
    int file = square_str[0] - 'a'; // 0-7
    int rank = square_str[1] - '1'; // 0-7
    
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return -1;
    
    return rank * 8 + file;
}

void uci_square_to_string(int square, char *square_str) {
    int file = square % 8;
    int rank = square / 8;
    
    square_str[0] = 'a' + file;
    square_str[1] = '1' + rank;
    square_str[2] = '\0';
}

int uci_parse_move(char *move_str) {
    size_t len = strlen(move_str);

    if (len < 4 || len > 5) return 0;
    string_execute_move(move_str, w, b);
    return 1; // Return success
}

// Plays the moves of a "position" command; moves_str is modified (strtok)
void uci_make_moves(char *moves_str) {
    char *token;

    token = strtok(moves_str, " ");
    while (token != NULL) {
        if (!uci_parse_move(token)) {
            // Invalid move
            break;
        }
        token = strtok(NULL, " ");
    }
}

// "position startpos|fen <fen> [moves <m1> <m2> ...]"; input is modified.
// No fixed-size copy: the moves list grows with the game.
void uci_parse_position(char *input) {
    char *moves;

    moves = strstr(input, " moves");
    if (moves) {
        *moves = '\0';
        moves += 6; // Skip " moves"
    }

    if (strncmp(input, "position startpos", 17) == 0) {
        uci_set_position_startpos();
    } else if (strncmp(input, "position fen ", 13) == 0) {
        uci_set_position_fen(input + 13);
    } else {
        return;
    }

    if (moves)
        uci_make_moves(moves);
}

void uci_parse_go(char *input) {
    char *token;
    
    // Initialize search parameters
    memset(&uci_search_params, 0, sizeof(UCI_SEARCH_PARAMS));
    // depth 0: not given, resolved in uci_search_and_move()
    
    token = strtok(input, " ");
    token = strtok(NULL, " "); // Skip "go"
    
    while (token != NULL) {
        if (strcmp(token, "depth") == 0) {
            token = strtok(NULL, " ");
            if (token) uci_search_params.depth = atoi(token);
        } else if (strcmp(token, "movetime") == 0) {
            token = strtok(NULL, " ");
            if (token) uci_search_params.movetime = atoi(token);
        } else if (strcmp(token, "wtime") == 0) {
            token = strtok(NULL, " ");
            if (token) uci_search_params.wtime = atoi(token);
        } else if (strcmp(token, "btime") == 0) {
            token = strtok(NULL, " ");
            if (token) uci_search_params.btime = atoi(token);
        } else if (strcmp(token, "winc") == 0) {
            token = strtok(NULL, " ");
            if (token) uci_search_params.winc = atoi(token);
        } else if (strcmp(token, "binc") == 0) {
            token = strtok(NULL, " ");
            if (token) uci_search_params.binc = atoi(token);
        } else if (strcmp(token, "movestogo") == 0) {
            token = strtok(NULL, " ");
            if (token) uci_search_params.movestogo = atoi(token);
        } else if (strcmp(token, "infinite") == 0) {
            uci_search_params.infinite = 1;
        }
        token = strtok(NULL, " ");
    }
}

void uci_search_and_move() {
    char bestmove[10];
    int color, depth, reached, mytime, myinc, movestogo;
    long limit_ms;
    struct timeval start, end;
    
    // Determine who is to move
    color = (last_moved_color == 'W') ? 1 : -1; // 1 for black, -1 for white
    
    // Time for this move: movetime, or a share of the remaining clock
    mytime = (color == -1) ? uci_search_params.wtime : uci_search_params.btime;
    myinc = (color == -1) ? uci_search_params.winc : uci_search_params.binc;
    limit_ms = 0;
    if (uci_search_params.movetime > 0) {
        limit_ms = uci_search_params.movetime;
    } else if (mytime > 0) {
        movestogo = (uci_search_params.movestogo > 0) ? uci_search_params.movestogo : 30;
        limit_ms = mytime / movestogo + myinc / 2;
        if (limit_ms > mytime / 2)
            limit_ms = mytime / 2;
    }
    
    // Set search depth: as given, else unlimited with a time limit, else default
    depth = uci_search_params.depth;
    if (depth <= 0)
        depth = (limit_ms > 0) ? MAX_SEARCH_DEPTH : STARTDPTH;
    
    // Perform search
    gettimeofday(&start, NULL);
    reached = search(depth, color, bestmove, limit_ms);
    gettimeofday(&end, NULL);
    
    char output[100];
    sprintf(output, "info depth %d time %ld nodes %llu", reached,
            (long)((end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) / 1000),
            search_nodes);
    uci_send(output);
    
    // Send best move
    sprintf(output, "bestmove %s", bestmove);
    uci_send(output);
    
    // Execute the move internally
    string_execute_move(bestmove, w, b);
}

void uci_loop() {
    // Long games send long "position ... moves" lines (about 5 bytes per ply)
    static char input[65536];
    
    uci_mode = 1;
    
    while (fgets(input, sizeof(input), stdin)) {
        // Remove newline
        input[strcspn(input, "\r\n")] = 0;
        
        if (strcmp(input, "uci") == 0) {
            uci_send_id();
            uci_send_options();
            uci_send("uciok");
        }
        else if (strncmp(input, "isready", 7) == 0) {
            uci_send("readyok");
        }
        else if (strncmp(input, "ucinewgame", 10) == 0) {
            uci_set_position_startpos();
        }
        else if (strncmp(input, "position", 8) == 0) {
            uci_parse_position(input);
        }
        else if (strncmp(input, "go", 2) == 0) {
            uci_parse_go(input);
            uci_search_and_move();
        }
        else if (strncmp(input, "stop", 4) == 0) {
            // TODO: Implement search stopping
        }
        else if (strncmp(input, "quit", 4) == 0) {
            break;
        }
        else if (strncmp(input, "setoption", 9) == 0) {
            // TODO: Implement option setting
        }
    }
}
