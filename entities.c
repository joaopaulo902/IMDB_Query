#include "entities.h"
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <string.h>
#include "binService.h"
#include "util.h"

int get_page_title_item(TitlesResponse *r, char fileName[]) {
    int pageCount = 0;
    FILE* jsonFilePointer = fopen(fileName, "r");

    if (!jsonFilePointer) {
        return -1;
    }

    char *buffer  = read_entire_file(jsonFilePointer);

    cJSON *root = cJSON_Parse(buffer);
    if (root == NULL) {
        const char *error = cJSON_GetErrorPtr();
        if (error) {
            printf("JSON Parse Error before: %s\n", error);
        }
        free(buffer);
        fclose(jsonFilePointer);
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
    r->titles = malloc(sizeof(ParseTitle) * pageCount);
    for (int i = 0; i < pageCount; i++) {
        r->titles[i] = parse_title(cJSON_GetArrayItem(titles, i));
    }
    cJSON_Delete(root);
    free(buffer);
    fclose(jsonFilePointer);
    return pageCount;
}

ParseTitle parse_title(const cJSON *item) {
    ParseTitle t = {0};

    t.id = json_strdup(cJSON_GetObjectItem(item, "id"));
    t.type = json_strdup(cJSON_GetObjectItem(item, "type"));
    t.primaryTitle = json_strdup(cJSON_GetObjectItem(item, "primaryTitle"));
    t.originalTitle = json_strdup(cJSON_GetObjectItem(item, "originalTitle"));
    t.plot = json_strdup(cJSON_GetObjectItem(item, "plot"));

    cJSON *startYear = cJSON_GetObjectItem(item, "startYear");
    if (cJSON_IsNumber(startYear)) t.startYear = startYear->valueint;

    cJSON *runtime = cJSON_GetObjectItem(item, "runtimeSeconds");
    if (cJSON_IsNumber(runtime)) t.runtimeSeconds = runtime->valueint;

    cJSON *rating = cJSON_GetObjectItem(item, "rating");
    if (cJSON_IsObject(rating)) {
        t.rating.aggregateRating = cJSON_GetObjectItem(rating, "aggregateRating")->valuedouble;
        t.rating.voteCount = cJSON_GetObjectItem(rating, "voteCount")->valueint;
    }

    cJSON *genres = cJSON_GetObjectItem(item, "genres");
    if (cJSON_IsArray(genres)) {
        t.genresCount = cJSON_GetArraySize(genres);
        t.genres = malloc(sizeof(char*) * t.genresCount);
        for (int i = 0; i < t.genresCount; i++) {
            cJSON *g = cJSON_GetArrayItem(genres, i);
            t.genres[i] = json_strdup(g);
        }
    }

    return t;
}

char* json_strdup(const cJSON* obj) {
    return (obj && cJSON_IsString(obj)) ? strdup(obj->valuestring) : NULL;
}

void free_titles_response(TitlesResponse *r) {
    for (int i = 0; i < r->pageCount; i++) {
        ParseTitle *t = &r->titles[i];

        free(t->id);
        free(t->type);
        free(t->primaryTitle);
        free(t->originalTitle);
        free(t->plot);

        for (int g = 0; g < t->genresCount; g++)
            free(t->genres[g]);

        free(t->genres);
    }
    free(r->titles);
    free(r->token);
    free(r);
}

Title record_title_on_binary(ParseTitle title, FileHeader fHeader, int indexInPage, char fileName[]) {
    FILE* fp = fopen(fileName, "rb+");
    Title entry = {0};
    if (!fp) {
        perror("Erro abrindo titles.bin");
        return entry;
    }

    // Ler header atual
    fseek(fp, 0, SEEK_SET);
    fread(&fHeader, sizeof(FileHeader), 1, fp);

    entry.id = fHeader.recordCount + indexInPage + 1;

    // Indexar o título (ESSENCIAL)
    add_title_name(title.originalTitle, entry.id);

    // Calcular offset da escrita do título
    off_t offset = sizeof(FileHeader) + sizeof(Title) * entry.id;
    _fseeki64(fp, offset - sizeof(Title), SEEK_SET);

    // Gravar título no arquivo
    put_title(&entry, title, fp);

    fclose(fp);
    return entry;
}

int get_file_header(FileHeader* fH, char fileName[]) {
    FILE* binFp = fopen(fileName, "rb");
    if (!binFp)
        binFp = fopen(fileName, "wb+");

    fseek(binFp, 0, SEEK_SET);

    if (is_file_empty(binFp)) {
        fH->ID = TITLE_FILE_ID;
        fH->version = 1;
        fH->recordCount = 0;
        fH->nextPageToken[0] = '\0';

        fwrite(fH, sizeof(FileHeader), 1, binFp);
        fseek(binFp, sizeof(FileHeader), SEEK_SET);
    }
    else if (fread(fH, sizeof(FileHeader), 1, binFp) != 1) {
            perror("Failed to read header");
            fclose(binFp);
            return -1;
    }
    if (fH->ID != TITLE_FILE_ID) {
        printf("Invalid file format\n");
        fclose(binFp);
        return -1;
    }
    fclose(binFp);
    return 0;
}