# Projet « Coloriage de graphes » - compilation
# Mêmes options pour toutes les versions (séquentielles et parallèles) :
# c'est indispensable pour comparer les temps honnêtement.

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -fopenmp
COMMUN  = graphe.o mesures.o

ifeq ($(OS),Windows_NT)
RM = del /Q /F
endif

PROGRAMMES = test_commun generer glouton_seq glouton_par backtrack_seq backtrack_par

all: $(PROGRAMMES)

test_commun: test_commun.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

generer: generer.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

backtrack_seq: backtrack_seq.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

glouton_seq: glouton_seq.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

glouton_par: glouton_par.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

backtrack_par: backtrack_par.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c graphe.h mesures.h
	$(CC) $(CFLAGS) -c $<

test: test_commun
	./test_commun

clean:
ifeq ($(OS),Windows_NT)
	cmd /C "del /Q *.o *.exe test_commun.exe generer.exe glouton_seq.exe glouton_par.exe backtrack_seq.exe backtrack_par.exe 2>NUL"
else
	$(RM) *.o $(PROGRAMMES) *.exe
endif

.PHONY: all test clean
