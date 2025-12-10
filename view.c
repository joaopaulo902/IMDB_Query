//
// Created by andri on 08/12/2025.
//

#include "view.h"

#include <stdio.h>

#include "filterGenre.h"

void read_title() {
    FILE *ptFile = fopen("title.txt", "r");
    if (!ptFile) return;

    int c;
    while ((c = fgetc(ptFile)) != EOF)
        putchar(c);

    putchar('\n');
    fclose(ptFile);
}


void clear_screen() {
    // Hide cursor
    printf("\033[?25l");
    // Clear screen
    printf("\033[2J");
    // Move cursor to top-left
    printf("\033[H");
    // Show cursor
    fflush(stdout);
}

void print_title_list_header(int currentPage, int totalPages) {
    read_title();
    printf("=====================================================================================\n");
    printf("||  LISTA DE FILMES                                               |  Pagina %d de %d *\n", currentPage + 1,
           totalPages);
    printf("=====================================================================================\n");
    printf("%-7s | %-50s | %-5s | %-4s | %-4s\n", "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("--------+----------------------------------------------------+--------+------+------\n");
}

void print_info_header() {
    read_title();
    printf("==================================================================================\n");
    printf("||  INFORMACOES DO SISTEMA                                                       *\n");
    printf("==================================================================================\n");
}

void print_title_search_header() {
    clear_screen();
    read_title();
    printf("==================================================================================\n");
    printf("|| Busca de titulos                                                             *\n");
    printf("==================================================================================\n");
    printf("Digite o termo de busca: ");
}

void print_menu_options() {
    printf("===================================================================================== \n");
    printf("[p] << Pagina anterior                                         [n] Proxima pagina >>  \n");
    printf("===================================================================================== \n");
    printf("[o] Ordenar por ano          [r] Ordenar por rating            [g] Filtrar por genero \n");
    printf("[s] Buscar registro                                                                   \n");
    printf("[i] Info | [q] Sair ================================================================= \n");
    printf("Comando: ");
}

void print_results_menu() {
    printf("=====================================================================================\n");
    printf("[p] << Pagina anterior                                          [n] Proxima pagina >> \n");
    printf("[q] Voltar ao menu ==================================================================\n");
    printf("Comando: ");
}

void print_genre_filter_header() {
    read_title();

    printf("=====================================================================================\n");
    printf("|| Filtro por Genero                                                                *\n");
    printf("=====================================================================================\n");

    int columns = 3;
    int rows = (GENRE_COUNT + columns - 1) / columns;

    // Largura de cada coluna (nome + número + bordas)
    const int colWidth = 30;

    for (int r = 0; r < rows; r++) {
        printf("|| ");

        for (int c = 0; c < columns; c++) {
            int index = r + c * rows;

            if (index < GENRE_COUNT) {
                printf("[%2d] %-19s", index + 1, GENRE_LIST[index]);
            } else {
                printf("%-*s", colWidth, ""); // coluna vazia
            }

            if (c < columns - 1)
                printf("  "); // espaço entre colunas
        }

        printf(" ||\n");
    }

    printf("==================================================================================\n");
    printf("Digite o numero do genero ou [q] para voltar: ");
}


void print_genre_results_header(const char *genre, int currentPage, int totalPages, double elapsedMs) {
    clear_screen();
    read_title();
    printf("=====================================================================================\n");
    printf("||  Genero: %-40s  Tempo: %.2f ms | Pag %d de %d *\n",
           genre, elapsedMs, currentPage + 1, totalPages);
    printf("=====================================================================================\n");
    printf("%-7s | %-50s | %-5s | %-4s | %-4s\n",
           "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("--------+----------------------------------------------------+--------+------+------\n");
}

void print_search_header(char *term, int currentPage, int totalPages, double elapsedMs) {
    clear_screen();
    read_title();
    printf("=====================================================================================\n");
    printf("||  Busca: %-44s Tempo: %.2f ms | Pag %d de %d * \n", term, elapsedMs, currentPage + 1, totalPages);
    printf("=====================================================================================\n");
    printf("%-7s | %-50s | %-5s | %-4s | %-4s\n",
           "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("--------+----------------------------------------------------+--------+------+------\n");
}

void print_order_year_header(double elapsedMs, int currentPage, int totalPages) {
    printf("=====================================================================================\n");
    printf("|| Ordenacao por Ano                               Tempo: %.2f ms | Pagina %d de %d *\n",
           elapsedMs, currentPage + 1, totalPages);
    printf("=====================================================================================\n");
    printf("%-7s | %-50s | %-5s | %-4s | %-4s\n",
       "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("--------+----------------------------------------------------+--------+------+------\n");
}

void print_order_rating_header(double elapsedMs, int currentPage, int totalPages) {
    printf("=====================================================================================\n");
    printf("|| Ordenacao por Rating                            Tempo: %.2f ms | Pagina %d de %d *\n",
           elapsedMs, currentPage + 1, totalPages);
    printf("=====================================================================================\n");
    printf("%-7s | %-50s | %-5s | %-4s | %-4s\n",
       "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("--------+----------------------------------------------------+--------+------+------\n");
}