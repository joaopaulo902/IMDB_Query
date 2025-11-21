#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "IQuery.h"

#define IMDB_QUERY_URL "https://api.imdbapi.dev/titles"
#define SEPARATOR "="

int main (void) {
    char* ptr = calloc(10, sizeof(char));
    scanf("%s", ptr);
    strtol(ptr, NULL, 10);
    free(ptr);

    return 0;
}