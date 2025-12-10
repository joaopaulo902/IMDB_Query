#include "dbContext.h"
#include "entities.h"
#include "apiHandler.h"
#include "binService.h"
#include "filterGenre.h"
#include <string.h>
#include "bpTree.h"
#include "titleSearch.h"

#define MAX_DATA 500000

void make_titles_full_request() {
    char url[1024] = {IMDB_QUERY_URL};
    int i = 0;
    FileHeader fH = {0};
    int rvalue = get_file_header(&fH, TITLES_BIN);

    if (rvalue != 0) {
        perror("error in get_file_header");
        return;
    }

    printf("%llu\n", fH.recordCount);

    if (fH.recordCount >= MAX_DATA) {
            fprintf(stderr ,"maximum record count reached");
            return;
    }
    printf("coletando dados...\n");
    BPTree *T = bpt_open(YEAR_INDEX_FILE);
    BPTree *R = bpt_open(RATING_INDEX_FILE);
    do {
        printf("%d ", i++);
        if (i != 0 && i % 20 == 0) {
            printf("\n");
        }
        TitlesResponse *t = malloc(sizeof(TitlesResponse));

        if (get_info(url, DATA_JSON_PATH) != 0) {
            printf("erro em get_info\n");
            break;
        }
        int pageCount = get_page_title_item(t, DATA_JSON_PATH);
        if (pageCount == -1) {
            perror("Failed to open data.json on reading");
            free_titles_response(t);
            break;
        }

        for (int j = 0; j < pageCount; j++) {
            Title lastEntry = record_title_on_binary(t->titles[j], fH, j, TITLES_BIN);
            bpt_insert(T, lastEntry.startYear, lastEntry.id, lastEntry.id);
            bpt_insert(R, lastEntry.rating.aggregateRating, lastEntry.id, lastEntry.id);
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
        update_file_header(&fH, TITLES_BIN);

        free_titles_response(t);
    } while (fH.recordCount < MAX_DATA && fH.nextPageToken[0] != '\0');
    bpt_close(T);
    bpt_close(R);

    save_dictionary(VOCABULARY_BIN_PATH, POSTINGS_BIN_PATH);
    printf("dados coletados ___('u')/___<===\n");
}

int get_titles_count() {
    FileHeader fH = {0};
    int rvalue = get_file_header(&fH, TITLES_BIN);
    if (rvalue != 0) {
        perror("error in get_file_header");
        return -1;
    }
    return (int) fH.recordCount;
}
