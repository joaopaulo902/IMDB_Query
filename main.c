#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "util.h"

#define IMDB_QUERY_URL "https://api.imdbapi.dev/titles"

int main(void) {
    CURL* curl = curl_easy_init();
    CURLcode res = 0;

    if (!curl) {
        printf("curl nao está funcionando corretamente");
        return 1;
    }

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;

    //operating principles
    /*
     * - set options
     * - perform actions
     * - clean up after yourself
    */

    curl_easy_setopt(curl, CURLOPT_URL, IMDB_QUERY_URL);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl);;
    if (res != CURLE_OK) {
        printf("curl erro %s\n", curl_easy_strerror(res));
        return 1;
    }
    FILE* fp = fopen("data.json", "w");

    printf("Size: %lu\n", (unsigned long)chunk.size);
    fprintf(fp, "Data:%s\n", chunk.memory);

    fclose(fp);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    return 0;
}