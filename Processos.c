#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>

typedef struct {
    int id;
    int inicio;
    int fim;
    int linhas1, colunas1, colunas2;
    double **M1, **M2;
} DadosDoProcesso;

// Lê matriz do arquivo e aloca memória
void ler_matriz(const char *nome_arquivo, int *linhas, int *colunas, double ***matriz) {
    FILE *fp = fopen(nome_arquivo, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    fscanf(fp, "%d %d", linhas, colunas);
    *matriz = (double **)malloc((*linhas) * sizeof(double *));
    
    for (int i = 0; i < *linhas; i++) {
        (*matriz)[i] = (double *)malloc((*colunas) * sizeof(double));
        for (int j = 0; j < *colunas; j++) {
            fscanf(fp, "%lf", &(*matriz)[i][j]);
        }
    }

    fclose(fp);
}

// Função executada por cada processo filho
void processo_filho(DadosDoProcesso *data) {
    clock_t inicio = clock();

    // Aloca matriz resultado local
    double **C_local = (double **)malloc(data->linhas1 * sizeof(double *));
    for (int i = 0; i < data->linhas1; i++) {
        C_local[i] = (double *)calloc(data->colunas2, sizeof(double));
    }

    // Calcula elementos atribuídos a este processo
    for (int index = data->inicio; index < data->fim; index++) {
        int i = index / data->colunas2;
        int j = index % data->colunas2;
        
        for (int k = 0; k < data->colunas1; k++) {
            C_local[i][j] += data->M1[i][k] * data->M2[k][j];
        }
    }

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;

    // Salva resultado parcial
    char nome_arquivo[50];
    sprintf(nome_arquivo, "resultado_processo_%d.txt", data->id);
    FILE *fp = fopen(nome_arquivo, "w");
    
    fprintf(fp, "%d %d\n", data->linhas1, data->colunas2);
    for (int index = data->inicio; index < data->fim; index++) {
        int i = index / data->colunas2;
        int j = index % data->colunas2;
        fprintf(fp, "c%d%d %.3f\n", i+1, j+1, C_local[i][j]);
    }
    fprintf(fp, "%.6f\n", tempo);
    fclose(fp);

    // Libera memória
    for (int i = 0; i < data->linhas1; i++) {
        free(C_local[i]);
    }
    free(C_local);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s matriz1.txt matriz2.txt P\n", argv[0]);
        return 1;
    }

    int P = atoi(argv[3]);
    if (P <= 0) {
        printf("Erro: P deve ser maior que 0\n");
        return 1;
    }

    int l1, c1, l2, c2;
    double **M1, **M2;

    // Lê matrizes
    ler_matriz(argv[1], &l1, &c1, &M1);
    ler_matriz(argv[2], &l2, &c2, &M2);
    
    if (c1 != l2) {
        printf("Erro: não é possível multiplicar matrizes %dx%d e %dx%d\n", l1, c1, l2, c2);
        return 1;
    }

    int total = l1 * c2;
    int num_processos = (total + P - 1) / P;  // ceil(total/P)
    
    if (P > total) {
        P = total;
        num_processos = total;
    }

    pid_t *pids = malloc(num_processos * sizeof(pid_t));
    DadosDoProcesso *dados = malloc(num_processos * sizeof(DadosDoProcesso));

    // Prepara dados antes de medir tempo
    int inicio = 0;
    for (int p = 0; p < num_processos; p++) {
        int qtd = P;
        if (inicio + qtd > total) {
            qtd = total - inicio;
        }
        int fim = inicio + qtd;

        dados[p].id = p;
        dados[p].inicio = inicio;
        dados[p].fim = fim;
        dados[p].linhas1 = l1;
        dados[p].colunas1 = c1;
        dados[p].colunas2 = c2;
        dados[p].M1 = M1;
        dados[p].M2 = M2;

        inicio = fim;
    }

    // Inicia medição e cria processos
    clock_t inicio_geral = clock();

    for (int p = 0; p < num_processos; p++) {
        pids[p] = fork();
        
        if (pids[p] == -1) {
            perror("Erro ao criar processo");
            for (int k = 0; k < p; k++) {
                kill(pids[k], SIGTERM);
            }
            exit(1);
        } else if (pids[p] == 0) {
            // Processo filho
            processo_filho(&dados[p]);
            exit(0);
        }
    }

    // Espera todos os processos terminarem
    for (int p = 0; p < num_processos; p++) {
        int status;
        waitpid(pids[p], &status, 0);
    }

    clock_t fim_geral = clock();
    double tempo_total = (double)(fim_geral - inicio_geral) / CLOCKS_PER_SEC;

    printf("Calculo paralelo concluido em %.6f segundos\n", tempo_total);
    printf("Resultados salvos em %d arquivos (resultado_processo_X.txt)\n", num_processos);

    // Libera memória
    for (int i = 0; i < l1; i++) free(M1[i]);
    for (int i = 0; i < l2; i++) free(M2[i]);
    free(M1);
    free(M2);
    free(pids);
    free(dados);

    return 0;
}