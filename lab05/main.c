#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>


int main(int argc, char **argv) {
    float soma_final = 0;
    float media_final = 0;
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 2) {
        if (rank == 0) {
            printf("O tamanho do vetor não foi informado");
        }
        MPI_Finalize();
        return 1;
    }
    srand(time(NULL) + rank);
    int n = atoi(argv[1]);
    float *vetor = (float *) malloc(n * sizeof(float));

    float soma_local = 0;
    float media_local = 0;

    for (int i = 0; i < n; i++) {
        vetor[i] = rand() / (float)RAND_MAX;
        soma_local += vetor[i];
    }

    media_local = soma_local / n;

    printf("[Processo %d] Soma local: %f Média local: %f\n", rank, soma_local, media_local);

    MPI_Reduce(&soma_local, &soma_final, 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        media_final = soma_final / (n * size);
        printf("Soma final: %f\n", soma_final);
        printf("Média final: %f\n", media_final);
    }

    free(vetor);
    MPI_Finalize();
    return 0;
}
