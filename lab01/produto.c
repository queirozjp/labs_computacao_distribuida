#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "produto.h"

void adicionar_produto(Produto** lista, int *tamanho, int *codigo){
    (*tamanho)++;
    (*codigo)++;

    Produto *temp = realloc(*lista, *tamanho * sizeof(Produto));
    if (temp == NULL){
        printf("Erro de alocação\n");
        (*tamanho)--;
        return;
    }
    int pos = *tamanho - 1;
    temp[pos].codigo = *codigo;
    char buffer[65];

    printf("\n\nNome do Produto: ");
    scanf(" %64[^\n]", buffer);
    temp[pos].nome = malloc(strlen(buffer) + 1);
    strcpy(temp[(*tamanho-1)].nome, buffer);

    printf("Preco do Produto: ");
    scanf("%f",&temp[pos].preco);

    printf("Quantidade do Produto: ");
    scanf("%d",&temp[pos].quantidade);
    
    printf("\n%s de codigo {%d} adicionado com sucesso!\n", buffer, *codigo);

    *lista = temp;
}

void listar_produtos(Produto* lista, int *tamanho){
    float total = 0;
    int i;
    printf("+--------+-----------------+------------+------------+\n");
    printf("| %-6s | %-15s | %-10s | %-10s |\n", "COD", "NOME", "PRECO", "QTD");
    printf("+--------+-----------------+------------+------------+\n");

    for (i = 0; i < *tamanho; i++){
        printf("| %-6d | %-15s | %10.2f | %-10d |\n",
           lista[i].codigo,
           lista[i].nome,
           lista[i].preco,
           lista[i].quantidade);
        total = total + lista[i].preco*lista[i].quantidade;
    }
    printf("+--------+-----------------+------------+------------+\n");
    printf("\nValor do estoque = %.2f\n", total);
}

Produto* buscar_produto(Produto* lista, int *tamanho){
    int cod = 0, i;
    printf("\nDigite o codigo: ");
    scanf("%d", &cod);
    for (i = 0; i < *tamanho; i++){
        if (lista[i].codigo == cod) return &lista[i];
    }
    printf("\nNão foi possivel achar o produto!");
    return NULL;
}

void atualizar_estoque(Produto* lista, int *tamanho){
    Produto* produto =  buscar_produto(lista, tamanho);
    if (produto == NULL) return;
    else{
        printf("\nProduto: %s\n Quantidade atual: %d", produto->nome, produto->quantidade);
        printf("\nDigite a nova quantidade: ");
        scanf("%d", &produto->quantidade);
        printf("\nQuantidade atualizada com sucesso!");
    }
}

void remover_produto(Produto** lista, int *tamanho){
    int i, j, cod = 0; 
    printf("Digite o codigo: ");
    scanf("%d", &cod);
    for (i = 0; i < *tamanho; i++){
        if ((*lista)[i].codigo == cod){
            free((*lista)[i].nome);
            (*tamanho)--;
            for (j = 0; j < *tamanho; j++){
                (*lista)[j] = (*lista)[j+1];
            }
            if (*tamanho == 0) {
                free(*lista);
                *lista = NULL;
            }
            else{
                Produto* temp = realloc(*lista, *tamanho * sizeof(Produto));
                if(temp != NULL) *lista = temp;
            }
            printf("\nProduto removido com sucesso!");
            return;
        }
    }
    printf("Não foi possivel achar o produto");
    return;
}

void liberar_memoria(Produto* lista, int *tamanho){
    int i;
    for(i = 0; i < *tamanho; i++){
        free(lista[i].nome);
    } 
    free(lista);
}
