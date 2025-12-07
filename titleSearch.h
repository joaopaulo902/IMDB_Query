//
// Created by andri on 06/12/2025.
//

#ifndef TITLESEARCH_H
#define TITLESEARCH_H

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DICT_SIZE 50021   // very large prime number for hash table size
#define BLOCK_SIZE 128    // postings block size for scalable index

typedef struct {
    char *term; // token
    int *ids; // vector of IDs (RAM only)
    int count; // used entries
    int capacity; // allocated entries
} PostingList;

// Block stored inside postings.bin
typedef struct {
    int count; // how many IDs this block contains
    int64_t nextOffset; // next block offset (0 = end of chain)
    int ids[BLOCK_SIZE]; // fixed-size payload
} PostingBlock;

// Entry stored inside vocabulary.bin
typedef struct {
    char term[32]; // normalized token
    int64_t firstBlockOffset; // offset of first block in postings.bin
} VocabularyEntry;

typedef struct {
    char* term;
    int count;
    int* ids;
} DictEntry;

extern PostingList dictionary[DICT_SIZE];

char remove_accent(unsigned char c);

void normalize_title(char *input, char *output);

void tokenize_and_index(char *normalized, int id);

unsigned int hash_token(char *s);

void insert_token_into_dictionary(char *token, int id);

int is_stopword(char *token);

// Writes a scalable postings list to postings.bin
int64_t write_posting_list(FILE* posts, int* ids, int count);

// Saves vocabulary + postings to disk
void save_dictionary(const char* vocabFile, const char* postingsFile);

// qsort helpers
int compare_terms(const void* a, const void* b);
int compare_int(const void* a, const void* b);

VocabularyEntry find_in_vocabulary(char* term);
int* load_postings(int64_t offset, int* outCount);

int* search_term(char* term, int* outCount);

#endif //TITLESEARCH_H
