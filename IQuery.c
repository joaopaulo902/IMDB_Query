//
// Created by joaop on 21/11/2025.
//

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include "IQuery.h"
#include "util.h"




int get_info(char* url, const char* fileName) {
    CURL* curl = curl_easy_init();
    CURLcode res = 0;
    //operating principles
    /*
     * - set options
     * - perform actions
     * - clean up after yourself
    */

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
    FILE* fp = fopen(fileName, "a");

    printf("Size: %lu\n", (unsigned long)chunk.size);
    fprintf(fp, "Data:%s\n", chunk.memory);

    fclose(fp);
    curl_easy_cleanup(curl);
    free(chunk.memory);

    return 0;
}


static char* json_strdup(const cJSON* obj) {
    return (obj && cJSON_IsString(obj)) ? strdup(obj->valuestring) : NULL;
}

Title parse_title(const cJSON *item) {
    Title t = {0};

    t.id = json_strdup(cJSON_GetObjectItem(item, "id"));
    t.type = json_strdup(cJSON_GetObjectItem(item, "type"));
    t.primaryTitle = json_strdup(cJSON_GetObjectItem(item, "primaryTitle"));
    t.originalTitle = json_strdup(cJSON_GetObjectItem(item, "originalTitle"));
    t.plot = json_strdup(cJSON_GetObjectItem(item, "plot"));

    cJSON *startYear = cJSON_GetObjectItem(item, "startYear");
    if (cJSON_IsNumber(startYear)) t.startYear = startYear->valueint;

    cJSON *runtime = cJSON_GetObjectItem(item, "runtimeSeconds");
    if (cJSON_IsNumber(runtime)) t.runtimeSeconds = runtime->valueint;

    // ---- primaryImage ----
    cJSON *image = cJSON_GetObjectItem(item, "primaryImage");
    if (cJSON_IsObject(image)) {
        t.image.href = json_strdup(cJSON_GetObjectItem(image, "url"));
        t.image.width = cJSON_GetObjectItem(image, "width")->valueint;
        t.image.height = cJSON_GetObjectItem(image, "height")->valueint;
    }

    // ---- rating ----
    cJSON *rating = cJSON_GetObjectItem(item, "rating");
    if (cJSON_IsObject(rating)) {
        t.rating.IMDBrating = cJSON_GetObjectItem(rating, "aggregateRating")->valuedouble;
        t.rating.voteCount = cJSON_GetObjectItem(rating, "voteCount")->valueint;
    }

    // ---- genres array ----
    cJSON *genres = cJSON_GetObjectItem(item, "genres");
    if (cJSON_IsArray(genres)) {
        t.genres_count = cJSON_GetArraySize(genres);
        t.genres = malloc(sizeof(char*) * t.genres_count);
        for (int i = 0; i < t.genres_count; i++) {
            cJSON *g = cJSON_GetArrayItem(genres, i);
            t.genres[i] = json_strdup(g);
        }
    }

    return t;
}

void free_titles(const TitlesResponse *r) {
    for (int i = 0; i < r->titlesCount; i++) {
        Title *t = &r->titles[i];

        free(t->id);
        free(t->type);
        free(t->primaryTitle);
        free(t->originalTitle);
        free(t->plot);
        free(t->image.href);

        for (int g = 0; g < t->genres_count; g++)
            free(t->genres[g]);

        free(t->genres);
    }

    free(r->titles);
}

void get_page_item(FILE* fp, TitlesResponse *r) {
    char buffer[32768];
    int pos = 0;
    int pageCount = 0;
    fseek(fp, 0, SEEK_SET);
    //-----read entire json file-----------
    while (!feof(fp)) {
        buffer[pos] = (char) fgetc(fp);
        pos++;
    }
    buffer[pos] = '\0';
    //-----parse buffer--------------------
    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        return;
    }
    cJSON *totalCount = cJSON_GetObjectItem(root, "totalCount");
    r->totalCount = totalCount->valueint;
    cJSON *nextPageToken = cJSON_GetObjectItem(root, "nextPageToken");
    if (!nextPageToken) {
        r->token = NULL;
    }
    r->token = json_strdup(nextPageToken);
    cJSON *titles = cJSON_GetObjectItem(root, "titles");
    pageCount = cJSON_GetArraySize(titles);
    r->titles = malloc(sizeof(Title) * pageCount);
    for (int i = 0; i < pageCount; i++) {
        r->titles[i] = parse_title(cJSON_GetArrayItem(titles, i));
    }
}

void record_on_binary() {

}
