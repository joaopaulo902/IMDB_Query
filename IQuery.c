//
// Created by joaop on 21/11/2025.
//

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include <stdio.h>
#include "IQuery.h"
#include "util.h"




int get_info(char* url, FILE* fp) {
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
    char *temp = realloc(chunk.memory, chunk.size + 1);
    if (!temp) {
        free(chunk.memory);
        fprintf(stderr, "Out of memory\n");
        return 0; // or CURLE_WRITE_ERROR
    }
    chunk.memory = temp;
    chunk.memory[chunk.size] = '\0';
    fprintf(fp, "%s\n", chunk.memory);
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

void free_titles_response(TitlesResponse *r) {
    for (int i = 0; i < r->pageCount; i++) {
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
    free(r->token);
    free(r);
}

int get_page_item(FILE* fp, TitlesResponse *r) {
    char *buffer  = read_entire_file(fp);
    int pageCount = 0;
    fseek(fp, 0, SEEK_SET);
    //-----parse buffer--------------------
    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        const char *error = cJSON_GetErrorPtr();
        if (error) {
            printf("JSON Parse Error before: %s\n", error);
        }
        return 0;
    }
    cJSON *totalCount = cJSON_GetObjectItem(root, "totalCount");
    r->totalCount = totalCount ? totalCount->valueint : 0; //analyse cJSON* totalCount and write valueInt or 0
    cJSON *nextPageToken = cJSON_GetObjectItem(root, "nextPageToken");
    if (!nextPageToken) {
        r->token = NULL;
    }
    else {
        r->token = json_strdup(nextPageToken);
    }

    cJSON *titles = cJSON_GetObjectItem(root, "titles");
    if (!titles || !cJSON_IsArray(titles)) {
        cJSON_Delete(root);
        free(buffer);
        return 0;
    }
    pageCount = cJSON_GetArraySize(titles);
    r->pageCount = pageCount;
    r->titles = malloc(sizeof(Title) * pageCount);
    for (int i = 0; i < pageCount; i++) {
        r->titles[i] = parse_title(cJSON_GetArrayItem(titles, i));
    }
    cJSON_Delete(root);
    free(buffer);
    return pageCount;
}

char *read_entire_file(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buf = malloc(size + 1);
    if (!buf) return NULL;

    size_t n = fread(buf, 1, size, fp);
    buf[n] = '\0';
    return buf;
}

void record_title_on_binary(const Title* titlesArray, int pageCount, FILE* fp) {
    char id[64] = {0};
    char type[32] = {0};
    char primaryTitle[64] = {0};
    char originalTitle[64] = {0};
    //double Rating;
    //long int voteCount;
    char plot[1024] = {0};
    //int startYear;
    //int runtimeSeconds;
    for (int i =0; i < pageCount; i++) {
        //copy data into fixed size strings
        strncpy(id, titlesArray[i].id, sizeof(id) - 1);
        strncpy(type, titlesArray[i].type, sizeof(type) - 1);
        strncpy(primaryTitle, titlesArray[i].primaryTitle, sizeof(primaryTitle) - 1);
        strncpy(originalTitle, titlesArray[i].originalTitle, sizeof(originalTitle) - 1);
        if (!titlesArray[i].plot) {
            printf("NULL plot detected at index %d\n", i);
        }
        if (titlesArray[i].plot) {
            strncpy(plot, titlesArray[i].plot, sizeof(plot) - 1);
            plot[sizeof(plot) - 1] = '\0';
        } else {
            plot[0] = '\0';  // safe empty string
        }

        //truncate end of data by substituting it with '\0' in case of overflow
        id[sizeof(id) - 1] = '\0';
        type[sizeof(type) - 1] = '\0';
        primaryTitle[sizeof(primaryTitle) - 1] = '\0';
        originalTitle[sizeof(originalTitle) - 1] = '\0';

        //write data into file
        fwrite(id, sizeof(char), sizeof(id), fp);
        fwrite(primaryTitle, sizeof(char), sizeof(primaryTitle), fp);
        fwrite(originalTitle, sizeof(char), sizeof(originalTitle), fp);
        fwrite(type, sizeof(char), sizeof(type), fp);
        fwrite(plot, sizeof(char), sizeof(plot), fp);
        fwrite(&titlesArray[i].rating.IMDBrating, sizeof(double), 1, fp);
        fwrite(&titlesArray[i].rating.voteCount, sizeof(long int), 1, fp);
        fwrite(&titlesArray[i].startYear, sizeof(int), 1, fp);
        fwrite(&titlesArray[i].runtimeSeconds, sizeof(int), 1, fp);
    }
}

void make_titles_full_request() {
    char url[1024] = {IMDB_QUERY_URL};
    int i = 0;
    FileHeader fH = {0};

    FILE* binFp = fopen("titlesBinaryInfo.bin", "rb+");
    if (!binFp)
        binFp = fopen("titlesBinaryInfo.bin", "wb+");

    fseek(binFp, 0, SEEK_SET);

    if (is_file_empty(binFp)) {
        fH.ID = FILE_ID;
        fH.version = 1;
        fH.recordCount = 0;
        fH.nextPageToken[0] = '\0';

        fwrite(&fH, sizeof(FileHeader), 1, binFp);
        fseek(binFp, sizeof(FileHeader), SEEK_SET);
    }
    else {
        if (fread(&fH, sizeof(FileHeader), 1, binFp) != 1) {
            perror("Failed to read header");
            fclose(binFp);
            return;
        }

        if (fH.ID != FILE_ID) {
            printf("Invalid file format\n");
            fclose(binFp);
            return;
        }

        if (fH.nextPageToken[0] != '\0') {
            snprintf(url, sizeof(url), "%s?pageToken=%s", IMDB_QUERY_URL, fH.nextPageToken);
            fseek(binFp, (long) (sizeof(FileHeader) + sizeof(TitleDisk) * fH.recordCount), SEEK_SET );
        }
    }
    printf("%llu\n", fH.recordCount);
    do {
        printf("%d\n", i++);
        TitlesResponse* t = malloc(sizeof(TitlesResponse));
        FILE* filePointer = fopen("data.json", "w");
        if (!filePointer) {
            perror("Failed to open data.json on writing");
            free_titles_response(t);
            break;
        }
        get_info(url, filePointer);
        fclose(filePointer);
        FILE* jsonFilePointer = fopen("data.json", "r");
        if (!jsonFilePointer) {
            perror("Failed to open data.json on reading");
            free_titles_response(t);
            break;
        }
        int pageCount = get_page_item(jsonFilePointer, t);
        fclose(jsonFilePointer);
        record_title_on_binary(t->titles, pageCount , binFp);
        fH.recordCount += pageCount;
        if (t->token != NULL && strlen(t->token) > 0) {
            snprintf(url, sizeof(url), "%s?pageToken=%s", IMDB_QUERY_URL, t->token);
            strncpy(fH.nextPageToken, t->token, sizeof(fH.nextPageToken) - 1);
            fH.nextPageToken[sizeof(fH.nextPageToken) - 1] = '\0';
        } else {
            snprintf(url, sizeof(url), "%s", IMDB_QUERY_URL);
            fH.nextPageToken[0] = '\0';
            free_titles_response(t);
            break;
        }
        fseek(binFp, 0, SEEK_SET);
        fwrite(&fH, sizeof(FileHeader), 1, binFp);
        fseek(binFp, 0, SEEK_END);
        free_titles_response(t);
    }while (fH.recordCount < 2378285 && fH.nextPageToken[0] != '\0');
    fclose(binFp);
}

int is_file_empty(FILE *fp) {
    fseek(fp, 0, SEEK_END);      // go to end
    long size = ftell(fp);       // get position (file size)
    rewind(fp);                  // return to start
    return size == 0;
}