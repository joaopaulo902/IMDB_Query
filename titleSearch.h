//
// Created by andri on 06/12/2025.
//

#ifndef TITLESEARCH_H
#define TITLESEARCH_H

#include <stdint.h>
#include <stdio.h>

#define DICT_SIZE 50021   // very large prime number for hash table size
#define BLOCK_SIZE 128    // postings block size for scalable index
#define ASCII_SIZE 256    // size of ascii alphabet
#define CUTOFF 16         // cutoff for radix sort change of behaviour

/**
 * In-memory posting list structure
 */
typedef struct {
    char *term; // token
    int *ids; // vector of IDs (RAM only)
    int count; // used entries
    int capacity; // allocated entries
} PostingList;

/**
 * Structure for a block in the postings list file.
 * Each block contains a fixed number of document IDs and a pointer to the next block.
 */
typedef struct {
    int count; // how many IDs this block contains
    int64_t nextOffset; // next block offset (0 = end of chain)
    int ids[BLOCK_SIZE]; // fixed-size payload
} PostingBlock;

/**
 * Vocabulary entry structure stored in vocabulary.bin
 */
typedef struct {
    char term[32]; // normalized token
    int64_t firstBlockOffset; // offset of first block in postings.bin
} VocabularyEntry;

/**
 * Temporary dictionary entry for in-memory indexing
 */
typedef struct {
    char* term;
    int count;
    int* ids;
} DictEntry;

/** In-memory dictionary for indexing
 */
extern PostingList dictionary[DICT_SIZE];

/**
 * Removes accents from a character
 * @param c input character
 * @return character without accent
 */
char remove_accent(unsigned char c);

/**
 * Normalizes a title by lowercasing, removing accents, and stripping punctuation
 * @param input raw title
 * @param output normalized title
 */
void normalize_title(char *input, char *output);

/**
 * Tokenizes a normalized title and indexes each token
 * @param normalized normalized title
 * @param id title ID
 */
void tokenize_and_index(char *normalized, int id);

/**
 * Hashes a token to an index in the dictionary
 * @param s input token
 * @return hash index
 */
unsigned int hash_token(char *s);

/**
 * Inserts a token and its associated title ID into the in-memory dictionary
 * @param token input token
 * @param id title ID
 */
void insert_token_into_dictionary(char *token, int id);

/**
 * Checks if a token is a stopword
 * @param token input token
 * @return 1 if stopword, 0 otherwise
 */
int is_stopword(char *token);

/**
 * Writes a scalable postings list to postings.bin
 * @param posts file pointer to postings.bin
 * @param ids array of title IDs
 * @param count number of IDs
 * @return offset of the first block in the postings list
 */
int64_t write_posting_list(FILE* posts, int* ids, int count);

/**
 * Saves vocabulary + postings to disk
 * @param vocabFile path to vocabulary.bin
 * @param postingsFile path to postings.bin
 */
void save_dictionary(const char* vocabFile, const char* postingsFile);

/**
 * Compares two dictionary entries by term
 */
int compare_terms(const void* a, const void* b);
/**
 * Compares two integers
 */
int compare_int(const void* a, const void* b);

/**
 * Finds a term in the vocabulary.bin
 * @param term input token
 * @return VocabularyEntry struct (firstBlockOffset = -1 if not found)
 */
VocabularyEntry find_in_vocabulary(char* term);

/**
 * Loads postings list from postings.bin given an offset
 * @param offset offset of the first block in the postings list
 * @param outCount output parameter for number of IDs loaded
 * @return array of title IDs
 */
int* load_postings(int64_t offset, int* outCount);

/**
 * Searches for a term and retrieves associated title IDs
 * @param term input token
 * @param outCount output parameter for number of IDs found
 * @return array of title IDs
 */
int* search_term(char* term, int* outCount);

/**
 * Compares character to '\0', returns -1 if the string has ended
 * @param s
 * @param d
 * @return
 */
int char_at(const char *s, int d);

/**
 *
 * @param a
 * @param lo
 * @param hi
 * @param d
 */
void bubble_sort(DictEntry *a, size_t lo, size_t hi, int d);

/**
 *
 * @param a
 * @param aux
 * @param lo
 * @param hi
 * @param d
 */
static void msd_sort_rec(DictEntry *a, DictEntry *aux, size_t lo, size_t hi, int d);

/**
 *
 * @param arr
 * @param n
 */
void msd_radix_sort_dict(DictEntry *arr, size_t n);

char *get_key(const DictEntry *e);
#endif //TITLESEARCH_H
