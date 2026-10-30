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
#include <sys/select.h>

UCI_SEARCH_PARAMS uci_search_params;
int uci_mode = 0;

// "Depth" option: search depth when "go" gives neither depth nor time
static int uci_default_depth = STARTDPTH;
// Commands received during a search
static int uci_stop_received, uci_quit_received;

// Input is read with read() into this buffer (not stdio), so that select()
// tells whether a command is waiting while searching.
// Long games send long "position ... moves" lines (about 5 bytes per ply).
static char uci_inbuf[65536];
static size_t uci_inlen;
static int uci_eof;

// Reads what stdin has into uci_inbuf, blocking only if wait is set
static void uci_fill_input(int wait) {
    fd_set fds;
    struct timeval tv = {0, 0};
    ssize_t n;

    if (uci_eof || uci_inlen >= sizeof(uci_inbuf) - 1)
        return;
    if (!wait) {
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        if (select(1, &fds, NULL, NULL, &tv) <= 0)
            return;
    }
    n = read(0, uci_inbuf + uci_inlen, sizeof(uci_inbuf) - 1 - uci_inlen);
    if (n <= 0)
        uci_eof = 1;
    else
        uci_inlen += n;
}

// Length of the first complete line in uci_inbuf (newline included), or 0.
// A full buffer, or the end of input, ends a line.
static size_t uci_first_line() {
    char *nl = memchr(uci_inbuf, '\n', uci_inlen);

    if (nl)
        return nl - uci_inbuf + 1;
    if (uci_inlen > 0 && (uci_eof || uci_inlen >= sizeof(uci_inbuf) - 1))
        return uci_inlen;
    return 0;
}

// Moves the first line (without "\r\n") into line and removes it from the buffer
static void uci_take_line(char *line, size_t size, size_t len) {
    size_t copy = (len < size) ? len : size - 1;

    memcpy(line, uci_inbuf, copy);
    line[copy] = '\0';
    line[strcspn(line, "\r\n")] = '\0';
    memmove(uci_inbuf, uci_inbuf + len, uci_inlen - len);
    uci_inlen -= len;
}

// Reads the next command, waiting for it. Returns 0 at the end of input.
static int uci_read_line(char *line, size_t size) {
    size_t len;

    while ((len = uci_first_line()) == 0) {
        if (uci_eof)
            return 0;
        uci_fill_input(1);
    }
    uci_take_line(line, size, len);
    return 1;
}

// Handles the commands allowed during a search ("stop", "quit", "isready",
// "ponderhit"). Any other command is left for after the search.
// Returns 1 if the search must stop.
static int uci_poll() {
    char line[64];
    size_t len;

    uci_fill_input(0);
    while ((len = uci_first_line()) != 0) {
        if (len >= sizeof(line))
            break;
        memcpy(line, uci_inbuf, len);
        line[len] = '\0';
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "stop") == 0)
            uci_stop_received = 1;
        else if (strcmp(line, "quit") == 0)
            uci_quit_received = 1;
        else if (strcmp(line, "isready") == 0)
            uci_send("readyok");
        else if (strcmp(line, "ponderhit") != 0)
            break;
        uci_take_line(line, sizeof(line), len);
    }
    return uci_stop_received || uci_quit_received;
}

// Sends the result of each completed depth: score, nodes, time, principal variation
static void uci_report(int depth, float score, long ms, unsigned long long nodes, const char *pv) {
    char output[700], scorestr[32];
    int plies;

    if (score >= 98.0 || score <= -98.0) {
        // mate: alphabeta scores it 99 + remaining depth, at the mated side
        plies = depth - (int)(fabs(score) - 99.0 + 0.5);
        sprintf(scorestr, "mate %d", (score > 0) ? (plies + 1) / 2 : -(plies / 2));
    } else {
        sprintf(scorestr, "cp %d", (int)lround(score * 100));
    }
    sprintf(output, "info depth %d score %s nodes %llu time %ld nps %llu pv %s", depth, scorestr,
            nodes, ms, (ms > 0) ? nodes * 1000 / ms : nodes, pv);
    uci_send(output);
}

void uci_send(char *message) {
    printf("%s\n", message);
    fflush(stdout);
}

void uci_send_id() {
    uci_send("id name GiuChess-2.0");
    uci_send("id author Giuliano Ippoliti");
}

void uci_send_options() {
    char output[100];

    sprintf(output, "option name Depth type spin default %d min 1 max 10", STARTDPTH);
    uci_send(output);
}

// "setoption name <id> [value <x>]"
void uci_parse_setoption(char *input) {
    char *name, *value;
    int depth;

    name = strstr(input, "name ");
    if (!name)
        return;
    name += 5;
    value = strstr(name, " value ");
    if (value) {
        *value = '\0';
        value += 7;
    }
    if (strcasecmp(name, "Depth") == 0 && value) {
        depth = atoi(value);
        if (depth >= 1 && depth <= 10)
            uci_default_depth = depth;
    }
}

void uci_set_position_startpos() {
    // Reset to starting position
    iniz(w, b);
    last_moved_color = 'B'; // White to move initially
}

// Returns 0 (and keeps the previous position) if the FEN cannot be loaded
int uci_set_position_fen(char *fen) {
    piece wbck[16], bbck[16];
    int rule50bck;
    char lastbck;

    memcpy(wbck, w, sizeof(w));
    memcpy(bbck, b, sizeof(b));
    rule50bck = rule50;
    lastbck = last_moved_color;
    if (load_fen(fen))
        return 1;
    memcpy(w, wbck, sizeof(w));
    memcpy(b, bbck, sizeof(b));
    rule50 = rule50bck;
    last_moved_color = lastbck;
    uci_send("info string invalid FEN, position unchanged");
    return 0;
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
        if (!uci_set_position_fen(input + 13))
            return;
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
    char bestmove[10], line[64], output[100];
    int color, depth, mytime, myinc, movestogo;
    long limit_ms;
    size_t len;
    
    // Determine who is to move
    color = (last_moved_color == 'W') ? 1 : -1; // 1 for black, -1 for white
    
    // Time for this move: movetime, or a share of the remaining clock
    mytime = (color == -1) ? uci_search_params.wtime : uci_search_params.btime;
    myinc = (color == -1) ? uci_search_params.winc : uci_search_params.binc;
    limit_ms = 0;
    if (uci_search_params.infinite) {
        limit_ms = 0;
    } else if (uci_search_params.movetime > 0) {
        limit_ms = uci_search_params.movetime;
    } else if (mytime > 0) {
        movestogo = (uci_search_params.movestogo > 0) ? uci_search_params.movestogo : 30;
        limit_ms = mytime / movestogo + myinc / 2;
        if (limit_ms > mytime / 2)
            limit_ms = mytime / 2;
        if (limit_ms < 1)
            limit_ms = 1;   // almost no time left: depth 1 only, not "no limit"
    }
    
    // Set search depth: as given, else unlimited with a time limit or
    // "infinite", else the "Depth" option
    depth = uci_search_params.depth;
    if (depth <= 0)
        depth = (limit_ms > 0 || uci_search_params.infinite) ? MAX_SEARCH_DEPTH : uci_default_depth;
    
    // Perform search; "stop" or "quit" end it early
    uci_stop_received = 0;
    bestmove[0] = '\0';
    search_poll = uci_poll;
    search(depth, color, bestmove, limit_ms, uci_report);
    search_poll = NULL;
    
    // "go infinite": the best move is only sent after "stop"
    while (uci_search_params.infinite && !uci_stop_received && !uci_quit_received) {
        while ((len = uci_first_line()) == 0 && !uci_eof)
            uci_fill_input(1);
        if (len == 0)
            break;
        uci_take_line(line, sizeof(line), len);
        if (strcmp(line, "stop") == 0)
            uci_stop_received = 1;
        else if (strcmp(line, "quit") == 0)
            uci_quit_received = 1;
        else if (strcmp(line, "isready") == 0)
            uci_send("readyok");
    }
    
    // Send best move ("0000": no legal move)
    if (bestmove[0] == '\0') {
        uci_send("bestmove 0000");
        return;
    }
    sprintf(output, "bestmove %s", bestmove);
    uci_send(output);
    
    // Execute the move internally
    string_execute_move(bestmove, w, b);
}

// UCI main loop; first, if given, is a command already read (protocol detection)
void uci_loop(const char *first) {
    static char input[sizeof(uci_inbuf)];
    
    uci_mode = 1;
    if (first != NULL && first[0] != '\0') {
        uci_inlen = snprintf(uci_inbuf, sizeof(uci_inbuf), "%s\n", first);
    }
    
    while (!uci_quit_received && uci_read_line(input, sizeof(input))) {
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
            // No search running: nothing to stop
        }
        else if (strncmp(input, "quit", 4) == 0) {
            break;
        }
        else if (strncmp(input, "setoption", 9) == 0) {
            uci_parse_setoption(input);
        }
    }
}
