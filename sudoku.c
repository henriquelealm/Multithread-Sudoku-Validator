#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct 
{
    int tam;
    int **sudoku;
    int linha;
    int coluna;
    
} datastruct;

typedef struct 
{
    int **sudoku;
    int linhaSubgrid;
    int colunaSubgrid;
    int nLinhas;
    int nColunas;
    
} subgrid;

int erro=0;

void ordenar(int *arr, int n);
void *verificaLinha(void *ptr);
void *verificaSubgrid(void *ptr);
void *verificaColuna(void *ptr);
int min(int a, int b);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Invalid number of parameters");
        exit(EXIT_FAILURE);
    }

    FILE *f = fopen(argv[1], "r");

    if (f == NULL) {
        fprintf(stderr, "Error to read file");
        exit(EXIT_FAILURE);
    }
    
    int linhas, colunas, sub_linhas, sub_colunas;
    long int ptrPos;
    char l1[10],l2[10];

    fgets(l1, 10, f);
    fgets(l2, 10, f);

    if ((strchr(l1, ' ') != NULL) || (strchr(l2, ' ') != NULL)){
        fprintf(stderr, "File out of format");
        exit(EXIT_FAILURE);
    }

    sscanf(l1, "%dx%d", &linhas, &colunas);
    sscanf(l2, "%dx%d", &sub_linhas, &sub_colunas);

    if(linhas != colunas){
        fprintf(stderr, "File out of format");
        exit(EXIT_FAILURE);
    }

    if(((sub_linhas*sub_colunas) != linhas) || ((sub_linhas*sub_colunas) != colunas)){
        fprintf(stderr, "File out of format");
        exit(EXIT_FAILURE);
    }

    ptrPos = ftell(f);
    int countN = 0,num,countC = 0,countL=1;
    char elemento;
    
    while (fscanf(f, "%d", &num) == 1) {
        if(num > linhas || num < 1){
            fprintf(stderr, "File out of format");
            exit(EXIT_FAILURE);
        }
        countN++;
    }

    fseek(f, ptrPos, SEEK_SET);

    while ((elemento = fgetc(f)) != EOF) {
        if (elemento == ' ') {
            countC++;
        }
        else if (elemento == '\n') {
            countL++;
        }
    }

    fseek(f, ptrPos, SEEK_SET);

    if(countC != (linhas*colunas)-linhas){
        fprintf(stderr, "File out of format");
        exit(EXIT_FAILURE);
    }

    if(countN != linhas * colunas){
        fprintf(stderr, "File out of format");
        exit(EXIT_FAILURE);
    }

    if(countL != linhas){
        fprintf(stderr, "File out of format");
        exit(EXIT_FAILURE);       
    }


    int **matriz = malloc(linhas * sizeof(int *));

    for (int i = 0; i < linhas; i++) {
        matriz[i] = malloc(colunas * sizeof(int));
    }

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++) {
            fscanf(f, "%d", &matriz[i][j]);
        }
    }

    pthread_t threads[linhas];
    datastruct ds[linhas];
    
    for (int i = 0; i < linhas; i++) {

        ds[i].tam = colunas;
        ds[i].sudoku = matriz;
        ds[i].linha = i;
        ds[i].coluna = i;

        int tlinha = pthread_create(&threads[i], NULL, verificaLinha, (void *)&ds[i]);
        int tcoluna = pthread_create(&threads[i], NULL, verificaColuna, (void *)&ds[i]);

        if (tlinha) {
            printf("Error - pthread_create() return code: %d\n", tlinha);
            exit(EXIT_FAILURE);
        }

        if (tcoluna) {
            printf("Error - pthread_create() return code: %d\n", tcoluna);
            exit(EXIT_FAILURE);
        }
    }

    pthread_t threads_subgrids[sub_linhas*sub_colunas];
    subgrid sg[sub_linhas*sub_colunas];

    int cont = 0;

    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < linhas; j++) {

            if (i % sub_linhas == 0 && j % sub_colunas == 0) {

                sg[cont].sudoku = matriz;
                sg[cont].linhaSubgrid = i;
                sg[cont].colunaSubgrid = j;
                sg[cont].nLinhas = sub_linhas;
                sg[cont].nColunas = sub_colunas;

                int tsubgrid = pthread_create(&threads_subgrids[cont], NULL, verificaSubgrid, (void *)&sg[cont]);

                if (tsubgrid) {
                    printf("Error - pthread_create() return code: %d\n", tsubgrid);
                    exit(EXIT_FAILURE);
                }
                cont++;
            }
        }
    }

    for (int i = 0; i < linhas; i++) {
        pthread_join(threads[i], NULL);
        pthread_join(threads_subgrids[i], NULL);
    }

    FILE *saida = fopen("sudoku_hlm.out", "w+");

    if(!erro)
        fprintf(saida,"SUCCESS");

    else
        fprintf(saida,"FAIL");

    free(matriz);
    pthread_exit(NULL);
    return 0;

}

void *verificaLinha(void *ptr) {

    datastruct *ds;
    ds = (datastruct *) ptr;
    int listaVerificadora[ds->tam];

    for (int i = 0; i < ds->tam; i++) {
        listaVerificadora[i] = ds->sudoku[ds->linha][i];
    }

    ordenar(listaVerificadora, ds->tam);

    for(int i = 0; i< ds->tam; i++){
        if(listaVerificadora[i] != i+1){
            erro = 1;
            //printf("deu erro na linha: %d\n",ds->linha);
            break;
        }
    }
    return NULL;
}

void *verificaColuna(void *ptr) {

    datastruct *ds;
    ds = (datastruct *) ptr;
    int listaVerificadora[ds->tam];

    for (int i = 0; i < ds->tam; i++) {
        listaVerificadora[i] = ds->sudoku[i][ds->coluna];
    }

    ordenar(listaVerificadora, ds->tam);

    for(int i = 0; i< ds->tam; i++){
        if(listaVerificadora[i] != i+1){
            erro = 1;
            //printf("deu erro na coluna: %d\n",ds->coluna);
            break;
        }
    }
    return NULL;
}

void *verificaSubgrid(void *ptr) {
    subgrid *sg;
    sg = (subgrid *) ptr;
    int listaVerificadora[sg->nLinhas * sg->nColunas];

    int cont = 0;
    int N = sg->nLinhas * sg->nColunas; 
    for (int i = sg->linhaSubgrid; i < sg->linhaSubgrid + sg->nLinhas; i++) {
        for (int j = sg->colunaSubgrid; j < sg->colunaSubgrid + sg->nColunas; j++) {
           // if (i < 0 || i >= N || j < 0 || j >= N) continue; 
            listaVerificadora[cont] = sg->sudoku[i][j];
            cont++;
        }
    }

    ordenar(listaVerificadora, sg->nLinhas * sg->nColunas);

    for(int i = 0; i < sg->nLinhas * sg->nColunas; i++){
        if(listaVerificadora[i] != i+1){
            erro = 1;
            //printf("deu erro na subgrid: linha %d a %d, coluna %d a %d\n", sg->linhaSubgrid, sg->linhaSubgrid + sg->nLinhas - 1, sg->colunaSubgrid, sg->colunaSubgrid + sg->nColunas - 1);
            break;
        }
    }

    return NULL;
}
void ordenar(int *arr, int n) {
    int i, j, temp;
    for (i = 0; i < n-1; i++) {
        for (j = 0; j < n-i-1; j++) {
            if (arr[j] > arr[j+1]) {
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }

}
int min(int a, int b) {
  return a < b ? a : b;
}
