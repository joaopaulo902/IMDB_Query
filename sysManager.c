#include "sysManager.h"
#include "entities.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

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
    Titles titles[] = {
        {
            1, "tt0000001", "short", "Carmencita", "Bom filme, começa e termina",
            1909, 0, {0}, (time_t) (-1),
            (time_t) (-1), -1
        },
        {
            2, "tt0000002", "short", "Le clown et ses chiens", "Um palhaço entretém cães",
            1892, 0, {0}, (time_t) (-1),
            (time_t) (-1), -1
        },
        {
            3, "tt0000003", "short", "Pauvre Pierrot", "Pierrot é enganado por Colombina",
            1892, 0, {0}, (time_t) (-1),
            (time_t) (-1), -1
        },
        {
            4, "tt0000004", "short", "Un bon bock", "Homem bebe cerveja em um bar",
            1892, 0, {0}, (time_t) (-1),
            (time_t) (-1), -1
        },
        {
            5, "tt0000005", "short", "Blacksmith Scene", "Cena de ferreiro em ação",
            1893, 0, {0}, (time_t) (-1),
            (time_t) (-1), -1
        },
        {
            6, "tt0000006", "short", "Chinese Opium Den", "Cena em uma casa de ópio chinesa", 1894, 0, {0},
            (time_t) (-1), (time_t) (-1), -1
        },
        {
            7, "tt0000007", "short", "Corbett and Courtney Before the Kinetograph", "Luta entre Corbett e Courtney",
            1894, 0, {0}, (time_t) (-1), (time_t) (-1), -1
        },
        {
            8, "tt0000008", "short", "Edison Kinetoscopic Record of a Sneeze", "Registro de um espirro por Edison",
            1894, 0, {0}, (time_t) (-1), (time_t) (-1), -1
        },
        {
            9, "tt0000009", "short", "Miss Jerry", "Primeiro filme narrativo americano completo", 1894, 0, {0},
            (time_t) (-1), (time_t) (-1), -1
        },
        {
            10, "tt0000010", "short", "The Execution of Mary Stuart", "Execução de Maria Stuart encenada", 1895, 0, {0},
            (time_t) (-1), (time_t) (-1), -1
        }
    };

    // Calculate total movies and pages
    int totalMovies = sizeof(titles) / sizeof(titles[0]);
    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    int currentPage = 0;
    char buffer[32];

    while (1) {
        char cmd = '\0';

        if (cmd == '\0' || cmd == 'l' || cmd == 'L') {
            clearScreen();
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
                show_info(titles, totalMovies);
                continue;
            default:
                printf("Comando desconhecido. Use n, p, i ou q.\n");
                Sleep(1000);
        }
    }

    clearScreen();
    // Show cursor before exiting
    printf("\033[?25h");
    printf("Encerrando...\n");
}

void clearScreen() {
    // Hide cursor
    printf("\033[?25l");
    // Clear screen
    printf("\033[2J");
    // Move cursor to top-left
    printf("\033[H");
    // Show cursor
    fflush(stdout);
}

void printTitleListHeader(int currentPage, int totalPages) {
    read_title();
    printf("==================================================================================\n");
    printf("||  LISTA DE FILMES                                            |  Pagina %d de %d *\n", currentPage + 1,
           totalPages);
    printf("==================================================================================\n");
    printf("%-4s | %-50s | %-5s | %-4s | %-4s\n", "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("-----+----------------------------------------------------+--------+------+------\n");
}

void print_titles_list(Titles *titles, int totalMovies, int currentPage) {
    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;
    int start = currentPage * PAGE_SIZE;
    int end = start + PAGE_SIZE;
    if (end > totalMovies) end = totalMovies;

    printTitleListHeader(currentPage, totalPages);

    for (int i = start; i < end; i++) {
        printf("%-4d | %-50s |  %4.1f  | %-4d | %-10s\n",
               i + 1,
               titles[i].primaryTitle,
               titles[i].rating.aggregateRating,
               titles[i].startYear,
               titles[i].type);
    }
}

void print_menu_options() {
    printf("==================================================================================\n");
    printf("[s] Buscar registro      |  [o] Ordenar                |  [i] Info\n");
    printf("[n] Proxima pagina       |  [p] Pagina anterior        |  [q] Sair\n");
    printf("Comando: ");
}

void show_info(Titles *movies, int totalMovies) {
    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    clearScreen();
    printf("\n--- Informacoes gerais ---\n");
    printf("Total de filmes: %d\n", totalMovies);
    printf("Total de paginas: %d (tamanho da pagina: %d)\n", totalPages, PAGE_SIZE);
    printf("Pressione ENTER para continuar...");

    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    clearScreen();
}

