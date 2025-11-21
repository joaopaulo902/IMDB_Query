#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include "IQuery.h"

#define IMDB_QUERY_URL "https://api.imdbapi.dev/titles"

int main (void) {
    get_info(IMDB_QUERY_URL, "data.json");


    return 0;
}