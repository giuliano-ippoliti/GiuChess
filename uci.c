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
    // For now, we'll implement a basic FEN parser
    // This is a simplified version - full FEN parsing would be more complex
    uci_set_position_startpos(); // Fallback to startpos for now
    // TODO: Implement full FEN parsing
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
    if (strlen(move_str) < 4) return 0;
    
    // Convert to internal format and execute
    char internal_move[6];
    strcpy(internal_move, move_str);
    
    // Add promotion piece if present
    if (strlen(move_str) == 5) {
        internal_move[4] = move_str[4];
        internal_move[5] = '\0';
    } else {
        internal_move[4] = '\0';
    }
    
    string_execute_move(internal_move, w, b);
    return 1; // Return success
}

void uci_make_moves(char *moves_str) {
    char *token;
    char moves_copy[1000];
    strcpy(moves_copy, moves_str);
    
    token = strtok(moves_copy, " ");
    while (token != NULL) {
        if (!uci_parse_move(token)) {
            // Invalid move
            break;
        }
        token = strtok(NULL, " ");
    }
}

void uci_parse_position(char *input) {
    char *token;
    char input_copy[1000];
    strcpy(input_copy, input);
    
    token = strtok(input_copy, " ");
    token = strtok(NULL, " "); // Skip "position"
    
    if (token && strcmp(token, "startpos") == 0) {
        uci_set_position_startpos();
        token = strtok(NULL, " ");
        
        if (token && strcmp(token, "moves") == 0) {
            // Get the rest of the string for moves
            char *moves_start = strstr(input, "moves");
            if (moves_start) {
                moves_start += 6; // Skip "moves "
                uci_make_moves(moves_start);
            }
        }
    } else if (token && strcmp(token, "fen") == 0) {
        // TODO: Parse FEN string
        uci_set_position_startpos(); // Fallback for now
    }
}

void uci_parse_go(char *input) {
    char *token;
    char input_copy[1000];
    strcpy(input_copy, input);
    
    // Initialize search parameters
    memset(&uci_search_params, 0, sizeof(UCI_SEARCH_PARAMS));
    uci_search_params.depth = STARTDPTH;
    
    token = strtok(input_copy, " ");
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
    int color;
    
    // Determine who is to move
    color = (last_moved_color == 'W') ? 1 : -1; // 1 for black, -1 for white
    
    // Set search depth
    depthmax = uci_search_params.depth;
    
    // Perform search
    alphabeta(depthmax, -200, 200, color, bestmove);
    
    // Send best move
    char output[100];
    sprintf(output, "bestmove %s", bestmove);
    uci_send(output);
    
    // Execute the move internally
    string_execute_move(bestmove, w, b);
}

void uci_loop() {
    char input[4096];
    
    uci_mode = 1;
    
    while (fgets(input, sizeof(input), stdin)) {
        // Remove newline
        input[strcspn(input, "\n")] = 0;
        
        if (strncmp(input, "uci", 3) == 0) {
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
