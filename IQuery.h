//
// Created by joaop on 21/11/2025.
//

#ifndef IMDB_QUERY_IQUERY_H
#define IMDB_QUERY_IQUERY_H

#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdlib.h>
#include "IQuery.h"
#include "util.h"

#define IMDB_QUERY_URL "https://api.imdbapi.dev/titles"
#define FILE_ID 0x42444d49

typedef struct {
    uint32_t ID;        // File identifier
    uint32_t version;      // Format version
    uint64_t recordCount;  // How many titles exist
    char nextPageToken[1024]; //what is the next page's token if processing is halted
} FileHeader;

typedef struct {
    int width;
    int height;
    char* href;
}ImageSpecifics;

typedef struct {
    double IMDBrating;
    long int voteCount;
}Rating;

/**
 * Struct for storing a @code Title's@endcode info
 * @code
    char *id;
    char *type;
    char *primaryTitle;
    char *originalTitle;

    ImageSpecifics image;
    int startYear;
    int runtimeSeconds;

    char **genres;
    int genres_count;

    Rating rating;

    char *plot;
 * @endcode
 */
typedef struct {
    char *id;
    char *type;
    char *primaryTitle;
    char *originalTitle;

    ImageSpecifics image;
    int startYear;
    int runtimeSeconds;

    char **genres;
    int genres_count;

    Rating rating;

    char *plot;
} Title;

/**
 * Struct for storing a page of the api's response
 */
typedef struct {
    Title *titles;
    long int pageCount;
    long int totalCount;
    char* token;
}TitlesResponse;

/**
 * Struct of definite size used for storing data in binary file
 * Advantages of definitive allocated size: fast indexing, easy to use
 *
 */
typedef struct {
    uint32_t primaryKey;
    char id[32];
    char type[32];
    char primaryTitle[128];
    char originalTitle[128];
    char plot[512];

    struct {
        char href[256];
        int width;
        int height;
    } image;

    struct {
        double IMDBrating;
        int voteCount;
    } rating;

    char genres[10][32];
    int genres_count;
} TitleDisk;



/**
 * @param url
 * @param fp file pointer
 * @return false - if ok \n
 * @return true - if error has occurred \n\n
 * gets info from api referenced by the @code url@endcode and inserts it into the desired @code fileName@endcode
 *
 *- appends every request to the end of the file
*/
int get_info(char* url, FILE* fp);


/**
 * @param obj
 * @return json string item
 *
 * if string object is a string and not NULL, returns the string object
 */
static char* json_strdup(const cJSON* obj);

/**
 *
 * @param fp json file pointer
 * @param r TitleResponse struct for storing data
 * @return number of titles in the page
 */
int get_page_item(FILE* fp, TitlesResponse *r);
/**
* @param item pointer to an item
* @return t individual title struct\n
* parse @code item@endcode  from the json file individually
*/
Title parse_title(const cJSON *item);

/**
 *
 * @param r struct that contains an array of titles and it's count
 *
 * frees allocated struct @code TitlesResponse@endcode
 */
void free_titles_response(TitlesResponse *r);

/**
 *
 * @param titlesArray allocated array that stores the api's pages titles
 * @param pageCount count of the titles in the api's page response
 * @param fp binary file pointer
 */
void record_title_on_binary(const Title* titlesArray, int pageCount, FILE* fp);

/**
 *
 * @param fp pointer to file
 * @return pointer to file buffer
 */
char *read_entire_file(FILE *fp);

/**
 * Reach out to the api and request full access to the titles database\n\n
 * Destination archive: binaryInfo.bin.
 * At the file header, there is information about stuff like: file identifier, file version, record count and next
 * url token, so the api's request can be resumed at any time.
 */
void make_titles_full_request();

/**
 *
 * @param fp file pointer
 * @return int that says if file is empty
 * checks if file is empty after opening it
 */
int is_file_empty(FILE *fp);
#endif //IMDB_QUERY_IQUERY_H

