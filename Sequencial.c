#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

void salvar_resultado(const char *nome_arquivo, double **C, int linhas, int colunas, double tempo) {
	FILE *fp = fopen(nome_arquivo, "w");
	if (!fp) {
		perror("Erro ao salvar arquivo");
		exit(1);
	}
	fprintf(fp, "%d %d\n", linhas, colunas);
	for (int i = 0; i < linhas; i++) {
		for (int j = 0; j < colunas; j++) {
			fprintf(fp, "c%d%d %.3f\n", i+1, j+1, C[i][j]);
		}
	}
	fprintf(fp, "%.3f\n", tempo);
	fclose(fp);
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Uso: %s matriz1.txt matriz2.txt\n", argv[0]);
		return 1;
	}

	int l1, c1, l2, c2;
	double **M1, **M2;

	ler_matriz(argv[1], &l1, &c1, &M1);
	ler_matriz(argv[2], &l2, &c2, &M2);
	if (c1 != l2) {
		printf("Erro: não é possivel multiplicar matrizes %dx%d e %dx%d\n", l1, c1, l2, c2);
		return 1;
	}
	double **C = (double **)malloc(l1 * sizeof(double *));
	for (int i = 0; i < l1; i++) {
		C[i] = (double *)calloc(c2, sizeof(double));
	}

	clock_t inicio = clock();

	for (int i = 0; i < l1; i++) {
		for (int j = 0; j < c2; j++) {
			for (int k = 0; k < c1; k++) {
				C[i][j] += M1[i][k] * M2[k][j];
			}
		}
	}
	clock_t fim = clock();
	double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
	salvar_resultado("resultado.txt", C, l1, c2, tempo);
	printf("Resultado salvo em 'resultado.txt'\n");

	for (int i = 0; i < l1; i++) free(M1[i]);
	for (int i = 0; i < l2; i++) free(M2[i]);
	for (int i = 0; i < l1; i++) free(C[i]);
	free(M1);
	free(M2);
	free(C);
	return 0;
}

