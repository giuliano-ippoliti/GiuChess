PROG = GiuChess-2.0
PERFT = perft-test
CSOURCES = check.c exec_mov.c main.c util_functs.c iniz.c legal.c pieces_legmoves.c pieces_ex_legmoves.c wblegmov.c rw_thread.c eval_pos.c list.c uci.c fen.c
HEADERS = check.h exec_mov.h main.h util_functs.h iniz.h legal.h pieces_legmoves.h pieces_ex_legmoves.h wblegmov.h rw_thread.h eval_pos.h list.h uci.h fen.h
COMPIL_FLAGS = -Wall -g -O2
# x86-64 only: keep the binary runnable on CPUs without BMI2
ifeq ($(shell uname -m),x86_64)
COMPIL_FLAGS += -mno-bmi2
endif
LIB = -lm -lpthread
CC = gcc
OBJECTS = $(CSOURCES:.c=.o)
PERFT_OBJECTS = $(filter-out main.o,$(OBJECTS)) perft.o

.PHONY:	all perft clean cleanpgn nett

all:	$(PROG)

$(PROG):	$(OBJECTS)
	$(CC) $(COMPIL_FLAGS) $(LIB) -o $(PROG) $(OBJECTS)
	
%.o:	%.c $(HEADERS)
	$(CC) $(COMPIL_FLAGS) -c $<

# Perft check: builds and runs the move generation test (exit status 1 on mismatch)
perft:	$(PERFT)
	./$(PERFT)

$(PERFT):	$(PERFT_OBJECTS)
	$(CC) $(COMPIL_FLAGS) $(LIB) -o $(PERFT) $(PERFT_OBJECTS)
	
clean:
	rm -f *.o $(PROG) $(PERFT)
	rm -f scripts/*.log
	rm -f scripts/*.debug

cleanpgn:
	rm -f scripts/*.pgn

nett:
	rm -f *~
