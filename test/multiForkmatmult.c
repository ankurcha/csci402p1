#include "syscall.h"
#include "print.c"
#define Dim 20

void matmult() {
    int A[Dim][Dim];
    int B[Dim][Dim];
    int C[Dim][Dim];

    int i, j, k;

    for (i = 0; i < Dim; i++) /* first initialize the matrices */
        for (j = 0; j < Dim; j++) {
            A[i][j] = i;
            B[i][j] = j;
            C[i][j] = 0;
        }

    for (i = 0; i < Dim; i++) /* then multiply them together */
        for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
                C[i][j] += A[i][k] * B[k][j];

    Exit(C[Dim - 1][Dim - 1]); /* and then we're done */
}

int main() {

    print("Forking matmult\n");
    Fork(matmult);
    Fork(matmult);
    print("Exiting multiForkmatmult main()\n");
    Exit(0);
}
