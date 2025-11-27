//
// Created by joaop on 25/11/2025.
//

#ifndef IMDB_QUERY_ENTITIES_H
#define IMDB_QUERY_ENTITIES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define TITLE_FILE_ID 0x4e49414d
#define STATS_FILE_ID 0x54415453
#define RATING_FILE_ID 0x4C415645


typedef struct {
    uint64_t ID;        // File identifier
    uint32_t version;      // Format version
    uint64_t recordCount;  // How many titles exist
    char nextPageToken[1024]; //what is the next page's token if processing is halted
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
} Genre;

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
    int64_t id;

    int64_t titleId;
} GenreTitle;

/**
 *
 */
typedef struct {
    int64_t id;
    char description[128];// I don't know what it is
    double aggregateRating;
    unsigned long int voteCount;
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
    int endYear; //final airing year (for tv series or shows)

    int64_t statId; //correspondent stat Id offset for title
    int64_t ratingId; //correspondent rating Id offset for title
    //int isAdult; actual boolean type: maybe we put it in
} Titles;

/**
 * struct that will go into seed types.bin
 * this file's items point to each type's listing files
 */
typedef struct {
    int32_t id;
    char name[50]; //type name. eg: MOVIE, TV_SERIES...
    char typeFileName[50];
}Type;

/**
 * Contains the offset of a title's entry classified as the type it's under
 */
typedef struct {
    int64_t id;

    int32_t titleId;
} TypeTitle;

/**
 *
 */
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

#endif //IMDB_QUERY_ENTITIES_H