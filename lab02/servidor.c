#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include "protocolo.h"
#include "jogo.h"

typedef struct {
    Jogador j1;
    Jogador j2;
} Partida;

void *gerenciar_partida(void *arg) {
    Partida *p = (Partida *)arg;
    char prefixo[BUFFER_SIZE];
    char msg[BUFFER_SIZE];

    // Solicitar nomes
    enviar_mensagem(p->j1.fd, NOME_PREFIX, "");
    receber_mensagem(p->j1.fd, prefixo, p->j1.nome);
    enviar_mensagem(p->j2.fd, NOME_PREFIX, "");
    receber_mensagem(p->j2.fd, prefixo, p->j2.nome);

    snprintf(msg, sizeof(msg), "%s vs %s", p->j1.nome, p->j2.nome);
    enviar_mensagem(p->j1.fd, MSG_PREFIX, msg);
    enviar_mensagem(p->j2.fd, MSG_PREFIX, msg);

    for (int r = 1; r <= MAX_RODADAS; r++) {
        char letra;
        gerar_letra_aleatoria(&letra);
        snprintf(msg, sizeof(msg), "%d|%c|%d", r, letra, TEMPO_RODADA);
        enviar_mensagem(p->j1.fd, RODADA_PREFIX, msg);
        enviar_mensagem(p->j2.fd, RODADA_PREFIX, msg);

        char p1_palavra[64] = "", p2_palavra[64] = "";
        bool p1_recebeu = false, p2_recebeu = false;
        
        time_t start_time = time(NULL);
        while (time(NULL) - start_time < TEMPO_RODADA) {
            if (p1_recebeu && p2_recebeu) break;

            fd_set read_fds;
            FD_ZERO(&read_fds);
            int max_fd = 0;
            if (!p1_recebeu) { FD_SET(p->j1.fd, &read_fds); if(p->j1.fd > max_fd) max_fd = p->j1.fd; }
            if (!p2_recebeu) { FD_SET(p->j2.fd, &read_fds); if(p->j2.fd > max_fd) max_fd = p->j2.fd; }

            struct timeval tv = {1, 0}; 
            int activity = select(max_fd + 1, &read_fds, NULL, NULL, &tv);

            if (activity > 0) {
                if (!p1_recebeu && FD_ISSET(p->j1.fd, &read_fds)) {
                    if (receber_mensagem(p->j1.fd, prefixo, p1_palavra) > 0) {
                        if (strcmp(prefixo, PALAVRA_PREFIX) == 0) p1_recebeu = true;
                    }
                }
                if (!p2_recebeu && FD_ISSET(p->j2.fd, &read_fds)) {
                    if (receber_mensagem(p->j2.fd, prefixo, p2_palavra) > 0) {
                        if (strcmp(prefixo, PALAVRA_PREFIX) == 0) p2_recebeu = true;
                    }
                }
            }
        }

        bool p1_ok = p1_recebeu ? validar_palavra(p1_palavra, letra) : false;
        bool p2_ok = p2_recebeu ? validar_palavra(p2_palavra, letra) : false;

        if (p1_ok && p2_ok && strcasecmp(p1_palavra, p2_palavra) == 0) {
            enviar_mensagem(p->j1.fd, RESULTADO_PREFIX, "Empate na palavra! 0 pontos.");
            enviar_mensagem(p->j2.fd, RESULTADO_PREFIX, "Empate na palavra! 0 pontos.");
        } else {
            if (p1_ok) p->j1.pontos++;
            if (p2_ok) p->j2.pontos++;
            
            snprintf(msg, sizeof(msg), "%s (Oponente: %s)", p1_ok ? "+1 ponto!" : "Inválida/Timeout", p2_recebeu ? p2_palavra : "---");
            enviar_mensagem(p->j1.fd, RESULTADO_PREFIX, msg);
            
            snprintf(msg, sizeof(msg), "%s (Oponente: %s)", p2_ok ? "+1 ponto!" : "Inválida/Timeout", p1_recebeu ? p1_palavra : "---");
            enviar_mensagem(p->j2.fd, RESULTADO_PREFIX, msg);
        }

        snprintf(msg, sizeof(msg), "%s|%d|%s|%d", p->j1.nome, p->j1.pontos, p->j2.nome, p->j2.pontos);
        enviar_mensagem(p->j1.fd, PLACAR_PREFIX, msg);
        enviar_mensagem(p->j2.fd, PLACAR_PREFIX, msg);
    }

    if (p->j1.pontos > p->j2.pontos) {
        enviar_mensagem(p->j1.fd, FIM_PREFIX, "Você venceu!");
        enviar_mensagem(p->j2.fd, FIM_PREFIX, "Você perdeu!");
    } else if (p->j2.pontos > p->j1.pontos) {
        enviar_mensagem(p->j1.fd, FIM_PREFIX, "Você perdeu!");
        enviar_mensagem(p->j2.fd, FIM_PREFIX, "Você venceu!");
    } else {
        enviar_mensagem(p->j1.fd, FIM_PREFIX, "Empate!");
        enviar_mensagem(p->j2.fd, FIM_PREFIX, "Empate!");
    }

    close(p->j1.fd);
    close(p->j2.fd);
    free(p);
    return NULL;
}

int main(int argc, char *argv[]) {
    int porta = (argc > 1) ? atoi(argv[1]) : PORTA_PADRAO;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(porta);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Erro no bind");
        return 1;
    }
    listen(server_fd, 5);

    printf("Servidor rodando na porta %d...\n", porta);

    while (1) {
        Partida *p = malloc(sizeof(Partida));
        p->j1.fd = accept(server_fd, NULL, NULL);
        printf("Jogador 1 conectado. Aguardando J2...\n");
        enviar_mensagem(p->j1.fd, AGUARDE_PREFIX, "Aguardando outro jogador...");
        
        p->j2.fd = accept(server_fd, NULL, NULL);
        printf("Jogador 2 conectado. Iniciando partida.\n");
        
        p->j1.pontos = 0;
        p->j2.pontos = 0;

        pthread_t tid;
        pthread_create(&tid, NULL, gerenciar_partida, p);
        pthread_detach(tid);
    }
    return 0;
}
