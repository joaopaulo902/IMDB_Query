#include "sysManager.h"
#include "entities.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "dbContext.h"
#include "IQuery.h"

void read_title() {
    FILE *ptFile;
    ptFile = fopen("title.txt", "r");
    if (ptFile != NULL) {
        while (!feof(ptFile)) {
            char letter = fgetc(ptFile);
            printf("%c", letter);
        }
        printf("\n");
        fclose(ptFile);
    } else {
        printf("Erro abrindo imagem inicial.\n");
    }
}

void initialize_system() {

    Titles titles[PAGE_SIZE];

    // Calculate total movies and pages
    int totalMovies = get_titles_count();
    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    int currentPage = 0;
    char buffer[32];

    while (1) {
        char cmd = '\0';

        if (cmd == '\0' || cmd == 'l' || cmd == 'L') {
            clear_screen();
            print_titles_list(titles, totalMovies, currentPage);
            print_menu_options();
        }

        // Evaluate wrong inputs
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }

        cmd = buffer[0];

        switch (cmd) {
            case 'q':
            case 'Q':
                // Exit system
                break;
            case 'n':
            case 'N':
                // Next page
                if (currentPage < totalPages - 1) {
                    currentPage++;
                }
                cmd = 'l'; // Force list refresh
                continue;
            case 'p':
            case 'P':
                // Previous page
                if (currentPage > 0) {
                    currentPage--;
                }
                cmd = 'l'; // Force list refresh
                continue;
            case 'i':
            case 'I':
                // Show info
                show_info_page(totalMovies);
                continue;
            case 's':
            case 'S':
                // Show search page
                continue;
            case 'o':
            case 'O':
                order_by_year(titles, totalMovies);
                // Show order options on menu
                continue;
            default:
                printf("Comando desconhecido. Use n, p, i ou q.\n");
                Sleep(1000);
        }
    }

    clear_screen();
    // Show cursor before exiting
    printf("\033[?25h");
    printf("Encerrando...\n");
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
    printf("==================================================================================\n");
    printf("||  LISTA DE FILMES                                            |  Pagina %d de %d *\n", currentPage + 1,
           totalPages);
    printf("==================================================================================\n");
    printf("%-4s | %-50s | %-5s | %-4s | %-4s\n", "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("-----+----------------------------------------------------+--------+------+------\n");
}

void print_info_header() {
    read_title();
    printf("==================================================================================\n");
    printf("||  INFORMACOES DO SISTEMA                                                       *\n");
    printf("==================================================================================\n");
}

void print_titles_list(Titles *page, int totalMovies, int currentPage) {

    query_titles_by_page(currentPage, page, totalMovies);

    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    print_title_list_header(currentPage, totalPages);

    for (int i = 0; i < PAGE_SIZE; i++) {
        printf("%-4d | %-50s |  %4.1f  | %-4d | %-10s\n",
               (currentPage * PAGE_SIZE) + i + 1,
               page[i].primaryTitle,
               page[i].rating.aggregateRating,
               page[i].startYear,
               page[i].type);
    }
}

void print_menu_options() {
    printf("==================================================================================\n");
    printf("[s] Buscar registro      |  [o] Ordenar por ano        |  [i] Info\n");
    printf("[n] Proxima pagina       |  [p] Pagina anterior        |  [q] Sair\n");
    printf("Comando: ");
}

void show_info_page(int totalMovies) {

    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    clear_screen();
    print_info_header();

    printf("Total de filmes: %d\n", totalMovies);
    printf("Total de paginas: %d (tamanho da pagina: %d)\n", totalPages, PAGE_SIZE);
    printf("Pressione ENTER para continuar...");

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    clear_screen();
}

void order_by_year(Titles *titles, int totalMovies) {
    for (int i = 0; i < totalMovies - 1; i++) {
        for (int j = 0; j < totalMovies - i - 1; j++) {
            if (titles[j].startYear > titles[j + 1].startYear) {
                Titles temp = titles[j];
                titles[j] = titles[j + 1];
                titles[j + 1] = temp;
            }
        }
    }
}


