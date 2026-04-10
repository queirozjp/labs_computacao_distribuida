/*
    Henrique Brainer Costa 10420717 
    João Pedro Queiroz de Andrade 10425822 
    João Victor Vidal Barbosa 10410165 
*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define N 40

int main(int argc, char **argv)
{
    int rank, size;
    int *vetor = NULL;
    int *sub_vetor = NULL;
    int soma_parcial = 0;
    int soma_total = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int slice = N / size;

    if (N % size != 0)
    {
        if (rank == 0)
            printf("Erro: N deve ser divisível pelo número de processos\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0)
    {
        vetor = (int *)malloc(N * sizeof(int));
        for (int i = 0; i < N; i++)
        {
            vetor[i] = i + 1;
        }
    }

    sub_vetor = (int *)malloc(slice * sizeof(int));
    MPI_Scatter(vetor, slice, MPI_INT, sub_vetor, slice, MPI_INT, 0, MPI_COMM_WORLD);

    printf("Processo %d recebeu: ", rank);
    for (int i = 0; i < slice; i++)
    {
        printf("%d ", sub_vetor[i]);
    }
    printf("\n");

    for (int i = 0; i < slice; i++)
    {
        soma_parcial += sub_vetor[i] * sub_vetor[i];
    }

    printf("Processo %d: soma local = %d\n", rank, soma_parcial);

    MPI_Reduce(&soma_parcial, &soma_total, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        int soma_seq = 0;
        for (int i = 1; i <= N; i++)
        {
            soma_seq += i * i;
        }

        printf("\n[RESULTADO] Processo %d:\n", rank);
        printf("Soma paralela = %d\n", soma_total);
        printf("Soma sequencial = %d\n", soma_seq);

        if (soma_total == soma_seq)
            printf("Resultado correto!\n");
        else
            printf("Resultado incorreto!\n");
    }

    free(sub_vetor);
    if (rank == 0)
        free(vetor);

    MPI_Finalize();
    return 0;
}
