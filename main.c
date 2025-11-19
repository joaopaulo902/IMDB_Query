#include <stdio.h>
#include "systemManager.h"
#include <windows.h>

int main(void) {
    int isRunning = 1;
    //initializeSystem();



    Title titles[] = {
        {0, NULL, NULL, "Drama", 0, "The Shawshank Redemption", 1994, 1944},
        {1, NULL, NULL, "Crime", 0, "The Godfather", 1972, 1944},
        {2, NULL, NULL, "Action", 0, "The Dark Knight", 2008, 1944},
        {3, NULL, NULL, "Action", 1, "The Godfather", 1972, 1944},
        {4, NULL, NULL, "Drama", 0, "12 Angry Men", 1957, 1944},
        {5, NULL, NULL, "Biography", 0, "Schindler's List", 1993, 1944},
        {6, NULL, NULL, "Adventure", 0, "The Lord of the Rings: The Return of the King", 2003, 1944},
        {7, NULL, NULL, "Crime", 0, "Pulp Fiction", 1994, 1944},
        {8, NULL, NULL, "Western", 0, "The Good, the Bad and the Ugly", 1966, 1944},
        {9, NULL, NULL, "Drama", 0, "Fight Club", 1999, 1944},
        {10, NULL, NULL, "Drama", 0, "Forrest Gump", 1994, 1944},
        {11, NULL, NULL, "Sci-Fi", 0, "Inception", 2010, 1944},
        {12, NULL, NULL, "Sci-Fi", 0, "The Matrix", 1999, 1944},
        {13, NULL, NULL, "Biography", 0, "Goodfellas", 1990, 1944},
        {14, NULL, NULL, "Adventure", 0, "Seven Samurai", 1954, 1944},
        {15, NULL, NULL, "Crime", 0, "City of God", 2002, 1944},
        {16, NULL, NULL, "Crime", 0, "Se7en", 1995, 1944},
        {17, NULL, NULL, "Thriller", 0, "The Silence of the Lambs", 1991, 1944},
        {18, NULL, NULL, "Adventure", 0, "Interstellar", 2014, 1944},
        {19, NULL, NULL, "Thriller", 0, "Parasite", 2019, 1944},
        {20, NULL, NULL, "Animation", 0, "Spirited Away", 2001, 1944},
        {21, NULL, NULL, "Drama", 0, "The Green Mile", 1999, 1944},
        {22, NULL, NULL, "Comedy", 0, "Life Is Beautiful", 1997, 1944},
        {23, NULL, NULL, "Crime", 0, "Léon: The Professional", 1994, 1944}
    };

    int totalMovies = sizeof(titles) / sizeof(titles[0]);
    int totalPages = (totalMovies + PAGE_SIZE - 1) / PAGE_SIZE;

    int currentPage = 0;
    char buffer[32];

    while (1) {
        printPage(titles, totalMovies, currentPage);

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break; // erro de leitura
        }

        char cmd = buffer[0];

        if (cmd == 'q' || cmd == 'Q') {
            break;
        } else if (cmd == 'n' || cmd == 'N') {
            if (currentPage < totalPages - 1) {
                currentPage++;
            }
        } else if (cmd == 'p' || cmd == 'P') {
            if (currentPage > 0) {
                currentPage--;
            }
        } else if (cmd == 'i' || cmd == 'I') {
            show_info(titles, totalMovies);
        } else {
            printf("Comando desconhecido. Use n, p, i ou q.\n");

            Sleep(1000);
        }
    }

    clearScreen();
    printf("Encerrando...\n");

    return 0;
}
