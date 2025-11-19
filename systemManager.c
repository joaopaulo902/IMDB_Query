//
// Created by andri on 19/11/2025.
//

#include "systemManager.h"
#include "entities.h"
#include <stdio.h>
#include <stdlib.h>


void initializeSystem() {
    FILE *ptFile;
    ptFile = fopen("title.txt", "r");
    if (ptFile != NULL) {
        while (!feof(ptFile)) {
            char letter = fgetc(ptFile);
            printf("%c", letter);
        }
        fclose(ptFile);
    } else {
        printf("Erro abrindo imagem inicial.\n");
    }
}

void clearScreen() {
    system("cls || clear");
}

void printHeader(int currentPage, int totalPages) {
    printf("==============================================\n");
    printf("  LISTA DE FILMES  |  Pagina %d de %d\n", currentPage + 1, totalPages);
    printf("==============================================\n");
    printf("%-4s | %-30s | %-4s | %-6s\n", "#", "Titulo", "Ano", "Tipo");
    printf("-----+--------------------------------+------+--------\n");
}

void printPage(Title *titles, int totalMovies, int currentPage) {
    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;
    int start = currentPage * PAGE_SIZE;
    int end = start + PAGE_SIZE;
    if (end > totalMovies) end = totalMovies;

    clearScreen();
    printHeader(currentPage, totalPages);

    for (int i = start; i < end; i++) {
        printf("%-4d | %-30s | %-4d | %-10s\n",
               i + 1,
               titles[i].primaryTitle,
               titles[i].startYear,
               titles[i].type);
    }

    printf("==============================================\n");
    printf("[n] Proxima pagina  [p] Pagina anterior  [i] Info  [q] Sair\n");
    printf("Comando: ");
}

void show_info(Title *movies, int totalMovies) {
    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    printf("\n--- Informacoes gerais ---\n");
    printf("Total de filmes: %d\n", totalMovies);
    printf("Total de paginas: %d (tamanho da pagina: %d)\n", totalPages, PAGE_SIZE);
    printf("Pressione ENTER para continuar...");

    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void updateData() {
    // Placeholder for future data update functionality
    printf("Funcao de atualizacao de dados ainda nao implementada.\n");
}