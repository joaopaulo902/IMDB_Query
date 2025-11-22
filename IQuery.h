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
    long int titlesCount;
    long int totalCount;
    char* token;
}TitlesResponse;



/**
 * @param url
 * @param fileName
 * @return false - if ok \n
 * @return true - if error has occurred \n\n
 * gets info from api referenced by the @code url@endcode and inserts it into the desired @code fileName@endcode
 *
 *- appends every request to the end of the file
*/
int Get_Info(char* url, const char* fileName);


/**
 * @param obj
 * @return json string item
 *
 * if string object is a string and not NULL, returns the string object
 */
static char* json_strdup(const cJSON* obj);
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
void free_titles(const TitlesResponse *r);

#endif //IMDB_QUERY_IQUERY_H