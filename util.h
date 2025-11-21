//
// Created by joaop on 18/11/2025.
//
#ifndef IMDB_QUERY_UTIL_H
#define IMDB_QUERY_UTIL_H
#include <stddef.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};


size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

#endif //IMDB_QUERY_UTIL_H