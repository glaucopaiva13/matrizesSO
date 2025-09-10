#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void gerar_matriz(const char *nome_arquivo, int linhas, int colunas) {
    FILE *fp = fopen(nome_arquivo, "w");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    fprintf(fp, "%d %d\n", linhas, colunas);

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            int valor = rand() % 10;
            fprintf(fp, "%d ", valor);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Uso: %s n1 m1 n2 m2\n", argv[0]);
        return 1;
    }

    int n1 = atoi(argv[1]);
    int m1 = atoi(argv[2]);
    int n2 = atoi(argv[3]);
    int m2 = atoi(argv[4]);

    srand(time(NULL));

    gerar_matriz("matriz1.txt", n1, m1);
    gerar_matriz("matriz2.txt", n2, m2);

    printf("Arquivos 'matriz1.txt' e 'matriz2.txt' gerados com sucesso!\n");

    return 0;
}
