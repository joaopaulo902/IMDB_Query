//
// Created by joaop on 07/12/2025.
//

#ifndef IMDB_QUERY_FILTERISADULT_H
#define IMDB_QUERY_FILTERISADULT_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NODE_PAGE_SIZE 4096     // change to 10 for debugging small pages
#define YEAR_INDEX_FILE "titleIndex.bin"
#define RATING_INDEX_FILE "ratingIndex.bin"

// Node header on disk (fixed size)
typedef struct {
    uint8_t is_leaf;     // 1 leaf, 0 internal
    uint16_t n;          // number of keys/entries used
    int64_t parent;      // page_id * PAGE_SIZE offset of parent, -1 if root
    int64_t next;        // for leaf: offset of next leaf page, else -1
    // padding to keep header small
    uint8_t reserved[7];
} __attribute__((packed)) NodeHeader;

// composite key type
typedef struct {
    int32_t value;
    int64_t id;
} Key;

// Node in-memory representation (variable capacities based on page size)
typedef struct {
    NodeHeader hdr;
    // arrays sized to capacity (allocated dynamically)
    Key *keys;         // length = n (for internal: separators)
    int64_t *ptrs;     // for leaf: values[] length n (title offsets)
    // for internal: child ptrs length n+1
    // note: for simplicity we allocate max capacity and treat unused slots as empty
} Node;


typedef struct {
    uint32_t page_size;
    uint64_t root_offset;
    uint64_t node_count;
} BPHeader;
// Tree handle
typedef struct {
    FILE *f;
    BPHeader hdr;
    int64_t root_offset; // byte offset of root page (page_id * PAGE_SIZE), -1 if empty
    size_t leaf_capacity;
    size_t internal_capacity;
} BPTree;



// range query: collect all value_offsets for keys with year in [ylo,yhi]
typedef void (*range_cb)(int64_t value_offset, void *ctx);

static int key_cmp(const Key *a, const Key *b);

static size_t max_leaf_entries(void);

static size_t max_internal_keys(void);

static void write_page(BPTree *T, int64_t page_offset, const uint8_t *buf);

static void read_page(BPTree *T, int64_t page_offset, uint8_t *buf);

static int64_t alloc_page(BPTree *T);

static void node_serialize(BPTree *T, Node *node, uint8_t *pagebuf);

static void node_deserialize(BPTree *T, Node *node, const uint8_t *pagebuf);

static void node_free(Node *n);

static Node *read_node(BPTree *T, int64_t page_offset);

static Node *node_create(BPTree *T, int is_leaf);

static void write_node(BPTree *T, int64_t page_offset, Node *n);

BPTree *bpt_open(const char *path);

void bpt_close(BPTree *T);

static Node *load_node_at(BPTree *T, int64_t page_offset);

static Node *load_node_at(BPTree *T, int64_t page_offset);

static void leaf_insert_nonfull(BPTree *T, Node *leaf, int64_t leaf_offset, const Key *k, int64_t value_offset);

static int64_t leaf_split(BPTree *T, Node *leaf, int64_t leaf_offset, const Key *k, int64_t value_offset, Key *promoted);

void bpt_insert(BPTree *T, int32_t value, int64_t id, int64_t value_offset);

int64_t* bpt_range_query_page(BPTree *T, int32_t ylo, int32_t yhi, int *count_out, int64_t *leaf_off_io, int *pos_io);

int print_value_cb(int64_t val, void *ctx);
//test that contains an example of how to call the query function for trees
int BP_tree_test(void);
#endif //IMDB_QUERY_FILTERISADULT_H