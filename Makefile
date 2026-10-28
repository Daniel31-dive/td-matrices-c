CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
LIB    = matrice.c matrice_alloc.c matrice_io.c matrice_calcul.c

all: demo_matrices test_ex1 test_ex2 test_ex3

demo_matrices: main.c $(LIB)
	$(CC) $(CFLAGS) main.c $(LIB) -o $@

test_ex1: test_ex1.c matrice.c matrice_alloc.c
	$(CC) $(CFLAGS) -DMAT_TEST_ALLOC $^ -o $@
test_ex2: test_ex2.c matrice.c matrice_alloc.c matrice_io.c
	$(CC) $(CFLAGS) -DMAT_TEST_ALLOC $^ -o $@
test_ex3: test_ex3.c matrice.c matrice_alloc.c matrice_calcul.c
	$(CC) $(CFLAGS) -DMAT_TEST_ALLOC $^ -o $@

tests: test_ex1 test_ex2 test_ex3
	./test_ex1 && ./test_ex2 && ./test_ex3

clean:
	rm -f demo_matrices test_ex1 test_ex2 test_ex3 *.o test_ex2_sortie.txt R*.txt
