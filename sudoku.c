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
    int tam;
    
} subgrid;


int erro=0;

void ordenar(int arr[], int n);
void *verificaLinha(void *ptr);
void *verificaGrid(void *ptr);
void *verificaColuna(void *ptr);

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
    fscanf(f, "%dx%d\n%dx%d", &linhas, &colunas, &sub_linhas, &sub_colunas);

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
    
    for (int i = 0; i < sub_linhas*sub_colunas; i++) {

        sg[i].sudoku = matriz;
        sg[i].linhaSubgrid = (i/sub_linhas)*sub_linhas;
        sg[i].colunaSubgrid = (i%sub_colunas)*sub_colunas;
        sg[i].tam = sub_linhas;

        int tgrid = pthread_create(&threads_subgrids[i], NULL, verificaGrid, (void *)&sg[i]);
        if (tgrid) {
            printf("Error - pthread_create() return code: %d\n", tgrid);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < sub_linhas*sub_colunas; i++) {
        pthread_join(threads_subgrids[i], NULL);
    }

    for (int i = 0; i < linhas; i++) {
        pthread_join(threads[i], NULL);
    }

    if(!erro)
        printf("certinho papai\n");
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
            printf("deu erro na linha: %d\n",ds->linha);
            break;
        }
    }
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
            printf("deu erro na coluna: %d\n",ds->coluna);
            break;
        }
    }
}

void *verificaGrid(void *ptr){

    subgrid *sg;
    sg = (subgrid *) ptr;
    int listaV[sg->tam];
    int listaIndex=0;

    for(int i = sg->linhaSubgrid; i < sg->linhaSubgrid + sg->tam; i++){
        for(int j = sg->colunaSubgrid; j < sg->colunaSubgrid + sg->tam; j++){
            listaV[listaIndex++] = sg->sudoku[i][j];
        }

    }

    ordenar(listaV, sg->tam*sg->tam);

    for(int i = 0; i < sg->tam*sg->tam; i++){
        if(listaV[i] != i+1){
            erro = 1;
            printf("deu erro no subgrid: %dx%d\n", sg->linhaSubgrid/sg->tam, sg->colunaSubgrid/sg->tam);
            break;
        }
    }

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

