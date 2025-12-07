//
// Created by joaop on 05/12/2025.
//

#ifndef IMDB_QUERY_POSTINGS_H
#define IMDB_QUERY_POSTINGS_H
#include "entities.h"

void insert_genre_index(ParseTitle title, Titles entry);
uint64_t hashFunction(uint64_t h);
void test_filter(void);

#endif //IMDB_QUERY_POSTINGS_H