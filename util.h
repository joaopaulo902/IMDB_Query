//
// Created by joaop on 18/11/2025.
//
//utilities library for the project
#ifndef IMDB_QUERY_UTIL_H
#define IMDB_QUERY_UTIL_H
#include <stdio.h>

/**
 * Structure to hold data in memory for libcurl
 */
struct MemoryStruct {
    char *memory;
    size_t size;
    size_t capacity;
};

/**
 * Callback function for libcurl to write data into memory
 * @param contents pointer to data
 * @param size size of each data element
 * @param nmemb number of data elements
 * @param userp pointer to MemoryStruct
 * @return number of bytes processed
 */
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

/**
 * Reads the entire contents of a file into a dynamically allocated buffer
 * @param fp pointer to file
 * @return pointer to file buffer
 */
char *read_entire_file(FILE *fp);

/**
 * Checks if a file is empty
 * @param fp file pointer
 * @return returns false if file is empty
 */
int is_file_empty(FILE *fp);

#endif //IMDB_QUERY_UTIL_H
