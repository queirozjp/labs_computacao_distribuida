#include <stdio.h>
#include <stdlib.h>
#include "produto.h"

int menu() {
    int opcao;

    printf("\n=================================================\n");
    printf("      SISTEMA DE CADASTRO DE PRODUTOS\n");
    printf("=================================================\n");

    printf("\nMenu:\n");
    printf("1. Adicionar produto\n");
    printf("2. Listar produtos\n");
    printf("3. Buscar produto\n");
    printf("4. Atualizar estoque\n");
    printf("5. Remover produto\n");
    printf("6. Sair\n");
    printf("Escolha uma opcao: ");

    scanf("%d", &opcao);

    return opcao;
}

int main() {
    Produto* lista = NULL;
    int tamanho = 0;
    int codigo = 0;
    int opcao;

    do {
        opcao = menu();

        switch (opcao) {
            case 1:
                adicionar_produto(&lista, &tamanho, &codigo);
                break;

            case 2:
                if (tamanho == 0)
                    printf("\nNenhum produto cadastrado.\n");
                else
                    listar_produtos(lista, &tamanho);
                break;

            case 3: {
                Produto* p = buscar_produto(lista, &tamanho);
                if (p != NULL) {
                    printf("\nProduto encontrado:\n");
                    printf("Codigo: %d\n", p->codigo);
                    printf("Nome: %s\n", p->nome);
                    printf("Preco: %.2f\n", p->preco);
                    printf("Quantidade: %d\n", p->quantidade);
                }
                break;
            }

            case 4:
                atualizar_estoque(lista, &tamanho);
                break;

            case 5:
                remover_produto(&lista, &tamanho);
                break;

            case 6:
                printf("\nLiberando memoria...\n");
                liberar_memoria(lista, &tamanho);
                printf("Programa encerrado com sucesso!\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
        }

    } while (opcao != 6);

    return 0;
}
