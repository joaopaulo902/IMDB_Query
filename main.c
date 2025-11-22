#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "IQuery.h"

#define IMDB_QUERY_URL "https://api.imdbapi.dev/titles"
#define SEPARATOR "="

int main (void) {
    FILE* fpointer = fopen("data.json", "r");
    TitlesResponse* t = malloc(sizeof(TitlesResponse));
    int pageCount = get_page_item(fpointer, t);
    FILE* fp = fopen("binaryInfo.bin", "ab");
    record_on_binary(t->titles, pageCount , fp);
    fclose(fp);
    free_titles_response(t);

    return 0;
}

