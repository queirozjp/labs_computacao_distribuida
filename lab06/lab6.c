#include <stdio.h>
#include <math.h>
#include <mpi.h>

#define DATA_SIZE 100
#define NUM_PROCESSES 5
#define CHUNK_SIZE 20

int main(int argc, char *argv[]) {
    int vetor[DATA_SIZE];
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        printf("[Processo %d] Vetor original:", rank);
        for (int i = 0; i < DATA_SIZE; i++) {
            vetor[i] = i + 1;
            printf(" %d", vetor[i]);
        }
        printf("\n");
    }

    int vetor_local[CHUNK_SIZE];

    MPI_Scatter(vetor, CHUNK_SIZE, MPI_INT, vetor_local, CHUNK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    for (int i = 0; i < CHUNK_SIZE; i++) {
        vetor_local[i] = pow(vetor_local[i], 2);
    }

    MPI_Gather(vetor_local, CHUNK_SIZE, MPI_INT, vetor, CHUNK_SIZE, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("[Processo %d] Vetor final:", rank);
        for (int i = 0; i < DATA_SIZE; i++) {
            printf(" %d", vetor[i]);
        }
        printf("\n");
    }
    MPI_Finalize();
    return 0;
}
