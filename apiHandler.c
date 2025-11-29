//
// Created by joaop on 28/11/2025.
//

#include "apiHandler.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdlib.h>
#include "util.h"



int get_info(char* url, char fileName[]) {
    CURL* curl = curl_easy_init();
    CURLcode res = 0;
    //operating principles
    /*
     * - set options
     * - perform actions
     * - clean up after yourself
    */
    FILE* filePointer = fopen(fileName, "w");
    if (!filePointer) {
        perror("Failed to open data.json on writing");
        return 2;
    }

    if (!curl) {
        printf("curl nao está funcionando corretamente");
        return 1;
    }

    struct MemoryStruct chunk;
    chunk.memory = malloc(1);
    chunk.size = 0;



    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) &chunk);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl);;
    if (res != CURLE_OK) {
        printf("curl erro %s\n", curl_easy_strerror(res));
        return 2;
    }
    char *temp = realloc(chunk.memory, chunk.size + 1);
    if (!temp) {
        free(chunk.memory);
        fprintf(stderr, "Out of memory\n");
        return 2; // or CURL_WRITE_ERROR
    }
    chunk.memory = temp;
    chunk.memory[chunk.size] = '\0';
    fprintf(filePointer, "%s\n", chunk.memory);
    curl_easy_cleanup(curl);
    free(chunk.memory);
    fclose(filePointer);

    return 0;
}

