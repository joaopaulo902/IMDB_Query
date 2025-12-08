#include "IQuery.h"
#include <stdio.h>
#include <string.h>
#include "sysManager.h"

void query_titles_by_page(int currentPage, Title *page, int totalMovies) {
    FILE *fp = fopen("titles.bin", "rb");
    if (!fp) {
        perror("Failed to open titles.bin");

        memset(page, 0, PAGE_SIZE * sizeof(Title));
        return;
    }

    memset(page, 0, PAGE_SIZE * sizeof(Title));

    int start = currentPage * PAGE_SIZE;
    int remaining = totalMovies - start;
    int toRead = remaining < PAGE_SIZE ? remaining : PAGE_SIZE;

    long long offset = (long long) sizeof(FileHeader) + (long long) start * (long long) sizeof(Title);
    _fseeki64(fp, offset, SEEK_SET);

    fread(page, sizeof(Title), toRead, fp);

    fclose(fp);
}