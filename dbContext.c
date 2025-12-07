//
// Created by joaop on 28/11/2025.
//
//abstraction of smaller methods

#include "dbContext.h"
#include "entities.h"
#include "apiHandler.h"
#include "binService.h"
#include "filterGenre.h"
#include <string.h>

#include "titleSearch.h"

#define MAX_DATA 200

void make_titles_full_request() {
    char url[1024] = {IMDB_QUERY_URL};
    int i = 0;
    FileHeader fH = {0};
    int rvalue = get_file_header(&fH, "titles.bin");
    if (rvalue != 0) {
        perror("error in get_file_header");
        return;
    }
    printf("%llu\n", fH.recordCount);
    do {
        printf("%d\n", i++);
        TitlesResponse* t = malloc(sizeof(TitlesResponse));

        if (get_info(url, "data.json") != 0) {
            printf("erro em get_info\n");
            break;
        }
        int pageCount =  get_page_title_item(t, "data.json");
        if (pageCount == -1) {
            perror("Failed to open data.json on reading");
            free_titles_response(t);
            break;
        }
        for (int j = 0; j < pageCount; j++) {
            Titles lastEntry = record_title_on_binary(t->titles[j], fH, j, "titles.bin");
            insert_genre_index(t->titles[j], lastEntry);
        }

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
        update_file_header(&fH, "titles.bin");

        free_titles_response(t);
    }while (fH.recordCount < MAX_DATA && fH.nextPageToken[0] != '\0');

    save_dictionary("vocabulary.bin", "postings.bin");
}


/*
 *streamlined logic for parsing awards from requests
 */
void parse_awards() {

}

int get_titles_count() {
    FileHeader fH = {0};
    int rvalue = get_file_header(&fH, "titles.bin");
    if (rvalue != 0) {
        perror("error in get_file_header");
        return -1;
    }
    return (int)fH.recordCount;
}