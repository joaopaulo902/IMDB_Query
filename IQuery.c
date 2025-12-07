//titles queries
// Created by joaop on 21/11/2025.
//

/*make request model:
* 1 - get info from titles and insert it into a binary file
* 2 - use the id that's being stored into the bin file and use it to get data from:
* titles\{titleId}\awardNominations
* titles\{titlesId}\episodes
* store each in a streamlined way from each url for every item stored in the titles binary file
*
*/

#include "IQuery.h"

#include <stdio.h>
#include <string.h>

#include "sysManager.h"

void query_titles_by_page(int currentPage, Title* page, int totalMovies) {
    FILE* fp = fopen("titles.bin", "rb");
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












