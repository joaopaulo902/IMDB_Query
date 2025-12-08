#include "sysManager.h"
#include "entities.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <windows.h>
#include "binService.h"
#include "bpTree.h"
#include "dbContext.h"
#include "filterGenre.h"
#include "IQuery.h"
#include "titleSearch.h"
#include "view.h"

BPTree *TREE_CTX = NULL;

void initialize_system() {
    TREE_CTX = bpt_open(YEAR_INDEX_FILE);

    Title titles[PAGE_SIZE];

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
                show_search_page();
                continue;
            case 'o':
            case 'O':
                //order_by_year(titles, totalMovies);
                // Show order options on menu
                continue;
            case 'g':
            case 'G':
                show_genre_filter_page();
                continue;

            case 'f':
            case 'F':
                //show_filter_page();
                continue;

            default:
                printf("Comando desconhecido. Use n, p, i ou q.\n");
                Sleep(1000);
        }
    }

    bpt_close(TREE_CTX);

    clear_screen();
    // Show cursor before exiting
    printf("\033[?25h");
    printf("Encerrando...\n");
}

void print_titles_list(Title *page, int totalMovies, int currentPage) {
    query_titles_by_page(currentPage, page, totalMovies);

    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    print_title_list_header(currentPage, totalPages);

    for (int i = 0; i < PAGE_SIZE; i++) {
        printf("%-4d | %-50s |  %4.1f  | %-4d | %-10s\n",
               (currentPage * PAGE_SIZE) + i + 1,
               page[i].primaryTitle,
               page[i].rating.aggregateRating / 100.0,
               page[i].startYear,
               page[i].type);
    }
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

void show_search_page() {
    char searchTerm[100];

    print_title_search_header();

    fgets(searchTerm, sizeof(searchTerm), stdin);

    size_t len = strlen(searchTerm);
    if (len > 0 && searchTerm[len - 1] == '\n')
        searchTerm[len - 1] = '\0';

    LARGE_INTEGER freq, begin, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&begin);

    int count;
    int *ids = search_term(searchTerm, &count);

    QueryPerformanceCounter(&end);

    double elapsedMs = (double) (end.QuadPart - begin.QuadPart) * 1000.0 / freq.QuadPart;

    if (!ids || count == 0) {
        printf("Nenhum titulo encontrado.\n");
        printf("Pressione ENTER para voltar...");
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        clear_screen();
        return;
    }

    Title *results = malloc(count * sizeof(Title));
    for (int i = 0; i < count; i++)
        results[i] = get_title_by_id(ids[i]);

    free(ids);

    int currentPage = 0;
    char buffer[16];

    while (1) {
        print_search_page_results(results, count, searchTerm, currentPage, elapsedMs);

        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        char cmd = buffer[0];

        switch (cmd) {
            case 'n':
            case 'N':
                if ((currentPage + 1) * PAGE_SIZE < count)
                    currentPage++;
                break;

            case 'p':
            case 'P':
                if (currentPage > 0)
                    currentPage--;
                break;

            case 'q':
            case 'Q':
                free(results);
                clear_screen();
                return;

            default:
                printf("Comando invalido.\n");
                Sleep(800);
        }
    }

    free(results);
    clear_screen();
}

void print_search_page_results(Title *results, int count, char *term, int currentPage, double elapsedMs) {
    int totalPages = (count + PAGE_SIZE - 1) / PAGE_SIZE;

    print_search_header(term, currentPage, totalPages, elapsedMs);

    int start = currentPage * PAGE_SIZE;
    int end = start + PAGE_SIZE;
    if (end > count) end = count;

    int printed = 0;

    for (int i = start; i < end; i++) {
        printf("%-4d | %-50s |  %4.1f  | %-4d | %-10s\n",
               results[i].id,
               results[i].primaryTitle,
               results[i].rating.aggregateRating / 100.0,
               results[i].startYear,
               results[i].type);
        printed++;
    }

    for (; printed < PAGE_SIZE; printed++) {
        printf("%-4s | %-50s | %-6s | %-4s | %-10s\n", "", "", "", "", "");
    }

    print_results_menu();
}

void show_genre_filter_page() {
    clear_screen();
    print_genre_filter_header();

    char buffer[32];
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    if (buffer[0] == 'q' || buffer[0] == 'Q') return;

    int choice = atoi(buffer);
    if (choice < 1 || choice > GENRE_COUNT) {
        printf("Opção inválida!\n");
        Sleep(800);
        clear_screen();
        return;
    }

    const char *genre = GENRE_LIST[choice - 1];

    LARGE_INTEGER freq, begin, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&begin);

    Title *results = NULL;
    int count = filter_genre_raw(genre, &results);

    QueryPerformanceCounter(&end);

    double elapsedMs =
        (double)(end.QuadPart - begin.QuadPart) * 1000.0 / freq.QuadPart;

    if (count == 0 || results == NULL) {
        printf("Nenhum registro encontrado para %s.\n", genre);
        printf("Pressione ENTER para voltar...");
        getchar();
        clear_screen();
        return;
    }

    int currentPage = 0;
    char cmdBuff[16];

    while (1) {
        clear_screen();
        read_title();

        int totalPages = (count + PAGE_SIZE - 1) / PAGE_SIZE;
        print_genre_results_header(genre, currentPage, totalPages, elapsedMs);

        int start = currentPage * PAGE_SIZE;
        int end = start + PAGE_SIZE;
        if (end > count) end = count;

        int printed = 0;
        for (int i = start; i < end; i++) {
            printf("%-4d | %-50s | %4.1f | %-4d | %-10s\n",
                   results[i].id,
                   results[i].primaryTitle,
                   results[i].rating.aggregateRating / 100.0,
                   results[i].startYear,
                   results[i].type);
            printed++;
        }

        for (; printed < PAGE_SIZE; printed++)
            printf("%-4s | %-50s | %-6s | %-4s | %-10s\n",
                   "", "", "", "", "");

        print_results_menu();

        if (!fgets(cmdBuff, sizeof(cmdBuff), stdin)) break;

        switch (cmdBuff[0]) {
            case 'q':
            case 'Q':
                free(results);
                clear_screen();
                return;

            case 'n':
            case 'N':
                if (currentPage + 1 < totalPages)
                    currentPage++;
                break;

            case 'p':
            case 'P':
                if (currentPage > 0)
                    currentPage--;
                break;

            default:
                printf("Comando inválido.\n");
                Sleep(800);
        }
    }

    free(results);
    clear_screen();
}

