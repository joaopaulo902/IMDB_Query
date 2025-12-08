#ifndef BPTREE_H
#define BPTREE_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NODE_PAGE_SIZE 4096     // change to 10 for debugging small pages
#define YEAR_INDEX_FILE "titleIndex.bin"
#define RATING_INDEX_FILE "ratingIndex.bin"
#define MAX_LEAF_RESULTS 10
#define RANGE_LIMIT 10

/**
 * Header for each node stored on disk
 */
typedef struct {
    uint8_t is_leaf; // 1 leaf, 0 internal
    uint16_t n; // number of keys/entries used
    int64_t parent; // page_id * PAGE_SIZE offset of parent, -1 if root
    int64_t next; // for leaf: offset of next leaf page, else -1
    // padding to keep header small
    uint8_t reserved[7];
} __attribute__((packed)) NodeHeader;

/**
 * Key structure
 */
typedef struct {
    int32_t value;
    int64_t id;
} Key;

/**
 * Node in-memory representation (variable capacities based on page size)
* for internal: child ptrs length n+1
* note: for simplicity we allocate max capacity and treat unused slots as empty
 */
typedef struct {
    NodeHeader hdr;
    // arrays sized to capacity (allocated dynamically)
    Key *keys; // length = n (for internal: separators)
    int64_t *ptrs; // for leaf: values[] length n (title offsets)
} Node;

/**
 * B+Tree file header
 */
typedef struct {
    uint32_t page_size;
    uint64_t root_offset;
    uint64_t node_count;
} BPHeader;

/**
 * B+Tree handle
 */
typedef struct {
    FILE *f;
    BPHeader hdr;
    int64_t root_offset; // byte offset of root page (page_id * PAGE_SIZE), -1 if empty
    size_t leaf_capacity;
    size_t internal_capacity;
} BPTree;

/**
 * @param value_offset offset of the value (title record offset in titles.bin)
 * @param ctx user context pointer
 */
typedef void (*range_cb)(int64_t value_offset, void *ctx);

/**
 * Compare two keys
 * @return negative if a < b, 0 if a == b, positive if a > b
 */
int key_cmp(const Key *a, const Key *b);

/**
 * @return maximum number of entries in a leaf node
 */
size_t max_leaf_entries();

/**
 * @return maximum number of keys in an internal node
 */
size_t max_internal_keys();

/**
 * Write B+Tree header to file
 * @param *T B+Tree handle
 * @param page_offset offset of the page to write
 * @param buf buffer containing page data
 */
void write_page(BPTree *T, int64_t page_offset, const uint8_t *buf);

/**
 * Read a page from file into buffer
 * @param *T B+Tree handle
 * @param page_offset offset of the page to read
 * @param buf buffer to store page data
 */
void read_page(BPTree *T, int64_t page_offset, uint8_t *buf);

/**
 * Allocate a new page in the B+Tree file
 * @param *T B+Tree handle
 * @return offset of the newly allocated page
 */
int64_t alloc_page(BPTree *T);

/**
 * Serialize a node into a page buffer
 * @param *T B+Tree handle
 * @param *node Node to serialize
 * @param *pagebuf Buffer to store serialized data
 */
void node_serialize(BPTree *T, Node *node, uint8_t *pagebuf);

/**
 * Deserialize a node from a page buffer
 * @param *T B+Tree handle
 * @param *node Node to populate
 * @param *pagebuf Buffer containing serialized data
 */
void node_deserialize(BPTree *T, Node *node, const uint8_t *pagebuf);

/**
 * Free a node's memory
 * @param *n Node to free
 */
void node_free(Node *n);

/**
 * Read a node from file at given offset
 * @param *T B+Tree handle
 * @param page_offset Offset of the page to read
 * @return Pointer to the loaded Node
 */
Node *read_node(BPTree *T, int64_t page_offset);

/**
 * Create a new node (leaf or internal)
 * @param *T B+Tree handle
 * @param is_leaf 1 for leaf, 0 for internal
 * @return Pointer to the created Node
 */
Node *node_create(BPTree *T, int is_leaf);

/**
 * Write a node to file at given offset
 * @param *T B+Tree handle
 * @param page_offset Offset of the page to write
 * @param *n Node to write
 */
void write_node(BPTree *T, int64_t page_offset, Node *n);

/**
 * Open a B+Tree file
 * @param *path Path to B+Tree file
 */
BPTree *bpt_open(const char *path);

/**
 * Close a B+Tree file
 * @param *T B+Tree handle
 */
void bpt_close(BPTree *T);

/**
 * Find the leaf node that should contain the given key
 * @param *T B+Tree handle
 * @param *k Key to search for
 * @param **leaf_out Output pointer to the found leaf node
 * @return Offset of the found leaf node, or -1 on error
 */
Node *load_node_at(BPTree *T, int64_t page_offset);

/**
 * Load a node from file at given offset
 * @param *T B+Tree handle
 * @param page_offset Offset of the page to read
 * @return Pointer to the loaded Node
 */
Node *load_node_at(BPTree *T, int64_t page_offset);

/**
 * Insert a key-value pair into a non-full leaf node
 * @param *T B+Tree handle
 * @param *leaf Leaf node to insert into
 * @param leaf_offset Offset of the leaf node
 * @param *k Key to insert
 * @param value_offset Offset of the value to insert
 */
void leaf_insert_nonfull(BPTree *T, Node *leaf, int64_t leaf_offset, const Key *k, int64_t value_offset);

/**
 * Split a full leaf node and promote a key to the parent
 * @param *T B+Tree handle
 * @param *leaf Leaf node to split
 * @param leaf_offset Offset of the leaf node
 * @param *k Key to insert
 * @param value_offset Offset of the value to insert
 * @param *promoted Output pointer for the promoted key
 * @return Offset of the newly created leaf node
 */
int64_t leaf_split(BPTree *T, Node *leaf, int64_t leaf_offset, const Key *k, int64_t value_offset, Key *promoted);

/**
 * Find the leaf node that should contain the given key
 * @param *T B+Tree handle
 * @param *k Key to search for
 * @param **leaf_out Output pointer to the found leaf node
 * @return Offset of the found leaf node, or -1 on error
 */
void bpt_insert(BPTree *T, int32_t value, int64_t id, int64_t value_offset);

/**
 * Perform a range query on the B+Tree
 * @param *T B+Tree handle
 * @param ylo Lower bound of the range (inclusive)
 * @param yhi Upper bound of the range (inclusive)
 * @param *count_out Output pointer for the count of results found
 * @param *leaf_off_io Input/output pointer for leaf offset (for continuation)
 * @param *pos_io Input/output pointer for position in leaf (for continuation)
 * @return Array of offsets of found values (dynamically allocated, must be freed by caller)
 */
int64_t *bpt_range_query_page(BPTree *T, int32_t ylo, int32_t yhi, int *count_out, int64_t *leaf_off_io, int *pos_io);

/**
 * Example callback function to print value offsets
 * @param val Value offset
 * @param ctx User context pointer
 * @return 0 to continue, non-zero to stop
 */
int print_value_cb(int64_t val, void *ctx);

/**
 * Test function for B+Tree operations
 * @return 0 on success, -1 on failure
 */
int BP_tree_test(void);

#endif //BPTREE_H
