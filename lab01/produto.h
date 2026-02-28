#ifndef PRODUTO_H
#define PRODUTO_H

typedef struct{
    int codigo;
    char *nome;
    float preco;
    int quantidade;
} Produto;

void adicionar_produto(Produto** lista, int *tamanho, int *codigo);
void listar_produtos(Produto* lista, int *tamanho);
Produto* buscar_produto(Produto* lista, int *tamanho);
void atualizar_estoque(Produto* lista, int *tamanho);
void remover_produto(Produto** lista, int *tamanho);
void liberar_memoria(Produto* lista, int *tamanho);

#endif
