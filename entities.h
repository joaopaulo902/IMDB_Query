#ifndef IMDB_QUERY_ENTITIES_H
#define IMDB_QUERY_ENTITIES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

#define TITLE_FILE_ID 0x4e49414d
#define STATS_FILE_ID 0x54415453

/**
 * Struct for storing a @code Rating's@endcode info
 * @code
    double aggregateRating;
    int voteCount;
 * @endcode
 */
typedef struct {
    double aggregateRating;
    int voteCount;
} ParseRating;


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
    int startYear;
    int runtimeSeconds;
    int64_t ratingId;

    char **genres;
    int genresCount;

    ParseRating rating;

    char *plot;
} ParseTitle;


/**
 * Struct for storing a page of the api's response
 */
typedef struct {
    ParseTitle *titles;
    long int pageCount;
    long int totalCount;
    char *token;
} TitlesResponse;

/**
 * struct that will go into the header of each binary file
 */
typedef struct {
    uint64_t ID; // File identifier
    uint32_t version; // Format version
    int64_t recordCount; // How many titles exist
    time_t updatedAt;
    time_t createdAt;

    char nextPageToken[1024];
    //what is the next page's token if processing is halted
} FileHeader;

/**
 * struct that will go into the seed genres.bin
 * this file's items point to each genre's listing files
 * ex:
 * 0 - Drama => Drama.bin =  [{ 0, 45}, {2, 47}, ...] (title entries classified as Drama)
 */
typedef struct {
    int32_t id; //genre ID
    char name[32]; //genre name
    char fileName[32]; //name of the file that contains the titles belonging to the genre
} ParseGenre;

/**
 * Contains a title's stats
 */
typedef struct {
    int64_t id; //stat ID
    int winCount;
    int nominationCount;
} Stat;

/**
 * Contains the offset of a title's entry classified as the genre it's under
 */
typedef struct {
    int64_t titleId;
} GenreTitle;

/**
 * struct that will go into the ratings.bin
 * this file's items point to each title's rating info
 */
typedef struct {
    int64_t id;

    int32_t aggregateRating;
    uint64_t voteCount;
} Rating;

/**
 *  @code
 *  fields:
    int64_t id;
    char IMDBid[32];
    char type[32];
    char primaryTitle[128];
    int startYear;
    int endYear;

    int64_t statId;
    int64_t ratingId;
    @endcode
*/
typedef struct {
    int64_t id; //primary title key
    char IMDBid[32]; // id used by IMDB api for other requests concerning the title
    char type[32]; // type of title for presentation of a single entry
    char primaryTitle[128]; // title name
    char plot[1024];
    int startYear; //start airing year
    int runtimeSeconds; //run time seconds for the show
    Rating rating; //rating info for the show
    time_t updatedAt;
    time_t createdAt;


    int64_t statId; //correspondent stat ID offset for title
    //int isAdult; actual boolean type: maybe we put it in
} Title;

/**
 * struct that will go into seed types.bin
 * this file's items point to each type's listing files
 */
typedef struct {
    int32_t id;
    char name[50]; //type name. eg: MOVIE, TV_SERIES...
    char typeFileName[50];
} Type;

/**
 * Contains the offset of a title's entry classified as the type it's under
 */
typedef struct {
    int64_t id;

    int32_t titleId;
} TypeTitle;

/** Defines an episode struct */
typedef struct {
    uint64_t id;
    char title[100];
    int season;
    int episodeNumber;

    int64_t titleId;
    int64_t ratingId;
} Episode;

/**
 *
 */
typedef struct {
    int32_t id;
    char name[50];
} Event;

/**
 *
 */
typedef struct {
    uint64_t id;
    char category[50];
    char text[100];
    int year;
} Award;

typedef struct {
    uint64_t id;

    int32_t eventId; //id used in event struct
    uint64_t awardId; //id used in award struct
} EventAward;

/**
 *
 */
typedef struct {
    uint64_t id;
    uint64_t awardId;
    int64_t titleId;
    bool isWinner;
} Nomination;

/**
 *
 * @param r TitleResponse struct for storing data
 * @param fileName file read
 * @return number of titles in the page
 * processes an api request. takes a json format entry and converts it into a temporary dynamically allocated struct
 */
int get_page_title_item(TitlesResponse *r, char fileName[]);

/**
 * @param obj
 * @return json string item
 *
 * if string object is a string and not NULL, returns the string object
 */
char *json_strdup(const cJSON *obj);


/**
* @param item pointer to an item
* @return t individual title struct\n
* parse @code item@endcode  from the json file individually
*/
ParseTitle parse_title(const cJSON *item);

/**
 *
 * @param r struct that contains an array of titles, and it's count
 *
 * frees allocated struct @code TitlesResponse@endcode
 */
void free_titles_response(TitlesResponse *r);

/**
 *
 * @param title allocated array that stores the api's pages titles
 * @param i index of current title
 * @param fHeader file Header struct
 * @param fileName binary file pointer
 */
Title record_title_on_binary(ParseTitle title, FileHeader fHeader, int i, char fileName[]);

/**
 *
 * @param fH pointer to file header struct that will be filled
 * @param fileName name of the binary file to read the header from
 * @return 0 - if ok \n
 * @return -1 - if error has occurred \n\n
 * reads the file header from the desired binary file into the provided struct pointer
 */
int get_file_header(FileHeader *fH, char fileName[]);

#endif //IMDB_QUERY_ENTITIES_H
