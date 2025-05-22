#include "keyboard.h"
#include "screen.h"
#include "timer.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define CHAR_NAVIO '^'
#define CHAR_INVASOR 'X'
#define CHAR_TIRO '|'
#define RANKING_FILE "ranking.txt"
#define VIDAS_INICIAIS 3

#define MAX_TIROS 50


#define RESET "\x1b[0m"
#define VERDE "\x1b[32m"
#define VERMELHO "\x1b[31m"
#define AMARELO "\x1b[33m"
#define AZUL "\x1b[34m"

const char *corNave = VERDE;
const char *corInvasor = VERMELHO;
const char *corTiro = AMARELO;

int velocidade = 100;  

typedef struct {
    int x, y;
} Nave;

typedef struct Tiro {
    int x, y;
    struct Tiro *prox;
} Tiro;

typedef struct Invasor {
    int x, y;
    struct Invasor *prox;
} Invasor;

Nave jogador;
Tiro *tiros = NULL;
Invasor *invasores = NULL;

int pontos = 0;
int vidas = VIDAS_INICIAIS;
bool jogoRodando = false;
bool jogoPerdido = false;

char nomeJogador[50];

void adicionarTiro(int x, int y) {
    
    int numTiros = 0;
    Tiro *t = tiros;
    while (t) {
        numTiros++;
        t = t->prox;
    }
    
    
    if (numTiros < MAX_TIROS) {
        Tiro *novo = malloc(sizeof(Tiro));
        if (novo) {
            novo->x = x;
            novo->y = y;
            novo->prox = tiros;
            tiros = novo;
        }
    }
}

void removerTiro(Tiro **anterior, Tiro **atual) {
    Tiro *remover = *atual;
    *atual = remover->prox;
    free(remover);
    if (*anterior) {
        (*anterior)->prox = *atual;
    } else {
        tiros = *atual;
    }
}

void adicionarInvasor(int x, int y) {
    Invasor *novo = malloc(sizeof(Invasor));
    novo->x = x;
    novo->y = y;
    novo->prox = invasores;
    invasores = novo;
}

void inicializarJogo() {
    jogador.x = MAXX / 2;
    jogador.y = MAXY - 2;
    tiros = NULL;
    invasores = NULL;
    pontos = 0;
    vidas = VIDAS_INICIAIS;
    jogoRodando = true;
    jogoPerdido = false;

    for (int i = 10; i < MAXX - 10; i += 5) {
        for (int j = 2; j < 6; j += 2) {
            adicionarInvasor(i, j);
        }
    }
}

void moverTiros() {
    Tiro *anterior = NULL;
    Tiro *atual = tiros;
    while (atual) {
        atual->y--;
        if (atual->y < 1) {
            removerTiro(&anterior, &atual);
        } else {
            anterior = atual;
            atual = atual->prox;
        }
    }
}

void moverInvasores() {
    Invasor *atual = invasores;
    bool reiniciar = false;
    
    while (atual) {
        atual->y++;
        if (atual->y >= jogador.y) {
            reiniciar = true;
            break;
        }
        atual = atual->prox;
    }
    
    if (reiniciar) {
        vidas--;
        if (vidas <= 0) {
            jogoPerdido = true;
            jogoRodando = false;
        }
        
        
        while (invasores) {
            Invasor *tmp = invasores;
            invasores = invasores->prox;
            free(tmp);
        }
        
        
        if (vidas > 0) {
            for (int i = 10; i < MAXX - 10; i += 5) {
                for (int j = 2; j < 6; j += 2) {
                    adicionarInvasor(i, j);
                }
            }
        }
    }
}

void verificarColisoes() {
    Tiro *t = tiros;
    Tiro *anteriorT = NULL;
    
    while (t) {
        bool colisao = false;
        Invasor *i = invasores;
        Invasor *anteriorI = NULL;
        
        while (i) {
            if (t->x == i->x && t->y == i->y) {
                
                if (anteriorI) anteriorI->prox = i->prox;
                else invasores = i->prox;
                free(i);
                
                
                Tiro *proximoT = t->prox;
                removerTiro(&anteriorT, &t);
                t = proximoT;
                
                pontos++;
                colisao = true;
                break;
            }
            anteriorI = i;
            i = i->prox;
        }
        
        if (!colisao) {
            anteriorT = t;
            t = t->prox;
        }
        
        
        if (invasores == NULL) {
            jogoRodando = false;
            screenInit(0);
            printf("PARABÉNS! Você venceu!\n");
            printf("Pontuação final: %d\n", pontos);
            printf("Pressione qualquer tecla para continuar...");
            while (!keyhit()) {}
            readch();
            break;
        }
    }
}

void desenharJogo() {
    screenInit(1);
    screenGotoxy(2, MAXY);
    printf("Pontos: %d | Vidas: %d", pontos, vidas);

    screenGotoxy(jogador.x, jogador.y);
    printf("%s%c%s", corNave, CHAR_NAVIO, RESET);

    Tiro *t = tiros;
    while (t) {
        screenGotoxy(t->x, t->y);
        printf("%s%c%s", corTiro, CHAR_TIRO, RESET);
        t = t->prox;
    }

    Invasor *i = invasores;
    while (i) {
        screenGotoxy(i->x, i->y);
        printf("%s%c%s", corInvasor, CHAR_INVASOR, RESET);
        i = i->prox;
    }

    screenUpdate();
}

void loopJogo() {
    timerInit(velocidade);
    while (jogoRodando) {
        if (keyhit()) {
            int tecla = readch();
            if (tecla == 'a' && jogador.x > MINX + 1) jogador.x--;
            if (tecla == 'd' && jogador.x < MAXX - 1) jogador.x++;
            if (tecla == ' ') adicionarTiro(jogador.x, jogador.y - 1);
            if (tecla == 27) jogoRodando = false;
        }

        if (timerTimeOver()) {
            moverTiros();
            moverInvasores();
            verificarColisoes();
            desenharJogo();
        }
        
        
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10000000; 
        nanosleep(&ts, NULL);
    }
}

void destruirListas() {
    while (tiros) {
        Tiro *tmp = tiros;
        tiros = tiros->prox;
        free(tmp);
    }
    while (invasores) {
        Invasor *tmp = invasores;
        invasores = invasores->prox;
        free(tmp);
    }
}

void gravarPontuacao() {
    FILE *fp = fopen(RANKING_FILE, "a");
    if (fp) {
        fprintf(fp, "%s %d\n", nomeJogador, pontos);
        fclose(fp);
    } else {
        screenInit(0);
        printf("Erro ao abrir arquivo de ranking!\n");
        printf("Pressione qualquer tecla para continuar...");
        while (!keyhit()) {}
        readch();
    }
}

void mostrarRanking() {
    screenInit(0);
    FILE *fp = fopen(RANKING_FILE, "r");
    if (fp) {
        char linha[100];
        printf("\nRanking:\n\n");
        while (fgets(linha, sizeof(linha), fp)) {
            printf("%s", linha);
        }
        fclose(fp);
    } else {
        printf("Erro ao abrir ranking.\n");
    }
    printf("\nPressione qualquer tecla para voltar ao menu...\n");
    while (!keyhit()) {}
    readch();
}

void configuracoes() {
    char op;
    do {
        screenInit(0);
        printf("CONFIGURAÇÕES\n\n");
        printf("1 - Cor da Nave\n2 - Cor dos Invasores\n3 - Cor do Tiro\n4 - Dificuldade\n5 - Voltar\n");
        op = readch();
        
        switch (op) {
            case '1':
                screenInit(0);
                printf("Cor da nave (1-Verde, 2-Azul): ");
                op = readch();
                if (op == '1') {
                    corNave = VERDE;
                    printf("\nCor da nave alterada para Verde!");
                } else if (op == '2') {
                    corNave = AZUL;
                    printf("\nCor da nave alterada para Azul!");
                } else {
                    printf("\nOpção inválida!");
                }
                printf("\nPressione qualquer tecla para continuar...");
                while (!keyhit()) {}
                readch();
                break;
                
            case '2':
                screenInit(0);
                printf("Cor dos invasores (1-Vermelho, 2-Azul): ");
                op = readch();
                if (op == '1') {
                    corInvasor = VERMELHO;
                    printf("\nCor dos invasores alterada para Vermelho!");
                } else if (op == '2') {
                    corInvasor = AZUL;
                    printf("\nCor dos invasores alterada para Azul!");
                } else {
                    printf("\nOpção inválida!");
                }
                printf("\nPressione qualquer tecla para continuar...");
                while (!keyhit()) {}
                readch();
                break;
                
            case '3':
                screenInit(0);
                printf("Cor dos tiros (1-Amarelo, 2-Verde): ");
                op = readch();
                if (op == '1') {
                    corTiro = AMARELO;
                    printf("\nCor dos tiros alterada para Amarelo!");
                } else if (op == '2') {
                    corTiro = VERDE;
                    printf("\nCor dos tiros alterada para Verde!");
                } else {
                    printf("\nOpção inválida!");
                }
                printf("\nPressione qualquer tecla para continuar...");
                while (!keyhit()) {}
                readch();
                break;
                
            case '4':
                screenInit(0);
                printf("Dificuldade (1-Fácil, 2-Médio, 3-Difícil): ");
                op = readch();
                if (op == '1') {
                    velocidade = 150;
                    printf("\nDificuldade alterada para Fácil!");
                } else if (op == '2') {
                    velocidade = 100;
                    printf("\nDificuldade alterada para Médio!");
                } else if (op == '3') {
                    velocidade = 50;
                    printf("\nDificuldade alterada para Difícil!");
                } else {
                    printf("\nOpção inválida!");
                }
                printf("\nPressione qualquer tecla para continuar...");
                while (!keyhit()) {}
                readch();
                break;
                
            case '5':
                return;
                
            default:
                screenInit(0);
                printf("Opção inválida!\n");
                printf("Pressione qualquer tecla para continuar...");
                while (!keyhit()) {}
                readch();
        }
    } while (1);
}

void menu() {
    screenInit(0);
    printf("SPACE INVADERS\n\n");
    printf("1 - Jogar\n");
    printf("2 - Ver Ranking\n");
    printf("3 - Configurações\n");
    printf("4 - Sair\n");
}

int main() {
    keyboardInit();
    screenInit(0);

    printf("Digite seu nome (máx. 49 caracteres): ");
    fgets(nomeJogador, 50, stdin);
    nomeJogador[strcspn(nomeJogador, "\n")] = 0;

    while (1) {
        menu();
        char opcao = readch();
        if (opcao == '1') {
            inicializarJogo();
            loopJogo();
            gravarPontuacao();
        } else if (opcao == '2') {
            mostrarRanking();
        } else if (opcao == '3') {
            configuracoes();
        } else if (opcao == '4') {
            break;
        }
    }

    destruirListas();
    keyboardDestroy();
    screenDestroy();
    timerDestroy();
    return 0;
}
