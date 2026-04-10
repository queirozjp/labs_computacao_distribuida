#include "jogo.h"
#include "protocolo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>

bool validar_palavra(const char *palavra, char letra_esperada) {
    int len = strlen(palavra);
    if (len < MIN_CARACTERES) return false;
    
    // Converte a primeira letra para minúsculo para comparar
    if (tolower((unsigned char)palavra[0]) != tolower((unsigned char)letra_esperada)) return false;
    
    for (int i = 0; i < len; i++) {
        // isalpha() em C padrão (ASCII) não aceita acentos (como 'á', 'ã', etc.)
        // Para manter simples e funcional, aceitamos apenas a-z e A-Z
        if (!isalpha((unsigned char)palavra[i])) return false;
    }
    return true;
}

void gerar_letra_aleatoria(char *letra) {
    static bool seeded = false;
    if (!seeded) {
        srand(time(NULL));
        seeded = true;
    }
    *letra = 'A' + (rand() % 26);
}

int enviar_mensagem(int fd, const char *prefixo, const char *conteudo) {
    char buffer[BUFFER_SIZE];
    if (conteudo && strlen(conteudo) > 0) {
        snprintf(buffer, sizeof(buffer), "%s|%s\n", prefixo, conteudo);
    } else {
        snprintf(buffer, sizeof(buffer), "%s|\n", prefixo);
    }
    return write(fd, buffer, strlen(buffer));
}

int receber_mensagem(int fd, char *prefixo, char *conteudo) {
    char buffer[BUFFER_SIZE];
    int n = 0;
    char c;
    
    while (n < BUFFER_SIZE - 1) {
        int r = read(fd, &c, 1);
        if (r <= 0) return r; 
        if (c == '\n') break;
        buffer[n++] = c;
    }
    buffer[n] = '\0';

    char *pipe = strchr(buffer, '|');
    if (pipe) {
        *pipe = '\0';
        strcpy(prefixo, buffer);
        strcpy(conteudo, pipe + 1);
    } else {
        strcpy(prefixo, buffer);
        conteudo[0] = '\0';
    }
    return n;
}

int receber_com_timeout(int fd, char *prefixo, char *conteudo, int segundos) {
    fd_set set;
    struct timeval timeout;
    FD_ZERO(&set);
    FD_SET(fd, &set);
    timeout.tv_sec = segundos;
    timeout.tv_usec = 0;

    int rv = select(fd + 1, &set, NULL, NULL, &timeout);
    if (rv == -1) return -1; 
    if (rv == 0) return 0;  
    return receber_mensagem(fd, prefixo, conteudo);
}
