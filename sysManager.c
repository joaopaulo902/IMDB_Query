#include "sysManager.h"
#include "entities.h"
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <windows.h>
#include "binService.h"
#include "bpTree.h"
#include "dbContext.h"
#include "IQuery.h"
#include "titleSearch.h"

BPTree *TREE_CTX = NULL;

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
                order_by_year(titles, totalMovies);
                // Show order options on menu
                continue;
            case 'f':
            case 'F':
                show_filter_page();
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

void print_search_header(char *term, int currentPage, int totalPages, double elapsedMs) {
    clear_screen();
    read_title();
    printf("==================================================================================\n");
    printf("||  Busca: %-41s Tempo: %.2f ms | Pag %d de %d * \n", term, elapsedMs, currentPage + 1, totalPages);
    printf("==================================================================================\n");
    printf("%-4s | %-50s | %-5s | %-4s | %-4s\n",
           "#", "Titulo", "Rating", "Ano", "Tipo");
    printf("-----+----------------------------------------------------+--------+------+------\n");
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

void print_menu_options() {
    printf("==================================================================================\n");
    printf("[f] Filtrar por ano | [s] Buscar registro | [o] Ordenar por ano | [i] Info\n");
    printf("[n] Proxima pagina  | [p] Pagina anterior                       | [q] Sair\n");
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

void show_search_page() {
    char searchTerm[100];

    clear_screen();
    read_title();
    printf("==================================================================================\n");
    printf("|| Busca de titulos                                                             *\n");
    printf("==================================================================================\n");
    printf("Digite o termo de busca: ");
    fgets(searchTerm, sizeof(searchTerm), stdin);

    size_t len = strlen(searchTerm);
    if (len > 0 && searchTerm[len - 1] == '\n')
        searchTerm[len - 1] = '\0';

    LARGE_INTEGER freq, begin, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&begin);

    int count;
    int* ids = search_term(searchTerm, &count);

    QueryPerformanceCounter(&end);

    double elapsedMs = (double)(end.QuadPart - begin.QuadPart) * 1000.0 / freq.QuadPart;

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

    printf("==================================================================================\n");
    printf("[n] Proxima pagina | [p] Pagina anterior | [q] Voltar ao menu\n");
    printf("Comando: ");
}

void show_filter_page() {
    clear_screen();
    read_title();
    printf("==================================================================================\n");
    printf("||  Filtro por Ano (B+ Tree)                                                   *\n");
    printf("==================================================================================\n");

    int ylo, yhi;
    printf("Ano minimo: ");
    scanf("%d", &ylo);
    printf("Ano maximo: ");
    scanf("%d", &yhi);

    // limpar \n restante do buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    // Cursor interno do B+ Tree
    int count = 0;
    int64_t leaf_off = -1;
    int pos = 0;

    // Carregar PRIMEIRA página de 10 IDs do índice
    int64_t *ids = bpt_range_query_page(TREE_CTX, ylo, yhi, &count, &leaf_off, &pos);

    if (count == 0) {
        printf("Nenhum registro encontrado no intervalo [%d, %d].\n", ylo, yhi);
        printf("Pressione ENTER para voltar...");
        getchar();
        clear_screen();
        return;
    }

    // Converte IDs em Titles
    Title *page = malloc(count * sizeof(Title));
    for (int i = 0; i < count; i++)
        page[i] = get_title_by_id(ids[i]);
    free(ids);

    int currentPage = 0;
    char buffer[16];

    while (1) {
        clear_screen();
        read_title();
        printf("==================================================================================\n");
        printf("||  Filtro Ano [%d - %d] — Página %d                                           *\n",
               ylo, yhi, currentPage + 1);
        printf("==================================================================================\n");
        printf("%-4s | %-50s | %-5s | %-4s | %-4s\n",
               "#", "Titulo", "Rating", "Ano", "Tipo");
        printf("-----+----------------------------------------------------+--------+------+------\n");

        int start = currentPage * PAGE_SIZE;
        int end = start + PAGE_SIZE;

        // Se precisamos de mais itens para preencher a UI
        while (end > count && leaf_off != -1) {
            // Buscar mais até preencher
            ids = bpt_range_query_page(TREE_CTX, ylo, yhi, &count, &leaf_off, &pos);
            if (count == 0) {
                // índice acabou
                leaf_off = -1;
                break;
            }

            // Carregar mais registros
            free(page);
            page = malloc(count * sizeof(Title));
            for (int i = 0; i < count; i++)
                page[i] = get_title_by_id(ids[i]);
            free(ids);
        }

        if (end > count) end = count;

        int printed = 0;
        for (int i = start; i < end; i++) {
            printf("%-4lld | %-50s |  %4.1f  | %-4d | %-10s\n",
                   (long long) page[i].id,
                   page[i].primaryTitle,
                   page[i].rating.aggregateRating / 100.0,
                   page[i].startYear,
                   page[i].type);
            printed++;
        }

        for (; printed < PAGE_SIZE; printed++)
            printf("%-4s | %-50s | %-6s | %-4s | %-10s\n", "", "", "", "", "");

        printf("==================================================================================\n");

        // Opções
        printf("[n] Próxima página");
        printf(" | [p] Página anterior");
        printf(" | [q] Voltar\n");
        printf("Comando: ");

        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        char cmd = buffer[0];

        switch (cmd) {
            case 'q':
            case 'Q':
                free(page);
                clear_screen();
                return;

            case 'p':
            case 'P':
                if (currentPage > 0)
                    currentPage--;
                break;

            case 'n':
            case 'N':
                // só avança se houver mais itens a exibir
                if ((currentPage + 1) * PAGE_SIZE < count || leaf_off != -1)
                    currentPage++;
                break;

            default:
                printf("Comando inválido.\n");
                Sleep(800);
        }
    }

    free(page);
    clear_screen();
}


void order_by_year(Title *titles, int totalMovies) {
    for (int i = 0; i < totalMovies - 1; i++) {
        for (int j = 0; j < totalMovies - i - 1; j++) {
            if (titles[j].startYear > titles[j + 1].startYear) {
                Title temp = titles[j];
                titles[j] = titles[j + 1];
                titles[j + 1] = temp;
            }
        }
    }
}
