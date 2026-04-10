#ifndef JOGO_H
#define JOGO_H

#include <stdbool.h>

// Estrutura para armazenar dados de um jogador
typedef struct {
    int fd;
    char nome[64];
    int pontos;
} Jogador;

// Funções de lógica do jogo
bool validar_palavra(const char *palavra, char letra_esperada);
void gerar_letra_aleatoria(char *letra);

// Funções de comunicação formatada
int enviar_mensagem(int fd, const char *prefixo, const char *conteudo);
int receber_mensagem(int fd, char *prefixo, char *conteudo);
int receber_com_timeout(int fd, char *prefixo, char *conteudo, int segundos);

#endif
