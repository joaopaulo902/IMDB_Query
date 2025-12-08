#ifndef FILTERGENRE_H
#define FILTERGENRE_H

#include "entities.h"

// Lista oficial de gêneros presentes no IMDB e suportados pelo índice
static const char *GENRE_LIST[] = {
    "Action", "Adult", "Adventure", "Animation", "Biography", "Comedy", "Crime",
    "Documentary", "Drama", "Family", "Fantasy", "Film Noir", "Game Show",
    "History", "Horror", "Musical", "Music", "Mystery", "News", "Reality-TV",
    "Romance", "Sci-Fi", "Short", "Sport", "Talk-Show", "Thriller",
    "War", "Western"
};

static const int GENRE_COUNT = sizeof(GENRE_LIST) / sizeof(*GENRE_LIST);

/**
 * Hash function for genre index
 * @param h input hash value
 * @return hashed value
 */
uint64_t hashFunction(uint64_t h);

/**
 * Inserts a title into the genre index files
 * @param title parsed title information
 * @param entry title entry to be indexed
 */
void insert_genre_index(ParseTitle title, Title entry);

/**
 * Checks if a genre contains a specific title ID
 * @param genre genre to check
 * @param id title ID to look for
 * @return 1 if genre contains the title ID, 0 otherwise
 */
int genre_contains(const char *genre, int64_t id);

/**
 * Filters titles by genre and returns matching titles
 * @param genre genre to filter by
 * @param outTitles output array of titles matching the genre
 * @return count of titles matching the genre
 */
int filter_genre_raw(const char *genre, Title **outTitles);

#endif
