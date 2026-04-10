#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "protocolo.h"
#include "jogo.h"

int main(int argc, char *argv[]) {
    char *ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int porta = (argc > 2) ? atoi(argv[2]) : PORTA_PADRAO;

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(porta);
    addr.sin_addr.s_addr = inet_addr(ip);

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Erro ao conectar");
        return 1;
    }

    printf("Conectado ao servidor %s:%d\n", ip, porta);

    char prefixo[BUFFER_SIZE], conteudo[BUFFER_SIZE];
    while (receber_mensagem(client_fd, prefixo, conteudo) > 0) {
        if (strcmp(prefixo, NOME_PREFIX) == 0) {
            printf("Digite seu nome: ");
            char nome[64];
            if (scanf("%63s", nome) == 1) {
                enviar_mensagem(client_fd, NOME_PREFIX, nome);
            }
        } else if (strcmp(prefixo, MSG_PREFIX) == 0) {
            printf("\n[MENSAGEM] %s\n", conteudo);
        } else if (strcmp(prefixo, AGUARDE_PREFIX) == 0) {
            printf("%s\n", conteudo);
        } else if (strcmp(prefixo, RODADA_PREFIX) == 0) {
            int num, tempo;
            char letra;
            if (sscanf(conteudo, "%d|%c|%d", &num, &letra, &tempo) == 3) {
                printf("\n--- RODADA %d ---\n", num);
                printf("Letra: [%c] | Tempo: %d seg\n", letra, tempo);
                printf("Sua palavra: ");
                fflush(stdout);

                fd_set set;
                struct timeval tv = {tempo, 0};
                FD_ZERO(&set);
                FD_SET(STDIN_FILENO, &set);
                
                if (select(STDIN_FILENO + 1, &set, NULL, NULL, &tv) > 0) {
                    char palavra[64];
                    if (scanf("%63s", palavra) == 1) {
                        enviar_mensagem(client_fd, PALAVRA_PREFIX, palavra);
                    }
                } else {
                    printf("\nTempo esgotado!\n");
                    enviar_mensagem(client_fd, TIMEOUT_PREFIX, "");
                }
            }
        } else if (strcmp(prefixo, RESULTADO_PREFIX) == 0) {
            printf("Resultado: %s\n", conteudo);
        } else if (strcmp(prefixo, PLACAR_PREFIX) == 0) {
            // Usando strtok para processar o placar com segurança
            char *p1_str = strtok(conteudo, "|");
            char *p1_pts = strtok(NULL, "|");
            char *p2_str = strtok(NULL, "|");
            char *p2_pts = strtok(NULL, "|");
            
            if (p1_str && p1_pts && p2_str && p2_pts) {
                printf("PLACAR: %s %s x %s %s\n", p1_str, p1_pts, p2_pts, p2_str);
            }
        } else if (strcmp(prefixo, FIM_PREFIX) == 0) {
            printf("\n--- FIM DE JOGO ---\n%s\n", conteudo);
            break;
        }
    }

    close(client_fd);
    return 0;
}
