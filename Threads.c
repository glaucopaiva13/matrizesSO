#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
	int id;
	int inicio;
	int fim;
	int linhas1, colunas1, colunas2;
	double **M1, **M2;
} DadosDaThread;

double **C;
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

void *multiplica(void *arg) {
	DadosDaThread *data = (DadosDaThread *)arg;
	clock_t inicio_t = clock();

	for (int index = data->inicio; index < data->fim; index++) {
		int i = index / data->colunas2; 
		int j = index % data->colunas2; 
		C[i][j] = 0;
		for (int k = 0; k < data->colunas1; k++) {
			C[i][j] += data->M1[i][k] * data->M2[k][j];
		}
	}

	clock_t fim_t = clock();
	double tempo = (double)(fim_t - inicio_t) / CLOCKS_PER_SEC;

	char nome_arquivo[50];
	sprintf(nome_arquivo, "resultado_thread_%d.txt", data->id);
	FILE *fp = fopen(nome_arquivo, "w");
	fprintf(fp, "%d %d\n", data->linhas1, data->colunas2);
	for (int index = data->inicio; index < data->fim; index++) {
		int i = index / data->colunas2;
		int j = index % data->colunas2;
		fprintf(fp, "c%d%d %.3f\n", i+1, j+1, C[i][j]);
	}
	fprintf(fp, "%.6f\n", tempo);
	fclose(fp);

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		printf("Uso: %s matriz1.txt matriz2.txt P\n", argv[0]);
		return 1;
	}

	int P = atoi(argv[3]);

	int l1, c1, l2, c2;
	double **M1, **M2;

	ler_matriz(argv[1], &l1, &c1, &M1);
	ler_matriz(argv[2], &l2, &c2, &M2);
	if (c1 != l2) {
		printf("Erro: não é possível multiplicar matrizes %dx%d e %dx%d\n", l1, c1, l2, c2);
		return 1;
	}

	C = (double **)malloc(l1 * sizeof(double *));
	for (int i = 0; i < l1; i++) {
		C[i] = (double *)calloc(c2, sizeof(double));
	}

	int total = l1 * c2;
	int base = total / P;
    	int resto = total % P;

    	pthread_t threads[P];
    	DadosDaThread dados[P];

	int inicio = 0;
	for (int t = 0; t < P; t++) {
		int qtd = base + (t < resto ? 1 : 0);
        	int fim = inicio + qtd;

        	dados[t].id = t;
        	dados[t].inicio = inicio;
        	dados[t].fim = fim;
        	dados[t].linhas1 = l1;
        	dados[t].colunas1 = c1;
        	dados[t].colunas2 = c2;
        	dados[t].M1 = M1;
        	dados[t].M2 = M2;

        	pthread_create(&threads[t], NULL, multiplica, &dados[t]);
		inicio = fim;
    }

    	for (int t = 0; t < P; t++) {
		pthread_join(threads[t], NULL);
    	}

    	printf("Calculo paralelo concluido. Resultados salvos em arquivos individuais.\n");

    	for (int i = 0; i < l1; i++) free(M1[i]);
    	for (int i = 0; i < l2; i++) free(M2[i]);
    	for (int i = 0; i < l1; i++) free(C[i]);
    	free(M1);
    	free(M2);
	free(C);

    return 0;
}
