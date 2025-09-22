#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Estrutura que representa um territorio no jogo
typedef struct {
    char nome[30];
    char cor[15];
    int jogador_id; // 0 = neutro, 1 = jogador, 2+ = outros jogadores
    int tropas;
    int* territorios_vizinhos; // Lista de índices dos territórios vizinhos
    int num_vizinhos;
} Territorio;

// Estrutura para representar uma missão
typedef struct {
    char descricao[100];
    int tipo;
    int jogador_alvo; // Para missões de eliminar jogador específico
} Missao;

// Vetor de missoes
Missao missoes[] = {
    {"Conquistar 3 territorios consecutivos", 1, -1},
    {"Eliminar todas as tropas de um jogador especifico", 2, -1},
    {"Controlar pelo menos 2 territorios", 3, -1},
    {"Ter mais de 13 tropas somadas", 4, -1},
    {"Conquistar um territorio na primeira tentativa", 5, -1}
};

// Cores disponíveis
char coresDisponiveis[][15] = {"Vermelho", "Azul", "Verde", "Amarelo", "Roxo", "Laranja"};
int totalCores = 6;

// Função para inicializar o mapa com adjacências
void inicializarMapa(Territorio* mapa, int total) {
    // Libera vizinhos anteriores se existirem
    for (int i = 0; i < total; i++) {
        if (mapa[i].territorios_vizinhos != NULL) {
            free(mapa[i].territorios_vizinhos);
        }
    }
    
    // Cria um layout circular simples onde cada território é vizinho do próximo e anterior
    for (int i = 0; i < total; i++) {
        mapa[i].num_vizinhos = 0;
        mapa[i].territorios_vizinhos = (int*)malloc(2 * sizeof(int)); // Máximo 2 vizinhos
        
        if (total > 1) {
            // Território anterior (circular)
            int anterior = (i - 1 + total) % total;
            mapa[i].territorios_vizinhos[mapa[i].num_vizinhos++] = anterior;
            
            // Próximo território (circular)
            int proximo = (i + 1) % total;
            mapa[i].territorios_vizinhos[mapa[i].num_vizinhos++] = proximo;
        }
    }
    
    // Para mapas com mais territórios, adiciona conexões extras
    if (total >= 4) {
        // Conecta território 0 com território 2
        mapa[0].territorios_vizinhos = (int*)realloc(mapa[0].territorios_vizinhos, 3 * sizeof(int));
        mapa[2].territorios_vizinhos = (int*)realloc(mapa[2].territorios_vizinhos, 3 * sizeof(int));
        mapa[0].territorios_vizinhos[mapa[0].num_vizinhos++] = 2;
        mapa[2].territorios_vizinhos[mapa[2].num_vizinhos++] = 0;
    }
    
    if (total >= 5) {
        // Conecta território 1 com território 3
        mapa[1].territorios_vizinhos = (int*)realloc(mapa[1].territorios_vizinhos, 3 * sizeof(int));
        mapa[3].territorios_vizinhos = (int*)realloc(mapa[3].territorios_vizinhos, 3 * sizeof(int));
        mapa[1].territorios_vizinhos[mapa[1].num_vizinhos++] = 3;
        mapa[3].territorios_vizinhos[mapa[3].num_vizinhos++] = 1;
    }
}

// Função para verificar se três territórios são consecutivos
int saoConsecutivos(Territorio* mapa, int a, int b, int c) {
    // Verifica se b é vizinho de a
    int b_vizinho_de_a = 0;
    for (int i = 0; i < mapa[a].num_vizinhos; i++) {
        if (mapa[a].territorios_vizinhos[i] == b) {
            b_vizinho_de_a = 1;
            break;
        }
    }
    
    // Verifica se c é vizinho de b
    int c_vizinho_de_b = 0;
    for (int i = 0; i < mapa[b].num_vizinhos; i++) {
        if (mapa[b].territorios_vizinhos[i] == c) {
            c_vizinho_de_b = 1;
            break;
        }
    }
    
    return b_vizinho_de_a && c_vizinho_de_b;
}

// Funcao que sorteia uma missao
Missao atribuirMissao(int totalJogadores) {
    int sorteio = rand() % 5;
    Missao missao = missoes[sorteio];
    
    // Para missões de eliminar jogador, sorteia um jogador alvo (exceto o jogador 1)
    if (missao.tipo == 2 && totalJogadores > 2) {
        missao.jogador_alvo = 2 + rand() % (totalJogadores - 2);
    } else {
        missao.jogador_alvo = 2; // Inimigo padrão
    }
    
    return missao;
}

// Funcao para verificar se a missao foi cumprida
int verificarMissao(Missao missao, Territorio* mapa, int totalTerritorios, int jogador_id, int turnos, int conquistaPrimeiroTurno) {
    switch(missao.tipo) {
        case 1: // Conquistar 3 territorios consecutivos
            {
                // Aloca memória para armazenar os territórios do jogador
                int* territoriosJogador = (int*)malloc(totalTerritorios * sizeof(int));
                int count = 0;
                
                for (int i = 0; i < totalTerritorios; i++) {
                    if (mapa[i].jogador_id == jogador_id) {
                        territoriosJogador[count++] = i;
                    }
                }
                
                int resultado = 0;
                
                // Verifica se existem 3 territórios consecutivos
                if (count >= 3) {
                    for (int i = 0; i < count - 2; i++) {
                        for (int j = i + 1; j < count - 1; j++) {
                            for (int k = j + 1; k < count; k++) {
                                if (saoConsecutivos(mapa, territoriosJogador[i], territoriosJogador[j], territoriosJogador[k]) ||
                                    saoConsecutivos(mapa, territoriosJogador[i], territoriosJogador[k], territoriosJogador[j]) ||
                                    saoConsecutivos(mapa, territoriosJogador[j], territoriosJogador[i], territoriosJogador[k]) ||
                                    saoConsecutivos(mapa, territoriosJogador[j], territoriosJogador[k], territoriosJogador[i]) ||
                                    saoConsecutivos(mapa, territoriosJogador[k], territoriosJogador[i], territoriosJogador[j]) ||
                                    saoConsecutivos(mapa, territoriosJogador[k], territoriosJogador[j], territoriosJogador[i])) {
                                    resultado = 1;
                                    break;
                                }
                            }
                            if (resultado) break;
                        }
                        if (resultado) break;
                    }
                }
                
                free(territoriosJogador);
                return resultado;
            }
            
        case 2: // Eliminar todas as tropas de um jogador especifico
            for (int i = 0; i < totalTerritorios; i++) {
                if (mapa[i].jogador_id == missao.jogador_alvo && mapa[i].tropas > 0) return 0;
            }
            return 1;
            
        case 3: // Controlar pelo menos 2 territorios
            {
                int contador = 0;
                for (int i = 0; i < totalTerritorios; i++) {
                    if (mapa[i].jogador_id == jogador_id) contador++;
                }
                return contador >= 2;
            }
            
        case 4: // Ter mais de 13 tropas somadas
            {
                int soma = 0;
                for (int i = 0; i < totalTerritorios; i++) {
                    if (mapa[i].jogador_id == jogador_id) soma += mapa[i].tropas;
                }
                return soma > 13;
            }
            
        case 5: // Conquistar um territorio na primeira tentativa
            return conquistaPrimeiroTurno && turnos == 1;
            
        default:
            return 0;
    }
}

// Funcao para cadastrar os territorios
void cadastrarTerritorios(Territorio* mapa, int total) {
    char entrada[10];
    char coresTemp[6][15];
    int coresRestantes = totalCores;

    // Copia as cores disponíveis para um array temporário
    for (int i = 0; i < totalCores; i++) {
        strcpy(coresTemp[i], coresDisponiveis[i]);
    }

    for (int i = 0; i < total; i++) {
        printf("\nCadastro do territorio %d:\n", i + 1);
        printf("Digite o nome do territorio: ");
        fgets(mapa[i].nome, sizeof(mapa[i].nome), stdin);
        mapa[i].nome[strcspn(mapa[i].nome, "\n")] = '\0';

        // Define jogador_id como 0 (neutro) inicialmente
        mapa[i].jogador_id = 0;
        
        // Permite escolher a cor (apenas cores disponíveis)
        printf("Escolha a cor para este territorio:\n");
        for (int j = 0; j < coresRestantes; j++) {
            printf("%d - %s\n", j + 1, coresTemp[j]);
        }
        
        int escolhaCor;
        int leituraValida = 0;
        do {
            printf("Digite o numero da cor desejada: ");
            fgets(entrada, sizeof(entrada), stdin);
            leituraValida = sscanf(entrada, "%d", &escolhaCor);
            if (leituraValida != 1 || escolhaCor < 1 || escolhaCor > coresRestantes) {
                printf("Entrada invalida! Digite um numero entre 1 e %d.\n", coresRestantes);
            }
        } while (leituraValida != 1 || escolhaCor < 1 || escolhaCor > coresRestantes);
        
        strcpy(mapa[i].cor, coresTemp[escolhaCor - 1]);

        // Remove a cor escolhida das opções disponíveis
        for (int j = escolhaCor - 1; j < coresRestantes - 1; j++) {
            strcpy(coresTemp[j], coresTemp[j + 1]);
        }
        coresRestantes--;

        int leituraTropas = 0;
        do {
            printf("Digite a quantidade de tropas (1-10): ");
            fgets(entrada, sizeof(entrada), stdin);
            leituraTropas = sscanf(entrada, "%d", &mapa[i].tropas);
            if (leituraTropas != 1 || mapa[i].tropas < 1 || mapa[i].tropas > 10) {
                printf("Entrada invalida! Digite um numero entre 1 e 10.\n");
            }
        } while (leituraValida != 1 || mapa[i].tropas < 1 || mapa[i].tropas > 10);
    }
}

// Exibe os territorios
void exibirTerritorios(Territorio* mapa, int total) {
    printf("\n===============================\n");
    printf("    Territorios Atualizados    \n");
    printf("===============================\n");
    for (int i = 0; i < total; i++) {
        printf("\nTerritorio %d:\n", i + 1);
        printf(" - Nome: %s\n", mapa[i].nome);
        printf(" - Cor: %s\n", mapa[i].cor);
        printf(" - Dono: ");
        if (mapa[i].jogador_id == 0) printf("Neutro\n");
        else if (mapa[i].jogador_id == 1) printf("Jogador\n");
        else printf("Inimigo %d\n", mapa[i].jogador_id);
        printf(" - Tropas: %d\n", mapa[i].tropas);
        
        // Mostra vizinhos
        printf(" - Vizinhos: ");
        if (mapa[i].num_vizinhos == 0) {
            printf("Nenhum");
        } else {
            for (int j = 0; j < mapa[i].num_vizinhos; j++) {
                printf("%s", mapa[mapa[i].territorios_vizinhos[j]].nome);
                if (j < mapa[i].num_vizinhos - 1) printf(", ");
            }
        }
        printf("\n");
    }
}

// Simula ataque entre dois territorios
int atacar(Territorio* atacante, Territorio* defensor, int* tropasDerrubadas, int turnoAtual) {
    if (atacante->jogador_id == defensor->jogador_id && atacante->jogador_id != 0) {
        printf("\nVoce nao pode atacar um territorio do mesmo jogador!\n");
        return 0;
    }

    int dadoAtacante = rand() % 6 + 1;
    int dadoDefensor = rand() % 6 + 1;

    printf("\nAtaque iniciado!\n");
    printf("Dado do atacante (%s): %d\n", atacante->nome, dadoAtacante);
    printf("Dado do defensor (%s): %d\n", defensor->nome, dadoDefensor);

    if (dadoAtacante > dadoDefensor) {
        printf("O atacante venceu e conquistou o territorio!\n");
        *tropasDerrubadas += defensor->tropas;
        
        // Lógica de conquista CORRIGIDA - O território é totalmente transferido
        defensor->jogador_id = atacante->jogador_id;
        
        // O defensor recebe todas as tropas do atacante (menos 1 pela batalha)
        defensor->tropas = atacante->tropas - 1;
        if (defensor->tropas < 1) defensor->tropas = 1;
        
        // O atacante mantém apenas uma tropa no território original
        atacante->tropas = 1;
        
        // Se for o primeiro turno e uma conquista, marca como conquista no primeiro turno
        if (turnoAtual == 1) {
            return 2; // Conquista no primeiro turno
        }
        
        return 1; // Conquista bem-sucedida
    } else {
        printf("O defensor resistiu! O atacante perde uma tropa.\n");
        if (atacante->tropas > 1) {
            atacante->tropas--;
            *tropasDerrubadas += 1;
        }
        
        // Verifica se o atacante foi eliminado
        if (atacante->tropas == 0) {
            printf("O territorio %s foi completamente destruido e volta a ser neutro!\n", atacante->nome);
            atacante->jogador_id = 0; // Volta a ser neutro
            atacante->tropas = 1; // Recebe uma tropa minima
        }
        
        return -1; // Ataque falhou
    }
}

// Verifica se um jogador ainda tem territórios com tropas
int jogadorTemTerritorios(Territorio* mapa, int total, int jogador_id) {
    for (int i = 0; i < total; i++) {
        if (mapa[i].jogador_id == jogador_id && mapa[i].tropas > 0) {
            return 1;
        }
    }
    return 0;
}

// Libera memoria alocada
void liberarMemoria(Territorio* mapa, int total) {
    for (int i = 0; i < total; i++) {
        if (mapa[i].territorios_vizinhos != NULL) {
            free(mapa[i].territorios_vizinhos);
        }
    }
    free(mapa);
}

int main() {
    srand(time(NULL));

    int total;
    char entrada[10];
    int leituraValida = 0;

    printf("=====================================\n");
    printf(" Bem-vindo ao Jogo War Estruturado!\n");
    printf("=====================================\n");

    do {
        printf("Quantos territorios voce deseja (minimo 2, maximo 5)? ");
        fgets(entrada, sizeof(entrada), stdin);
        leituraValida = sscanf(entrada, "%d", &total);
        if (leituraValida != 1 || total < 2 || total > 5) {
            printf("Entrada invalida! Digite um numero entre 2 e 5.\n");
        }
    } while (leituraValida != 1 || total < 2 || total > 5);

    Territorio* mapa = (Territorio*)calloc(total, sizeof(Territorio));
    if (mapa == NULL) {
        printf("Erro ao alocar memoria.\n");
        return 1;
    }

    // Inicializa o mapa com adjacências
    inicializarMapa(mapa, total);

    // Define número de jogadores (pelo menos 2: jogador + 1 inimigo)
    int totalJogadores = 2 + rand() % 2; // 2 ou 3 jogadores
    Missao missaoJogador = atribuirMissao(totalJogadores);
    
    printf("\nMissao atribuida: %s", missaoJogador.descricao);
    if (missaoJogador.tipo == 2) {
        printf(" (Jogador %d)", missaoJogador.jogador_alvo);
    }
    printf("\n");

    cadastrarTerritorios(mapa, total);
    
    // Define automaticamente os donos dos territórios (todos começam como neutros)
    printf("\nTodos os territorios comecam como neutros (sem dono).\n");
    printf("Os donos serao definidos durante os ataques!\n");
    
    exibirTerritorios(mapa, total);

    int tropasDerrubadas = 0;
    int atacante_idx, defensor_idx;
    int turnos = 0;
    int conquistaPrimeiroTurno = 0;

    // Primeiro turno
    printf("\n===== PRIMEIRO TURNO =====\n");
    
    // O jogador precisa escolher um território para ser seu (atacante)
    do {
        printf("\nEscolha o territorio que sera SEU (1 a %d): ", total);
        fgets(entrada, sizeof(entrada), stdin);
        leituraValida = sscanf(entrada, "%d", &atacante_idx);
    } while (leituraValida != 1 || atacante_idx < 1 || atacante_idx > total);
    
    // Define o território escolhido como do jogador
    mapa[atacante_idx-1].jogador_id = 1;
    printf("Territorio '%s' agora e seu!\n", mapa[atacante_idx-1].nome);
    
    // Distribui os territórios restantes entre os inimigos
    int inimigoAtual = 2;
    for (int i = 0; i < total; i++) {
        if (i != atacante_idx - 1 && mapa[i].jogador_id == 0) {
            mapa[i].jogador_id = inimigoAtual;
            inimigoAtual = (inimigoAtual % totalJogadores) + 1;
            if (inimigoAtual == 1) inimigoAtual = 2; // Pula o jogador 1
        }
    }
    
    printf("Territorios distribuidos entre os inimigos!\n");
    
    exibirTerritorios(mapa, total);
    
    // Agora realiza o primeiro ataque
    printf("\nRealizando primeiro ataque...\n");
    
    // Escolhe um defensor inimigo
    do {
        printf("Escolha o territorio DEFENSOR (1 a %d) que seja inimigo: ", total);
        fgets(entrada, sizeof(entrada), stdin);
        leituraValida = sscanf(entrada, "%d", &defensor_idx);
    } while (leituraValida != 1 || defensor_idx < 1 || defensor_idx > total || 
             mapa[defensor_idx-1].jogador_id == 1 || mapa[defensor_idx-1].jogador_id == 0);
    
    int resultado = atacar(&mapa[atacante_idx - 1], &mapa[defensor_idx - 1], &tropasDerrubadas, 1);
    turnos++;
    
    if (resultado == 2) {
        conquistaPrimeiroTurno = 1;
        printf("Conquista realizada no primeiro turno!\n");
    }
    
    exibirTerritorios(mapa, total);

    // Verifica se o jogo terminou após o primeiro turno
    int jogadoresRestantes = 0;
    for (int jogador = 2; jogador <= totalJogadores; jogador++) {
        if (jogadorTemTerritorios(mapa, total, jogador)) {
            jogadoresRestantes++;
        }
    }
    
    if (jogadoresRestantes == 0) {
        printf("\nFim do jogo. Você venceu!\n");
        if (verificarMissao(missaoJogador, mapa, total, 1, turnos, conquistaPrimeiroTurno)) {
            printf("Parabens! Voce cumpriu sua missao: %s\n", missaoJogador.descricao);
        } else {
            printf("Missao nao cumprida: %s\n", missaoJogador.descricao);
        }
        liberarMemoria(mapa, total);
        return 0;
    }

    // Turnos seguintes
    printf("\n===== TURNOS SEGUINTES =====\n");
    while (1) {
        char resposta;
        printf("\nDeseja continuar jogando? (s/n): ");
        scanf(" %c", &resposta);
        while (getchar() != '\n'); // Limpa o buffer

        if (resposta == 'n' || resposta == 'N') {
            printf("\nFim do jogo.\n");
            break;
        }
        
        turnos++;
        
        // Escolher atacante (deve ser do jogador e ter mais de 1 tropa)
        do {
            printf("\nEscolha o territorio ATACANTE (1 a %d) que seja seu: ", total);
            fgets(entrada, sizeof(entrada), stdin);
            leituraValida = sscanf(entrada, "%d", &atacante_idx);
            if (leituraValida == 1 && atacante_idx >= 1 && atacante_idx <= total) {
                if (mapa[atacante_idx-1].jogador_id != 1) {
                    printf("Este territorio nao e seu! Escolha um territorio do jogador.\n");
                } else if (mapa[atacante_idx-1].tropas <= 1) {
                    printf("Este territorio nao tem tropas suficientes para atacar! (minimo 2 tropas)\n");
                }
            }
        } while (leituraValida != 1 || atacante_idx < 1 || atacante_idx > total || 
                mapa[atacante_idx-1].jogador_id != 1 || mapa[atacante_idx-1].tropas <= 1);

        // Escolher defensor (deve ser inimigo)
        do {
            printf("Escolha o territorio DEFENSOR (1 a %d) que seja inimigo: ", total);
            fgets(entrada, sizeof(entrada), stdin);
            leituraValida = sscanf(entrada, "%d", &defensor_idx);
            if (leituraValida == 1 && defensor_idx >= 1 && defensor_idx <= total) {
                if (mapa[defensor_idx-1].jogador_id == 1) {
                    printf("Este territorio e seu! Escolha um territorio inimigo.\n");
                } else if (mapa[defensor_idx-1].jogador_id == 0) {
                    printf("Este territorio e neutro! Escolha um territorio inimigo.\n");
                }
            }
        } while (leituraValida != 1 || defensor_idx < 1 || defensor_idx > total || 
                 mapa[defensor_idx-1].jogador_id == 1 || mapa[defensor_idx-1].jogador_id == 0);

        resultado = atacar(&mapa[atacante_idx - 1], &mapa[defensor_idx - 1], &tropasDerrubadas, turnos);
        exibirTerritorios(mapa, total);
        
        // Verifica se o jogo terminou
        jogadoresRestantes = 0;
        for (int jogador = 2; jogador <= totalJogadores; jogador++) {
            if (jogadorTemTerritorios(mapa, total, jogador)) {
                jogadoresRestantes++;
            }
        }
        
        if (jogadoresRestantes == 0) {
            printf("\nFim do jogo. Você venceu!\n");
            break;
        }
    }

    // Verificação final da missão
    if (verificarMissao(missaoJogador, mapa, total, 1, turnos, conquistaPrimeiroTurno)) {
        printf("Parabens! Voce cumpriu sua missao: %s", missaoJogador.descricao);
        if (missaoJogador.tipo == 2) {
            printf(" (Jogador %d)", missaoJogador.jogador_alvo);
        }
        printf("\n");
    } else {
        printf("Missao nao cumprida: %s", missaoJogador.descricao);
        if (missaoJogador.tipo == 2) {
            printf(" (Jogador %d)", missaoJogador.jogador_alvo);
        }
        printf("\n");
    }

    liberarMemoria(mapa, total);
    return 0;
}
